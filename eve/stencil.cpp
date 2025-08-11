#include <iostream>
#include <chrono>
#include <cmath>
#include <vector>

#include <eve/eve.hpp>
#include <eve/module/math.hpp>
#include <eve/module/algo.hpp>

void process_scalar(std::span<float const> in, std::span<float> out)
{
  float scale = 1.f/3;
  for(std::size_t i=1;i<in.size()-1;++i)
    out[i] = (in[i-1] + in[i] + in[i+1]) * scale;
}

void process_eve(std::span<float const> in, std::span<float> out)
{
  auto im = std::span<float const>{in.data()+1,in.size()-1};
  auto id = std::span<float const>{in.data()+2,in.size()-1};
  float scale = 1.f/3;

  eve::algo::transform_to[eve::algo::unroll<4>]
  ( eve::views::zip(in,im,id)
  , out
  , [scale](auto lanes)
    {
      auto[e0,e1,e2] = lanes;
      return (e0 + e1 + e2)*scale;
    }
  );
}

constexpr int repetitions = 1000;
constexpr int size        = 1024*1024;
std::vector<float> x(size,0.5f), y(size,0.f);

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