#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "html2md.h"
#include "md4c-html.h"
#include "table.h"

using std::cerr;
using std::cout;
using std::ifstream;
using std::string;
using std::stringstream;
using std::vector;
using std::chrono::duration;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;
namespace fs = std::filesystem;

// Markdown and HTML utility functions
namespace markdown {
void captureHtmlFragment(const MD_CHAR *data, const MD_SIZE data_size,
                         void *userData) {
  auto *str = static_cast<stringstream *>(userData);
  str->write(data, data_size);
}

string toHTML(const string &md) {
  stringstream html;
  static MD_TOC_OPTIONS options;
  md_html(md.c_str(), md.size(), &captureHtmlFragment, &html, MD_DIALECT_GITHUB,
          MD_HTML_FLAG_SKIP_UTF8_BOM, &options);
  return html.str();
}

string fromHTML(string &html) {
  static html2md::Options options;
  options.splitLines = false;
  html2md::Converter c(html, &options);
  return c.convert();
}
} // namespace markdown

// Benchmark result structure
struct BenchmarkResult {
  string test_name;
  double avg_time_us;     // Average time in microseconds
  double std_dev_us;      // Standard deviation in microseconds
  size_t input_size;      // Input size in bytes
  double throughput_mbps; // Throughput in megabytes per second
};

// Benchmark runner class
class BenchmarkRunner {
public:
  void addTest(const string &name, const string &input, bool is_markdown) {
    tests_.push_back({name, input, is_markdown});
  }

  void run(int iterations) {
    auto start_total = high_resolution_clock::now(); // Start total timer
    for (const auto &test : tests_) {
      runTest(test, iterations);
    }
    auto end_total = high_resolution_clock::now(); // End total timer
    total_duration_ms_ =
        duration<double, std::milli>(end_total - start_total).count();
    printSummary();
  }

private:
  struct Test {
    string name;
    string input;
    bool is_markdown; // false for HTML-to-Markdown
  };

  vector<Test> tests_;
  vector<BenchmarkResult> results_;
  double total_duration_ms_ = 0.0; // Total duration in milliseconds

  void runTest(const Test &test, int iterations) {
    vector<double> times_us(iterations);
    size_t input_size = test.input.size();

    for (int i = 0; i < iterations; ++i) {
      auto start = high_resolution_clock::now();
      if (test.is_markdown) {
        markdown::toHTML(test.input);
      } else {
        string input_copy = test.input; // fromHTML modifies input
        markdown::fromHTML(input_copy);
      }
      auto end = high_resolution_clock::now();
      times_us[i] = duration<double, std::micro>(end - start).count();
    }

    // Calculate average and standard deviation
    double sum = 0.0;
    for (double t : times_us)
      sum += t;
    double avg_time_us = sum / iterations;

    double variance_sum = 0.0;
    for (double t : times_us) {
      variance_sum += (t - avg_time_us) * (t - avg_time_us);
    }
    double std_dev_us = std::sqrt(variance_sum / iterations);

    // Calculate throughput (MB/s)
    double avg_time_s = avg_time_us / 1e6;
    double throughput_mbps = (input_size / (1024.0 * 1024.0)) / avg_time_s;

    results_.push_back(
        {test.name, avg_time_us, std_dev_us, input_size, throughput_mbps});
  }

  void printSummary() {
    cout << "\n=== Benchmark Summary ===\n";
    cout << std::left << std::setw(30) << "Test Name" << std::setw(15)
         << "Avg Time (us)" << std::setw(15) << "Std Dev (us)" << std::setw(15)
         << "Input Size (B)" << std::setw(15) << "Throughput (MB/s)\n";
    cout << std::string(90, '-') << "\n";

    for (const auto &result : results_) {
      cout << std::left << std::setw(30) << result.test_name << std::fixed
           << std::setprecision(2) << std::setw(15) << result.avg_time_us
           << std::setw(15) << result.std_dev_us << std::setw(15)
           << result.input_size << std::setw(15) << result.throughput_mbps
           << "\n";
    }

    cout << "\nTotal Benchmark Duration: " << std::fixed << std::setprecision(2)
         << total_duration_ms_ << " ms\n";
  }
};

namespace file {
string readAll(const string &name) {
  ifstream in(name);
  stringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}
} // namespace file

int main() {
  using namespace markdown;
  BenchmarkRunner runner;
  const int iterations = 10000; // Number of iterations per test

  // Add tests for Markdown files in the directory
  vector<string> files;
  static vector<string> markdownExtensions = {".md", ".markdown", ".mkd"};
  for (const auto &p : fs::recursive_directory_iterator(DIR)) {
    if (std::find(markdownExtensions.begin(), markdownExtensions.end(),
                  p.path().extension()) != markdownExtensions.end() &&
        p.path().parent_path() == DIR) {
      files.emplace_back(p.path().string());
    }
  }
  std::sort(files.begin(), files.end());

  for (const auto &file : files) {
    string md = file::readAll(file);
    string html = markdown::toHTML(md);
    string filename = fs::path(file).filename().string();
    runner.addTest(filename, html, false);
  }

  // Run benchmarks
  cout << "Running benchmarks with " << iterations
       << " iterations per test...\n";
  runner.run(iterations);

  return 0;
}