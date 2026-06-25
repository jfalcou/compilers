#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cmath>
#include <iomanip>
#include <string>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <ranges>
#include <complex>

// Include EVE library
#include <eve/eve.hpp>
#include <eve/module/math.hpp>
#include <eve/module/algo.hpp>

// Type alias for EVE's tuple representation of a complex number
using complex_t = kumi::tuple<float, float>;

// ---------------------------------------------------------
// Helper function to prevent compiler from optimizing away
// unused variables or loops with no side-effects.
// ---------------------------------------------------------
template <typename T>
inline void do_not_optimize_away(const std::vector<T>& data)
{
  asm volatile("" : : "r"(data.data()) : "memory");
}

// ---------------------------------------------------------
// Benchmark Runner (Returns iteration times for CSV)
// ---------------------------------------------------------
template <typename Func>
std::vector<double> run_benchmark(const std::string& name, int iterations, Func&& func)
{
  std::vector<double> iter_times(iterations);
  double total_time = 0.0;

  for (int iter = 0; iter < iterations; ++iter)
  {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();

    double iter_time = std::chrono::duration<double, std::milli>(end - start).count();
    iter_times[iter] = iter_time;
    total_time += iter_time;
  }

  double avg_time = total_time / iterations;
  std::cout << "[" << std::left << std::setw(15) << name << "] Average time: " << avg_time << " ms";

  return iter_times;
}

// ---------------------------------------------------------
void process_scalar(std::vector<std::complex<float>> const& x, std::vector<float>& n)
{
  std::ranges::transform(x, n.begin(), [](auto e)
  {
    return std::hypot(e.real(), e.imag());
  });
}

// ---------------------------------------------------------
void process_simd_zip(std::vector<float> const& real, std::vector<float> const& imag, std::vector<float>& out)
{
  eve::algo::transform_to(eve::views::zip(real, imag), out, [](auto e)
  {
    auto[r, i] = e;
    return eve::hypot(r,i);
  });
}

// ---------------------------------------------------------
void process_simd_soa(eve::algo::soa_vector<complex_t> const& cmplx, std::vector<float>& out)
{
  eve::algo::transform_to(cmplx, out, [](auto z)
  {
    return eve::hypot(z);
  });
}

// ---------------------------------------------------------
void process_simd_raw_hypot(eve::algo::soa_vector<complex_t> const& cmplx, std::vector<float>& out)
{
  eve::algo::transform_to(cmplx, out, [](auto z)
  {
    return eve::hypot[eve::raw](z);
  });
}

// ---------------------------------------------------------
// Main Function
// ---------------------------------------------------------
int main(int argc, char* argv[])
{
  // Parse command line arguments
  bool generate_csv = false;
  for (int i = 1; i < argc; ++i)
  {
    if (std::string(argv[i]) == "-csv")
    {
      generate_csv = true;
      break;
    }
  }

  const int DATA_SIZE = 500000;
  const int NUM_ITERATIONS = 2000;

  std::cout << "========================================\n";
  std::cout << "Complex Magnitude EVE Algorithms Benchmark\n";
  std::cout << "Elements   : " << DATA_SIZE << "\n";
  std::cout << "Iterations : " << NUM_ITERATIONS << "\n";
  std::cout << "Export CSV : " << (generate_csv ? "Yes" : "No") << "\n";
  std::cout << "========================================\n\n";

  std::vector<float> r(DATA_SIZE);
  std::vector<float> i(DATA_SIZE);
  std::vector<std::complex<float>> c;
  c.reserve(DATA_SIZE);

  eve::algo::soa_vector<complex_t> z;
  z.reserve(DATA_SIZE);

  std::mt19937 gen(42);
  std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

  for (int k = 0; k < DATA_SIZE; ++k)
  {
    r[k] = dist(gen);
    i[k] = dist(gen);
    c.emplace_back(r[k], i[k]);
    z.push_back(complex_t{r[k], i[k]});
  }

  std::vector<float> out(DATA_SIZE, 0.0f);
  std::cout << std::fixed << std::setprecision(3);

  auto times_scalar = run_benchmark("Scalar ranges", NUM_ITERATIONS, [&]()
  {
    process_scalar(c, out);
    do_not_optimize_away(out);
  });
  double avg_scalar = std::accumulate(times_scalar.begin(), times_scalar.end(), 0.0) / NUM_ITERATIONS;
  std::cout << "\n";

  auto times_zip = run_benchmark("SIMD Zip", NUM_ITERATIONS, [&]()
  {
    process_simd_zip(r, i, out);
    do_not_optimize_away(out);
  });
  double avg_zip = std::accumulate(times_zip.begin(), times_zip.end(), 0.0) / NUM_ITERATIONS;
  std::cout << " \t(Speedup: " << avg_scalar / avg_zip << "x)\n";

  auto times_soa = run_benchmark("SIMD SOA", NUM_ITERATIONS, [&]()
  {
    process_simd_soa(z, out);
    do_not_optimize_away(out);
  });
  double avg_soa = std::accumulate(times_soa.begin(), times_soa.end(), 0.0) / NUM_ITERATIONS;
  std::cout << " \t(Speedup: " << avg_scalar / avg_soa << "x)\n";

  auto times_hypot = run_benchmark("SIMD Hypot", NUM_ITERATIONS, [&]()
  {
    process_simd_raw_hypot(z, out);
    do_not_optimize_away(out);
  });
  double avg_hypot = std::accumulate(times_hypot.begin(), times_hypot.end(), 0.0) / NUM_ITERATIONS;
  std::cout << " \t(Speedup: " << avg_scalar / avg_hypot << "x)\n";

  if (generate_csv)
  {
    std::ofstream csv_file("benchmark_complex.csv");
    if (csv_file.is_open())
    {
      csv_file << "scalar_ranges,simd_zip,simd_hypot,simd_raw_hypot\n";
      for (int k = 0; k < NUM_ITERATIONS; ++k)
      {
        csv_file << times_scalar[k] << "," << times_zip[k] << ","
                 << times_soa[k] << "," << times_hypot[k] << "\n";
      }
      csv_file.close();
      std::cout << "\n[Info] Data exported to 'benchmark_complex.csv'\n";
    }
  }
}