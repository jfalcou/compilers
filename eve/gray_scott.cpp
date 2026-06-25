#include <iostream>
#include <chrono>
#include <vector>
#include <span>
#include <iomanip>

#include <utility> // for std::index_sequence

#include <eve/eve.hpp>
#include <eve/module/algo.hpp>
#include <eve/module/core.hpp>

struct config
{
  std::size_t W;
  std::size_t H;
  std::size_t N;

  float Du;
  float Dv;
  float F;
  float k;
  float dt;
};

constexpr config cfg =
{
    .W = 256,
    .H = 256,
    .N = 256 * 256,

    // (preset "mitosis")
    .Du = 0.2097f,
    .Dv = 0.1050f,

    .F = 0.0280f,
    .k = 0.0620f,

    .dt = 1.0f
};

// -----------------------------------------------------------------------------
// Paramètres de la grille
// -----------------------------------------------------------------------------
constexpr std::size_t W = cfg.W;
constexpr std::size_t H = cfg.H;
constexpr std::size_t N = cfg.N;

// -----------------------------------------------------------------------------
// Paramètres Gray-Scott
// -----------------------------------------------------------------------------
constexpr float Du   = cfg.Du;
constexpr float Dv   = cfg.Dv;
constexpr float F    = cfg.F;
constexpr float k    = cfg.k;
constexpr float dt   = cfg.dt;

// Poids du stencil
constexpr float stencil_weight[3][3]
{
  {0.25 , 0.5 , 0.25},
  { 0.5 , 0.0 ,  0.5},
  {0.25 , 0.5 , 0.25}
};

// -----------------------------------------------------------------------------
// Utilitaires communs
// -----------------------------------------------------------------------------
EVE_FORCEINLINE float laplacian_at(std::span<float const> g, std::size_t row, std::size_t col)
{
  auto at = [&](std::size_t r, std::size_t c) -> float {
    return g[((r+H)%H)*W + ((c+W)%W)];
  };
  return at(row-1,col)+at(row+1,col)+at(row,col-1)+at(row,col+1) - 4.f*at(row,col);
}

void init(std::vector<float>& u, std::vector<float>& v)
{
  std::fill(u.begin(), u.end(), 1.f);
  std::fill(v.begin(), v.end(), 0.f);
  auto seed = [&](std::size_t cx, std::size_t cy, std::size_t r = 5){
    for (std::size_t dy=0;dy<r;++dy) for (std::size_t dx=0;dx<r;++dx)
      if (cy+dy<H && cx+dx<W){ u[(cy+dy)*W+(cx+dx)]=0.5f; v[(cy+dy)*W+(cx+dx)]=0.25f; }
  };
  seed(W/2-2,H/2-2); seed(W/4,H/4); seed(3*W/4,H/4); seed(W/4,3*H/4); seed(3*W/4,3*H/4);
}

// -----------------------------------------------------------------------------
// Version SCALAIRE (référence)
// -----------------------------------------------------------------------------
void step_scalar(std::span<float const> u_in,  std::span<float const> v_in,
                 std::span<float>       u_out, std::span<float>       v_out)
{
  for (std::size_t row=1;row<H-1;++row)
  {
    for (std::size_t col=1;col<W-1;++col)
    {
      float uc = u_in[row*W+col];
      float vc = v_in[row*W+col];

      auto uvv = uc * vc * vc;
      auto full_u = 0.f;
      auto full_v = 0.f;

      for (std::size_t i = 0; i < 3; ++i)
      {
        for(std::size_t j = 0; j < 3; ++j)
        {
          std::size_t n_row = row + i - 1;
          std::size_t n_col = col + j - 1;
          std::size_t n_idx = n_row * W + n_col;

          float uu = u_in[n_idx];
          float vv = v_in[n_idx];

          full_u += stencil_weight[i][j] * (uu - uc);
          full_v += stencil_weight[i][j] * (vv - vc);
        }
      }
      auto du = Du * full_u - uvv + F * (1.0f - uc);
      auto dv = Dv * full_v + uvv - (F + k) * vc;

      u_out[row*W+col] = uc + du * dt;
      v_out[row*W+col] = vc + dv * dt;
    }
  }

  auto update_border = [&](std::size_t row, std::size_t col){
    float uc=u_in[row*W+col], vc=v_in[row*W+col];
    float lu=laplacian_at(u_in,row,col), lv=laplacian_at(v_in,row,col);
    float uvv=uc*vc*vc;
    u_out[row*W+col]=uc+dt*(Du*lu-uvv+F*(1.f-uc));
    v_out[row*W+col]=vc+dt*(Dv*lv+uvv-(F+k)*vc);
  };
  for (std::size_t c=0;c<W;++c){ update_border(0,c); update_border(H-1,c); }
  for (std::size_t r=1;r<H-1;++r){ update_border(r,0); update_border(r,W-1); }
}

