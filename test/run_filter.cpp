#include <chrono>
#include <cstdint>
#include <iostream>
#include <pthread.h>
#include <thread> 
#include <sched.h>
#include "../src/vcl/vectorclass.h"
#include "../include/filter.h"
#include <cmath>
#include <fstream>

static inline uint64_t rdtsc_begin() {
    unsigned hi, lo;
    asm volatile(
        "cpuid\n\t"        // full serialize
        "rdtsc\n\t"        // read TSC
        : "=a"(lo), "=d"(hi)
        :: "rbx", "rcx"
    );
    return (uint64_t(hi) << 32) | lo;
}

static inline uint64_t rdtsc_end() {
    unsigned hi, lo;
    asm volatile(
        "rdtscp\n\t"       // read TSC + serialize into ECX
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t"
        "cpuid\n\t"        // serialize again
        : "=r"(hi), "=r"(lo)
        :: "rax", "rbx", "rcx", "rdx"
    );
    return (uint64_t(hi) << 32) | lo;
}

// CPU frequency detection function
double detect_cpu_frequency() {
    // Method 1: Try /sys filesystem (most reliable for TSC)
    std::ifstream file("/sys/devices/system/cpu/cpu0/cpufreq/base_frequency");
    if (file.is_open()) {
        std::string freq_str;
        std::getline(file, freq_str);
        if (!freq_str.empty()) {
            double freq = std::stod(freq_str) * 1000.0; // Convert kHz to Hz
            std::cout << "CPU frequency detected from /sys/devices: " << freq / 1e9 << " GHz\n";
            return freq;
        }
    }
    
    // Method 2: Try /proc/cpuinfo
    file.close();
    file.open("/proc/cpuinfo");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("cpu MHz") != std::string::npos) {
                size_t colon_pos = line.find(':');
                if (colon_pos != std::string::npos) {
                    std::string freq_str = line.substr(colon_pos + 1);
                    // Remove whitespace
                    freq_str.erase(0, freq_str.find_first_not_of(" \t"));
                    freq_str.erase(freq_str.find_last_not_of(" \t") + 1);
                    if (!freq_str.empty()) {
                        double freq = std::stod(freq_str) * 1e6; // Convert MHz to Hz
                        std::cout << "CPU frequency detected from /proc/cpuinfo: " << freq / 1e9 << " GHz\n";
                        return freq;
                    }
                }
            }
        }
    }
    
    // Method 3: Fallback to calibration
    std::cout << "Calibrating CPU frequency using TSC...\n";
    
    // Perform multiple calibration runs for better accuracy
    const int calibration_runs = 5;
    double total_frequency = 0.0;
    
    for (int run = 0; run < calibration_runs; ++run) {
        auto start_time = std::chrono::high_resolution_clock::now();
        uint64_t start_tsc = rdtsc_begin();
        
        // Wait for 100ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        uint64_t end_tsc = rdtsc_end();
        auto end_time = std::chrono::high_resolution_clock::now();
        
        auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
        double elapsed_seconds = elapsed_ns.count() / 1e9;
        uint64_t tsc_cycles = end_tsc - start_tsc;
        
        double frequency = tsc_cycles / elapsed_seconds;
        total_frequency += frequency;
    }
    
    double avg_frequency = total_frequency / calibration_runs;
    std::cout << "CPU frequency calibrated: " << avg_frequency / 1e9 << " GHz\n";
    return avg_frequency;
}



using V = Vec8f;
using T = decltype(std::declval<V>().extract(0));
constexpr int L = V::size();
constexpr static int N = 8;
constexpr static int M = 8;

constexpr T b1 = 2, b2 = 1, a1 = 1.3, a2 = -0.4;
constexpr T xi1 = 2, xi2 = 1, yi1 = -3, yi2 = -5;

// constexpr T inits[M][4] = {xi1,xi2,yi1,yi2};
// constexpr T coefs[M][5] = {1,b1,b2,a1,a2};

// constexpr T inits[M][4] = {{xi1,xi2,yi1,yi2},
//                            {xi1,xi2,yi1,yi2}
//                           };
// constexpr T coefs[M][5] = {{1,b1,b2,a1,a2},
//                            {1,b1,b2,a1,a2}
//                           };

// constexpr T inits[M][4] = {{xi1,xi2,yi1,yi2},
//                            {xi1,xi2,yi1,yi2},
//                            {xi1,xi2,yi1,yi2},
//                            {xi1,xi2,yi1,yi2}
//                           };
// constexpr T coefs[M][5] = {{1,b1,b2,a1,a2},
//                            {1,b1,b2,a1,a2},
//                            {1,b1,b2,a1,a2},
//                            {1,b1,b2,a1,a2}
//                           };

constexpr T inits[M][4] = {{xi1,xi2,yi1,yi2},
                           {xi1,xi2,yi1,yi2},
                           {xi1,xi2,yi1,yi2},
                           {xi1,xi2,yi1,yi2},
                           {xi1,xi2,yi1,yi2},
                           {xi1,xi2,yi1,yi2},
                           {xi1,xi2,yi1,yi2},
                           {xi1,xi2,yi1,yi2}
                          };
constexpr T coefs[M][5] = {{1,b1,b2,a1,a2},
                           {1,b1,b2,a1,a2},
                           {1,b1,b2,a1,a2},
                           {1,b1,b2,a1,a2},
                           {1,b1,b2,a1,a2},
                           {1,b1,b2,a1,a2},
                           {1,b1,b2,a1,a2},
                           {1,b1,b2,a1,a2}
                          };


