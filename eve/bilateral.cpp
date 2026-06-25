#include <iostream>
#include <vector>
#include <span>
#include <chrono>
#include <random>
#include <cmath>
#include <iomanip>
#include <string>
#include <fstream>
#include <numeric>

// Include EVE library
#include <eve/eve.hpp>
#include <eve/module/math.hpp> // For eve::exp

// Filter parameters
const int RADIUS = 5;
const float SIGMA_S = 3.0f;
const float SIGMA_I = 0.5f;

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
// Benchmark Runner
// Now returns a vector containing the time of EVERY iteration
// ---------------------------------------------------------
template <typename Func>
std::vector<double> run_benchmark(const std::string& name, int iterations, Func&& func)
{
  std::vector<double> iter_times(iterations);
  double total_time = 0.0;

  for (int iter = 0; iter < iterations; ++iter)
  {
    auto start = std::chrono::high_resolution_clock::now();
    func(); // Execute the lambda
    auto end = std::chrono::high_resolution_clock::now();

    double iter_time = std::chrono::duration<double, std::milli>(end - start).count();
    iter_times[iter] = iter_time;
    total_time += iter_time;
  }

  double avg_time = total_time / iterations;

  // std::setw ensures the output columns are nicely aligned
  std::cout << "[" << std::left << std::setw(12) << name << "] Average time: " << avg_time << " ms";

  return iter_times;
}

// ---------------------------------------------------------
// 1. Generic Scalar Version
// ---------------------------------------------------------
template <typename ExpFunc>
void bilateral_scalar(std::span<const float> in, std::span<float> out, ExpFunc exp_func)
{
  int n = in.size();
  float var_s = 2.0f * SIGMA_S * SIGMA_S;
  float var_i = 2.0f * SIGMA_I * SIGMA_I;

  for (int i = RADIUS; i < n - RADIUS; ++i)
  {
    float sum = 0.0f;
    float weight_sum = 0.0f;

    for (int j = -RADIUS; j <= RADIUS; ++j)
    {
      float spatial_diff = (float)(j * j);
      float val_j = in[i + j];
      float intensity_diff = (in[i] - val_j) * (in[i] - val_j);

      float weight = exp_func(-(spatial_diff / var_s + intensity_diff / var_i));

      sum += val_j * weight;
      weight_sum += weight;
    }
    out[i] = sum / weight_sum;
  }
}

// ---------------------------------------------------------
// 2. Generic EVE Version
// ---------------------------------------------------------
template <typename ExpFunc>
void bilateral_eve(std::span<const float> in, std::span<float> out, ExpFunc eve_exp)
{
  int n = in.size();
  float var_s = 2.0f * SIGMA_S * SIGMA_S;
  float var_i = 2.0f * SIGMA_I * SIGMA_I;

  using w_t = eve::wide<float>;
  int N = w_t::size();

  int end_simd = (n - RADIUS) - N;
  int i = RADIUS;

  // Main SIMD loop
  for (; i <= end_simd; i += N)
  {
    w_t img_i(&in[i]);
    w_t sum(0.0f);
    w_t weight_sum(0.0f);

    for (int j = -RADIUS; j <= RADIUS; ++j)
    {
      float spatial_diff = (float)(j * j);
      w_t val_j(&in[i + j]);

      w_t intensity_diff = (img_i - val_j) * (img_i - val_j);
      w_t weight = eve_exp(-(spatial_diff / var_s + intensity_diff / var_i));

      sum += val_j * weight;
      weight_sum += weight;
    }
    eve::store(sum / weight_sum, &out[i]);
  }

  // Tail handling
  if (i < n - RADIUS)
  {
    std::size_t tail_start = i - RADIUS;
    bilateral_scalar(in.subspan(tail_start), out.subspan(tail_start), eve_exp);
  }
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

  const int DATA_SIZE = 50000;
  const int NUM_ITERATIONS = 2000;

  std::cout << "========================================\n";
  std::cout << "1D Bilateral Filter Benchmark\n";
  std::cout << "Elements   : " << DATA_SIZE << "\n";
  std::cout << "Iterations : " << NUM_ITERATIONS << "\n";
  std::cout << "Export CSV : " << (generate_csv ? "Yes" : "No") << "\n";
  std::cout << "========================================\n\n";

  std::vector<float> input(DATA_SIZE);
  std::vector<float> output_scalar(DATA_SIZE, 0.0f);
  std::vector<float> output_eve(DATA_SIZE, 0.0f);
  std::vector<float> output_eve_raw(DATA_SIZE, 0.0f);

  std::mt19937 gen(42);
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);

  for (float& f : input)
  {
    f = dist(gen);
  }

  std::cout << std::fixed << std::setprecision(3);

  auto std_exp_lambda = [](float x) { return std::exp(x); };

  // --- Run Benchmarks ---

  auto times_scalar = run_benchmark("Scalar", NUM_ITERATIONS, [&]()
  {
    bilateral_scalar(input, output_scalar, std_exp_lambda);
    do_not_optimize_away(output_scalar);
  });
  double avg_scalar = std::accumulate(times_scalar.begin(), times_scalar.end(), 0.0) / NUM_ITERATIONS;
  std::cout << "\n";

  auto times_eve = run_benchmark("EVE Standard", NUM_ITERATIONS, [&]()
  {
    bilateral_eve(input, output_eve, eve::exp);
    do_not_optimize_away(output_eve);
  });
  double avg_eve = std::accumulate(times_eve.begin(), times_eve.end(), 0.0) / NUM_ITERATIONS;
  std::cout << " \t(Speedup: " << avg_scalar / avg_eve << "x)\n";

  auto times_raw = run_benchmark("EVE Raw", NUM_ITERATIONS, [&]()
  {
    bilateral_eve(input, output_eve_raw, eve::exp[eve::raw]);
    do_not_optimize_away(output_eve_raw);
  });
  double avg_raw = std::accumulate(times_raw.begin(), times_raw.end(), 0.0) / NUM_ITERATIONS;
  std::cout << " \t(Speedup: " << avg_scalar / avg_raw << "x)\n";

  // --- Export to CSV ---
  if (generate_csv)
  {
    std::ofstream csv_file("data.csv");
    if (csv_file.is_open())
    {
      csv_file << "std,eve,eve_raw\n";
      for (int i = 0; i < NUM_ITERATIONS; ++i)
        csv_file << times_scalar[i] << "," << times_eve[i] << "," << times_raw[i] << "\n";
      csv_file.close();
      std::cout << "\n[Info] Raw iterations data exported to data.csv\n";
    }
    else
    {
      std::cerr << "\n[Error] Unable to open data.csv for writing.\n";
    }
  }
}