// -----------------------------------------------------------------------------
// Version EVE - Unrolled Kernel - Partial use of FMAs
// -----------------------------------------------------------------------------
struct unrolled_mixedfma
{
  EVE_FORCEINLINE constexpr void operator()(auto i, auto su, auto sv, auto out_u, auto out_v) const
  {
    auto u = eve::load(&su[i]);
    auto v = eve::load(&sv[i]);

    auto uvv = u * v * v;
    auto d_u =  F * (1.0f - u);
    auto d_v =  -1.0f * ((F + k) * v);

    auto full_u1 = stencil_weight[0][0] * (eve::load(&su[i - W - 1]) - u);
    auto full_u2 = stencil_weight[0][1] * (eve::load(&su[i - W    ]) - u);
    auto full_u3 = stencil_weight[0][2] * (eve::load(&su[i - W + 1]) - u);
    auto full_u4 = stencil_weight[1][0] * (eve::load(&su[i - 1    ]) - u);

    auto full_v1 = stencil_weight[0][0] * (eve::load(&sv[i - W - 1]) - v);
    auto full_v2 = stencil_weight[0][1] * (eve::load(&sv[i - W    ]) - v);
    auto full_v3 = stencil_weight[0][2] * (eve::load(&sv[i - W + 1]) - v);
    auto full_v4 = stencil_weight[1][0] * (eve::load(&sv[i - 1    ]) - v);

    static_assert(stencil_weight[1][1] == 0.0f);

    full_u1 = eve::fma(stencil_weight[1][2], (eve::load(&su[i     + 1]) - u), full_u1);
    full_u2 = eve::fma(stencil_weight[2][0], (eve::load(&su[i + W - 1]) - u), full_u2);
    full_u3 = eve::fma(stencil_weight[2][1], (eve::load(&su[i + W    ]) - u), full_u3);
    full_u4 = eve::fma(stencil_weight[2][2], (eve::load(&su[i + W + 1]) - u), full_u4);

    full_v1 = eve::fma(stencil_weight[1][2], (eve::load(&sv[i     + 1]) - v), full_v1);
    full_v2 = eve::fma(stencil_weight[2][0], (eve::load(&sv[i + W - 1]) - v), full_v2);
    full_v3 = eve::fma(stencil_weight[2][1], (eve::load(&sv[i + W    ]) - v), full_v3);
    full_v4 = eve::fma(stencil_weight[2][2], (eve::load(&sv[i + W + 1]) - v), full_v4);

    auto full_u = (full_u1 + full_u2) + (full_u3 + full_u4);
    auto full_v = (full_v1 + full_v2) + (full_v3 + full_v4);

    d_u += (Du * full_u) - uvv;
    d_v += (Dv * full_v) + uvv;

    auto vu = u + d_u * dt;
    auto vv = v + d_v * dt;

    eve::store(vu, &out_u[i]);
    eve::store(vv, &out_v[i]);
  }
};

