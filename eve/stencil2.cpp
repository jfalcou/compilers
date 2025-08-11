#include <iostream>
#include <chrono>
#include <vector>

#include <eve/eve.hpp>
#include <eve/module/math.hpp>
#include <eve/module/algo.hpp>

struct matrix
{
  public:
  matrix(std::size_t d0 = 0, std::size_t d1 = 0) : data_(d0 * d1), dim0_(d0), dim1_(d1) {}

  float&        operator()(std::size_t i0, std::size_t i1)        { return data_[i0 + i1 * dim0_]; }
  float const&  operator()(std::size_t i0, std::size_t i1) const  { return data_[i0 + i1 * dim0_]; }

  std::size_t size() const { return data_.size(); }
  std::size_t size(int d) const { return d == 0 ? dim0_ : dim1_; }

  std::span<float const> slice(int i, int offset, std::size_t sz) const
  {
    return std::span<float const>{&data_[i*dim0_+offset], sz};
  }

  std::span<float> slice(int i, int offset, std::size_t sz)
  {
    return std::span<float>{&data_[i*dim0_+offset], sz};
  }

  private:
  std::vector<float> data_;
  std::size_t dim0_, dim1_;
};

void process_scalar(matrix const& in, matrix& out)
{
  float scale = 1.f/9;
  for(std::size_t i1=1;i1<in.size(1)-1;++i1)
    for(std::size_t i0=1;i0<in.size(0)-1;++i0)
      out(i0,i1)  = ( in(i0-1,i1-1) + in(i0,i1-1) + in(i0+1,i1-1)
                    + in(i0-1,i1  ) + in(i0,i1  ) + in(i0+1,i1  )
                    + in(i0-1,i1+1) + in(i0,i1+1) + in(i0+1,i1+1)
                    ) * scale;
}

void process_eve(matrix const& in, matrix& out)
{
  float scale = 1.f/9;
  auto sz = in.size(1)-1;
  for(std::size_t i1=1;i1<sz;++i1)
  {
    auto r00  = in.slice(i1  ,0,sz);
    auto r01  = in.slice(i1  ,1,sz);
    auto r02  = in.slice(i1  ,2,sz);
    auto r10  = in.slice(i1+1,0,sz);
    auto r11  = in.slice(i1+1,1,sz);
    auto r12  = in.slice(i1+1,2,sz);
    auto r20  = in.slice(i1+2,0,sz);
    auto r21  = in.slice(i1+2,1,sz);
    auto r22  = in.slice(i1+2,2,sz);

    auto stencil = eve::views::zip(r00,r01,r02
                                  ,r10,r11,r12
                                  ,r20,r21,r22
                                  );
    eve::algo::transform_to
    ( stencil, out.slice(i1,1,sz)
    , [scale](auto lanes)
    {
      auto[e00,e01,e02,e10,e11,e12,e20,e21,e22] = lanes;
      auto c0 = e00+e10+e20;
      auto c1 = e01+e11+e21;
      auto c2 = e02+e22+e22;
      return (c0+c1+c2)*scale;
    }
    );
  }
}

constexpr int repetitions = 10000;
constexpr int size        = 755;
matrix x(size,size), y(size,size);

int main()
{
  std::cout << eve::current_api << "\n";

  auto sc0 = std::chrono::steady_clock::now();
  for(int i=0;i<repetitions;i++)
  {
    process_scalar(x,y);
  }
  auto sc1 = std::chrono::steady_clock::now();

  auto ds = std::chrono::duration<double,std::micro>(sc1-sc0);
  std::cout << "Scalar time: " << ds/repetitions << "\n";

  auto ec0 = std::chrono::steady_clock::now();
  for(int i=0;i<repetitions;i++)
  {
    process_eve(x,y);
  }
  auto ec1 = std::chrono::steady_clock::now();

  auto es = std::chrono::duration<double,std::micro>(ec1-ec0);
  std::cout << "SIMD   time: " << es/repetitions << "\n";
  std::cout << "Ratio      : " << ds/es << "\n";
}