// match your large-array workload
constexpr static int vector_size = 131072;

// number of runs to average
constexpr int ITERS  = 10000;
constexpr int WARMUP = 200;


// // PH measurement cps V8
// constexpr T measured_cps[4][7] = {2.766,2.301,2.214,2.469,2.713,3.026,3.752,
//                                   4.447,3.666,3.451,3.990,4.056,4.354,5.374,
//                                   7.868,6.280,5.890,6.324,6.577,7.586,9.064,
//                                   14.509,11.505,10.631,11.273,11.721,15.154,16.179
//                                 }; // filter order: 2,4,8,16, block size: 8,16,32,64,128,256,512

// // CR measurement cps V8
// constexpr T measured_cps[4][7] = {3.156,2.324,2.007,2.101,2.252,2.501,2.916,
//                                   5.320,3.657,3.011,3.088,3.270,3.468,4.389,
//                                   9.535,6.407,5.044,5.067,5.158,5.426,8.611,
//                                   18.128,11.833,9.065,9.079,10.224,13.587,16.157    
//                                 }; // filter order: 2,4,8,16, block size: 8,16,32,64,128

// // BF measurement cps V8
// constexpr T measured_cps[4][1] = {1.785,5.580,17.802,49.280}; // filter order: 2,4,8,16, block size: 8,16,32,64,128

// scalar measurement cps V8
constexpr T measured_cps[4][1] = {27.614,31.990,47.428,87.669}; // filter order: 2,4,8,16, block size: 8,16,32,64,128

int main(){

    // Auto-detect CPU frequency
    const double TSC_HZ = detect_cpu_frequency();
    std::cout << "Using TSC frequency: " << TSC_HZ / 1e9 << " GHz\n\n";

    //–– Pin to core 0
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(0, &cpus);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpus), &cpus) != 0) {
        std::cerr << "Warning: failed to set CPU affinity\n";
    }

    //–– Build filter & data
    alignas(64) std::array<T, vector_size> in{0}, out;
    in[0] = 1;
    auto _F = Filter<V,M,N>(coefs, inits);

    //–– Warm up caches/TLB/branch predictor
    for(int i = 0; i < WARMUP; ++i)
        _F(in.begin(), in.end(), out.begin());

    //–– Measure empty loop overhead in ns
    double loop_overhead_ns = 0;
    {
        auto t0 = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < ITERS; ++i) { /* nothing */ }
        auto t1 = std::chrono::high_resolution_clock::now();
        loop_overhead_ns = 
        double(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count())
        / ITERS;
    }

    //–– Combined wall-clock and cycle-count timing (single measurement run)
    auto wc_start = std::chrono::high_resolution_clock::now();
    uint64_t c_start = rdtsc_begin();
    
    for(int i = 0; i < ITERS; ++i)
        _F(in.begin(), in.end(), out.begin());
    
    uint64_t c_end = rdtsc_end();
    auto wc_end = std::chrono::high_resolution_clock::now();

    // Wall-clock calculations
    double total_ns   = double(std::chrono::duration_cast<std::chrono::nanoseconds>(wc_end - wc_start).count());
    double avg_ns_raw = total_ns / ITERS;
    double avg_ns     = avg_ns_raw - loop_overhead_ns;

    // Cycle-count calculations
    double total_cycles   = double(c_end - c_start);
    double avg_cycles     = total_cycles / ITERS;
    double cycles_per_sample = avg_cycles / vector_size;
    double expected_ns    = avg_cycles / (TSC_HZ/1e9);

    //–– Report everything
    std::cout << "vector_size = " << vector_size 
            << ", block_size = " << N
            << ", IIR filter order = " << 2*M
            << ", iterations = " << ITERS << "\n\n";

    std::cout << "Wall-clock timing:\n"
            << "  raw avg time:      " << avg_ns_raw      << " ns\n"
            << "  loop overhead:     " << loop_overhead_ns<< " ns\n"
            << "  adjusted avg time: " << avg_ns          << " ns\n\n";

    std::cout << "Cycle-count timing:\n"
            << "  total cycles:      " << total_cycles    << "\n"
            << "  avg cycles/iter:   " << avg_cycles      << "\n"
            << "  avg cycles/sample: " << cycles_per_sample << "\n"
            << "  expected avg time: " << expected_ns     << " ns"
            << "  (using TSC_HZ=" << (TSC_HZ/1e9) << " GHz)\n\n";

    unsigned ind_M = std::log2(M*2)-1;
    unsigned ind_N = std::log2(N)-3;

    // unsigned ind_M = std::log2(M*2)-1;
    // unsigned ind_N = std::log2(N)-4;

    std::cout << "Manual computed timing:\n"
            << "  cycles per sample:      " << measured_cps[ind_M][ind_N]    << "\n"
            << "  expected computed time: " << measured_cps[ind_M][ind_N]*vector_size / (TSC_HZ/1e9)  
                << " ns"
            << "  (using TSC_HZ=" << (TSC_HZ/1e9) << " GHz)\n\n";

    // Comparison between measurements
    std::cout << "Measurement comparison:\n"
            << "  Wall-clock vs Cycle-count time difference: " 
            << std::abs(avg_ns - expected_ns) << " ns ("
            << std::abs(avg_ns - expected_ns) / avg_ns * 100 << "%)\n"
            << "  Measured vs Predicted cycles/sample: " 
            << cycles_per_sample << " vs " << measured_cps[ind_M][ind_N] 
            << " (difference: " << std::abs(cycles_per_sample - measured_cps[ind_M][ind_N]) << ")\n";

    return 0;
}