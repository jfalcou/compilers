#include <iostream>
#include <chrono>
#include <cmath>
#include <vector>

#include <eve/eve.hpp>
#include <eve/module/math.hpp>
#include <eve/module/algo.hpp>

void process_scalar(std::span<float> data, float alpha)
{
  for(auto& e : data)
    e = std::expm1(e);
}

void process_eve(std::span<float> data, float alpha)
{
  eve::algo::transform_inplace
  ( data
  , [alpha](auto e) { return eve::expm1(e); }
  );
}

int main()
{
  constexpr int repetitions = 1000;
  constexpr int size        = 1024*1024;

  std::cout << eve::current_api << "\n";
  std::vector<float> x(size,0.5f);

  auto sc0 = std::chrono::steady_clock::now();
  for(int i=0;i<repetitions;i++)
  {
    process_scalar(x,0.2357f);
  }
  auto sc1 = std::chrono::steady_clock::now();

  auto ds = std::chrono::duration<double,std::micro>(sc1-sc0);
  std::cout << "Scalar time: " << ds/repetitions << "\n";

  auto ec0 = std::chrono::steady_clock::now();
  for(int i=0;i<repetitions;i++)
  {
    process_eve(x,0.2357f);
  }
  auto ec1 = std::chrono::steady_clock::now();

  auto es = std::chrono::duration<double,std::micro>(ec1-ec0);
  std::cout << "SIMD   time: " << es/repetitions << "\n";
  std::cout << "Ratio      : " << ds/es << "\n";
}