// -----------------------------------------------------------------------------
// Version EVE - Unrolled Kernel - Full use of FMAs
// -----------------------------------------------------------------------------
struct unrolled_fullfma
{
  EVE_FORCEINLINE constexpr void operator()(auto i, auto su, auto sv, auto out_u, auto out_v) const
  {
    auto u = eve::load(&su[i]);
    auto v = eve::load(&sv[i]);

    auto uvv = u * v * v;
    auto d_u =  F * (1.0f - u);
    auto d_v =  -1.0f * ((F + k) * v);

    auto full_u1 = stencil_weight[0][0] * (eve::load(&su[i - W - 1]) - u);
    auto full_u2 = stencil_weight[0][1] * (eve::load(&su[i - W    ]) - u);
    auto full_u3 = stencil_weight[0][2] * (eve::load(&su[i - W + 1]) - u);
    auto full_u4 = stencil_weight[1][0] * (eve::load(&su[i - 1    ]) - u);

    auto full_v1 = stencil_weight[0][0] * (eve::load(&sv[i - W - 1]) - v);
    auto full_v2 = stencil_weight[0][1] * (eve::load(&sv[i - W    ]) - v);
    auto full_v3 = stencil_weight[0][2] * (eve::load(&sv[i - W + 1]) - v);
    auto full_v4 = stencil_weight[1][0] * (eve::load(&sv[i - 1    ]) - v);

    static_assert(stencil_weight[1][1] == 0.0f);

    full_u1 = eve::fma(stencil_weight[1][2], (eve::load(&su[i     + 1]) - u), full_u1);
    full_u2 = eve::fma(stencil_weight[2][0], (eve::load(&su[i + W - 1]) - u), full_u2);
    full_u3 = eve::fma(stencil_weight[2][1], (eve::load(&su[i + W    ]) - u), full_u3);
    full_u4 = eve::fma(stencil_weight[2][2], (eve::load(&su[i + W + 1]) - u), full_u4);

    full_v1 = eve::fma(stencil_weight[1][2], (eve::load(&sv[i     + 1]) - v), full_v1);
    full_v2 = eve::fma(stencil_weight[2][0], (eve::load(&sv[i + W - 1]) - v), full_v2);
    full_v3 = eve::fma(stencil_weight[2][1], (eve::load(&sv[i + W    ]) - v), full_v3);
    full_v4 = eve::fma(stencil_weight[2][2], (eve::load(&sv[i + W + 1]) - v), full_v4);

    auto full_u = (full_u1 + full_u2) + (full_u3 + full_u4);
    auto full_v = (full_v1 + full_v2) + (full_v3 + full_v4);

    d_u += eve::fms(Du, full_u, uvv);
    d_v += eve::fma(Dv, full_v, uvv);

    auto vu = eve::fma(d_u, dt, u);
    auto vv = eve::fma(d_v, dt, v);

    eve::store(vu, &out_u[i]);
    eve::store(vv, &out_v[i]);
  }
};

// -----------------------------------------------------------------------------
// Engine to run the benchs
// -----------------------------------------------------------------------------
template <typename Kernel, std::size_t Unroll>
struct stepper
{
  EVE_FORCEINLINE void operator()(std::span<float const> u_in,  std::span<float const> v_in,
                                std::span<float>       u_out, std::span<float>       v_out) const
  {
    constexpr std::size_t lanes = eve::wide<float>::size();
    constexpr std::size_t s = Unroll * lanes;
    constexpr Kernel kernel{};

    for (std::size_t row = 1; row < H - 1; ++row) {
      for (std::size_t col = 1; col + s<= W -1 ; col += s) {
        std::size_t i = row * W + col;
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
          (kernel(i + Is * lanes, u_in, v_in, u_out, v_out), ...);
        }(std::make_index_sequence<Unroll>{});
      }
    }

    // Border updates
    auto update_border = [&](std::size_t row, std::size_t col) {
        float uc = u_in[row * W + col], vc = v_in[row * W + col];
        float lu = laplacian_at(u_in, row, col), lv = laplacian_at(v_in, row, col);
        float uvv = uc * vc * vc;
        u_out[row * W + col] = uc + dt * (Du * lu - uvv + F * (1.f - uc));
        v_out[row * W + col] = vc + dt * (Dv * lv + uvv - (F + k) * vc);
    };
    for (std::size_t c = 0; c < W; ++c) { update_border(0, c); update_border(H - 1, c); }
    for (std::size_t r = 1; r < H - 1; ++r) { update_border(r, 0); update_border(r, W - 1); }
  }
};

template<typename K, std::size_t U>
inline constexpr stepper<K,U> step{};

// -----------------------------------------------------------------------------
// Benchmark
// -----------------------------------------------------------------------------
struct BenchResult { double per_step_us; double throughput_mcells; };

template<typename StepFn>
BenchResult bench(StepFn&& step, int steps, int reps)
{
  std::vector<float> u(N),v(N),u2(N),v2(N);
  double total=0.0;
  for (int rep=0;rep<reps;++rep){
    init(u,v);
    auto t0=std::chrono::steady_clock::now();
    for (int s=0;s<steps;++s){ step(u,v,u2,v2); std::swap(u,u2); std::swap(v,v2); }
    auto t1=std::chrono::steady_clock::now();
    total+=std::chrono::duration<double,std::micro>(t1-t0).count();
  }
  double avg=total/reps;
  return {avg/steps, double(W*H)*steps/(avg/1e6)/1e6};
}

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------
int main()
{
  constexpr int steps=2000, reps=5;

  std::cout << "╔══════════════════════════════════════════╗\n";
  std::cout << "║      Gray-Scott  —  Benchmark FMA        ║\n";
  std::cout << "╠══════════════════════════════════════════╣\n";
  std::cout << "║  SIMD arch : " << std::setw(28) << std::left << eve::current_api << "║\n";
  std::cout << "║  Grille    : " << std::setw(28) << std::left << (std::to_string(W)+"×"+std::to_string(H)) << "║\n";
  std::cout << "║  Steps     : " << std::setw(28) << std::left << steps << "║\n";
  std::cout << "╚══════════════════════════════════════════╝\n\n";

  std::cout << "Benchmark scalaire...  " << std::flush;
  auto sc = bench(step_scalar,   steps, reps); std::cout << "OK\n";
  std::cout << "Benchmark EVE - FMA...                  " << std::flush;
  auto ev1 = bench(step<unrolled_fullfma,1>,      steps, reps); std::cout << "OK\n";
  std::cout << "Benchmark EVE - FMA - Unrolled 2        " << std::flush;
  auto ev2 = bench(step<unrolled_fullfma,2>,      steps, reps); std::cout << "OK\n";
  std::cout << "Benchmark EVE - FMA - Unrolled 4        " << std::flush;
  auto ev3 = bench(step<unrolled_fullfma,4>,      steps, reps); std::cout << "OK\n";
  std::cout << "Benchmark EVE - Mixed...       " << std::flush;
  auto ev4 = bench(step<unrolled_mixedfma,1>,      steps, reps); std::cout << "OK\n";
  std::cout << "Benchmark EVE - Mixed - Unrolled 2      " << std::flush;
  auto ev5 = bench(step<unrolled_mixedfma,2>,      steps, reps); std::cout << "OK\n";
  std::cout << "Benchmark EVE - Mixed - Unrolled 4      " << std::flush;
  auto ev6 = bench(step<unrolled_mixedfma,4>,      steps, reps); std::cout << "OK\n";

  std::cout << std::fixed << std::setprecision(2);

  int    lanes  = eve::wide<float>::size();
  double sp_ev1 = sc.per_step_us / ev1.per_step_us;
  double sp_ev2 = sc.per_step_us / ev2.per_step_us;
  double sp_ev3 = sc.per_step_us / ev3.per_step_us;
  double sp_ev4 = sc.per_step_us / ev4.per_step_us;
  double sp_ev5 = sc.per_step_us / ev5.per_step_us;
  double sp_ev6 = sc.per_step_us / ev6.per_step_us;

  auto eff = [](auto sp, auto n){ return sp / n * 100; };

  auto header = []()
  {
    std::cout << " " << std::left << std::setw(20) << " "
              << std::right << std::setw(10) << "µs/step"     << " "
                            << std::setw(10) << "Mcells/s"    << " "
                            << std::setw(10) << "Speedup"     << " "
                            << std::setw(10) << "Efficiency"  << "\n";
  };

  auto row = [](std::string lbl, double step, double cells, double speedup, double eff)
  {
    std::cout << "  " << std::left  << std::setw(20) << lbl
              << std::right << std::setw(10) << step    << "  "
                            << std::setw(10) << cells   << "  "
                            << std::setw(10) << speedup << "x "
                            << std::setw(10) << eff     << "\n";
  };

  std::cout << "  " << std::string(80,'-') << "\n";

  header();
  row("Scalar   ", sc.per_step_us  , sc.throughput_mcells  , 1.0     , eff(1.0, lanes)     );
  row("EVE FMA  ", ev1.per_step_us  , ev1.throughput_mcells  , sp_ev1  , eff(sp_ev1, lanes)  );
  row("EVE FMA 2", ev2.per_step_us  , ev2.throughput_mcells  , sp_ev2  , eff(sp_ev2, lanes)  );
  row("EVE FMA 4", ev3.per_step_us  , ev3.throughput_mcells  , sp_ev3  , eff(sp_ev3, lanes)  );
  row("EVE Mix  ", ev4.per_step_us  , ev4.throughput_mcells  , sp_ev4  , eff(sp_ev4, lanes)  );
  row("EVE Mix 2", ev5.per_step_us  , ev5.throughput_mcells  , sp_ev5  , eff(sp_ev5, lanes)  );
  row("EVE Mix 4", ev6.per_step_us  , ev6.throughput_mcells  , sp_ev6  , eff(sp_ev6, lanes)  );


  std::cout << "  " << std::string(80,'-') << "\n";

  return 0;
}

