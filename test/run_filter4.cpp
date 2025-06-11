#include <chrono>
#include <cstdint>
#include <iostream>
#include <pthread.h>
#include <thread> 
#include <sched.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cstdlib>
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

// PMC Core Cycle Counter class
class CoreCycleCounter {
private:
    int fd;
    bool enabled;
    
public:
    CoreCycleCounter() : fd(-1), enabled(false) {
        struct perf_event_attr pe;
        memset(&pe, 0, sizeof(pe));
        pe.type = PERF_TYPE_HARDWARE;
        pe.config = PERF_COUNT_HW_CPU_CYCLES;
        pe.size = sizeof(pe);
        pe.disabled = 1;
        pe.exclude_kernel = 1;
        pe.exclude_hv = 1;
        pe.exclude_idle = 1;
        
        fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
        if (fd == -1) {
            perror("perf_event_open failed - you may need to run: echo 0 | sudo tee /proc/sys/kernel/perf_event_paranoid");
            std::cerr << "Note: Core cycle counting will be disabled, using TSC only\n";
            enabled = false;
        } else {
            enabled = true;
            std::cout << "PMC Core cycle counter initialized successfully\n";
        }
    }
    
    void start() { 
        if (enabled) {
            ioctl(fd, PERF_EVENT_IOC_RESET, 0); 
            ioctl(fd, PERF_EVENT_IOC_ENABLE, 0); 
        }
    }
    
    void stop() { 
        if (enabled) {
            ioctl(fd, PERF_EVENT_IOC_DISABLE, 0); 
        }
    }
    
    uint64_t read() {
        if (!enabled) return 0;
        
        uint64_t count;
        ssize_t result = ::read(fd, &count, sizeof(count));
        if (result != sizeof(count)) {
            perror("Failed to read PMC counter");
            return 0;
        }
        return count;
    }
    
    bool is_enabled() const { return enabled; }
    
    ~CoreCycleCounter() { 
        if (fd != -1) {
            close(fd); 
        }
    }
};

// Enhanced CPU frequency detection using run_filter2.cpp calibration method
double detect_cpu_frequency() {
    // Check for invariant TSC first
    bool is_invariant_tsc = false;
    std::ifstream cpuinfo_check("/proc/cpuinfo");
    if (cpuinfo_check.is_open()) {
        std::string line;
        while (std::getline(cpuinfo_check, line)) {
            if (line.find("flags") != std::string::npos && 
                line.find("constant_tsc") != std::string::npos &&
                line.find("nonstop_tsc") != std::string::npos) {
                is_invariant_tsc = true;
                std::cout << "Invariant TSC detected - TSC runs at constant frequency\n";
                break;
            }
        }
    }
    cpuinfo_check.close();

    // Show base frequency for reference (like run_filter2.cpp)
    std::ifstream base_file("/sys/devices/system/cpu/cpu0/cpufreq/base_frequency");
    if (base_file.is_open()) {
        std::string freq_str;
        std::getline(base_file, freq_str);
        if (!freq_str.empty()) {
            double base_freq = std::stod(freq_str) * 1000.0;
            std::cout << "Base CPU frequency: " << base_freq / 1e9 << " GHz\n";
        }
    }
    base_file.close();

    // Show max frequency for reference (like run_filter2.cpp)
    std::ifstream max_file("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    if (max_file.is_open()) {
        std::string freq_str;
        std::getline(max_file, freq_str);
        if (!freq_str.empty()) {
            double max_freq = std::stod(freq_str) * 1000.0;
            std::cout << "Max CPU frequency: " << max_freq / 1e9 << " GHz\n";
        }
    }
    max_file.close();

    // Force calibration (like run_filter2.cpp does) - this gives accurate TSC frequency
    std::cout << "Calibrating CPU frequency using TSC...\n";
    
    // Set performance governor for more consistent calibration
    std::system("echo performance | sudo tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor > /dev/null 2>&1");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Use exact calibration method from run_filter2.cpp
    const int calibration_runs = 5;
    double total_frequency = 0.0;
    
    for (int run = 0; run < calibration_runs; ++run) {
        auto start_time = std::chrono::high_resolution_clock::now();
        uint64_t start_tsc = rdtsc_begin();
        
        // Wait for 200ms (run_filter2.cpp uses this)
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
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
    
    if (is_invariant_tsc) {
        std::cout << "Note: Calibrated frequency is TSC frequency (constant), CPU may boost higher\n";
    }
    
    return avg_frequency;
}

using V = Vec8f;
using T = decltype(std::declval<V>().extract(0));
constexpr int L = V::size();
constexpr static int N = 32;
constexpr static int M = 8;

constexpr T b1 = 2, b2 = 1, a1 = 1.3, a2 = -0.4;
constexpr T xi1 = 2, xi2 = 1, yi1 = -3, yi2 = -5;

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

// scalar measurement cps V8
constexpr T measured_cps[4][1] = {27.614,31.990,47.428,87.669}; // filter order: 2,4,8,16, block size: 8,16,32,64,128

int main(){

    // Auto-detect CPU frequency
    const double TSC_HZ = detect_cpu_frequency();
    std::cout << "Using frequency for timing calculations: " << TSC_HZ / 1e9 << " GHz\n\n";

    // Initialize PMC counter
    CoreCycleCounter core_counter;
    std::cout << "\n";

    //–– Pin to core 0
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(0, &cpus);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpus), &cpus) != 0) {
        std::cerr << "Warning: failed to set CPU affinity\n";
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //–– Build filter & data
    alignas(64) std::array<T, vector_size> in{0}, out;
    in[0] = 1;
    auto _F = Filter<V,M,N>(coefs, inits);

    //–– Warm up caches/TLB/branch predictor
    for(int i = 0; i < WARMUP; ++i)
        _F(in.begin(), in.end(), out.begin());

    //–– Measure empty loop overhead
    double loop_overhead_ns = 0;
    uint64_t loop_overhead_tsc = 0;
    uint64_t loop_overhead_core = 0;
    
    {
        auto t0 = std::chrono::high_resolution_clock::now();
        uint64_t tsc0 = rdtsc_begin();
        core_counter.start();
        
        for(int i = 0; i < ITERS; ++i) { /* nothing */ }
        
        core_counter.stop();
        uint64_t tsc1 = rdtsc_end();
        auto t1 = std::chrono::high_resolution_clock::now();
        
        loop_overhead_ns = 
            double(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count())
            / ITERS;
        loop_overhead_tsc = (tsc1 - tsc0) / ITERS;
        if (core_counter.is_enabled()) {
            loop_overhead_core = core_counter.read() / ITERS;
        }
    }

    //–– Combined wall-clock, TSC, and core cycle timing (single measurement run)
    auto wc_start = std::chrono::high_resolution_clock::now();
    uint64_t tsc_start = rdtsc_begin();
    core_counter.start();
    
    for(int i = 0; i < ITERS; ++i)
        _F(in.begin(), in.end(), out.begin());
    
    core_counter.stop();
    uint64_t tsc_end = rdtsc_end();
    auto wc_end = std::chrono::high_resolution_clock::now();

    // Get core cycle count
    uint64_t core_cycles_total = 0;
    if (core_counter.is_enabled()) {
        core_cycles_total = core_counter.read();
    }

    // Wall-clock calculations
    double total_ns   = double(std::chrono::duration_cast<std::chrono::nanoseconds>(wc_end - wc_start).count());
    double avg_ns_raw = total_ns / ITERS;
    double avg_ns     = avg_ns_raw - loop_overhead_ns;

    // TSC calculations
    double total_tsc_cycles = double(tsc_end - tsc_start);
    double avg_tsc_cycles = total_tsc_cycles / ITERS - loop_overhead_tsc;
    double tsc_cycles_per_sample = avg_tsc_cycles / vector_size;
    double expected_ns_from_tsc = avg_tsc_cycles / (TSC_HZ/1e9);

    // Core cycle calculations
    double avg_core_cycles = 0;
    double core_cycles_per_sample = 0;
    double expected_ns_from_core = 0;
    
    if (core_counter.is_enabled()) {
        avg_core_cycles = double(core_cycles_total) / ITERS - loop_overhead_core;
        core_cycles_per_sample = avg_core_cycles / vector_size;
        // For core cycles, we don't know the exact core frequency, so we can't convert to ns accurately
    }

    //–– Report everything
    std::cout << "=== CONFIGURATION ===\n";
    std::cout << "vector_size = " << vector_size 
            << ", block_size = " << N
            << ", IIR filter order = " << 2*M
            << ", iterations = " << ITERS << "\n\n";

    std::cout << "=== TIMING RESULTS ===\n";
    std::cout << "Wall-clock timing:\n"
            << "  raw avg time:      " << avg_ns_raw      << " ns\n"
            << "  loop overhead:     " << loop_overhead_ns<< " ns\n"
            << "  adjusted avg time: " << avg_ns          << " ns\n\n";

    std::cout << "TSC timing:\n"
            << "  total TSC cycles:      " << total_tsc_cycles    << "\n"
            << "  avg TSC cycles/iter:   " << avg_tsc_cycles      << "\n"
            << "  avg TSC cycles/sample: " << tsc_cycles_per_sample << "\n"
            << "  expected avg time:     " << expected_ns_from_tsc << " ns"
            << "  (using TSC_HZ=" << (TSC_HZ/1e9) << " GHz)\n\n";

    if (core_counter.is_enabled()) {
        std::cout << "Core cycle timing (PMC):\n"
                << "  total core cycles:     " << core_cycles_total   << "\n"
                << "  avg core cycles/iter:  " << avg_core_cycles     << "\n"
                << "  avg core cycles/sample:" << core_cycles_per_sample << "\n"
                << "  (Note: Core frequency may be higher than TSC frequency due to boost)\n\n";
    } else {
        std::cout << "Core cycle timing: DISABLED (PMC not available)\n\n";
    }

    // Compare with predicted values
    unsigned ind_M = std::log2(M*2)-1;
    unsigned ind_N = std::log2(N)-3;
    if (ind_N >= 1) ind_N = 0; // scalar measurement only has 1 column

    std::cout << "=== COMPARISON WITH PREDICTIONS ===\n";
    std::cout << "Predicted cycles/sample (from measurements): " << measured_cps[ind_M][ind_N] << "\n";
    std::cout << "Expected total time: " << measured_cps[ind_M][ind_N]*vector_size / (TSC_HZ/1e9)  
            << " ns (using TSC_HZ=" << (TSC_HZ/1e9) << " GHz)\n\n";

    std::cout << "=== MEASUREMENT COMPARISON ===\n";
    std::cout << "Wall-clock vs TSC time difference: " 
            << std::abs(avg_ns - expected_ns_from_tsc) << " ns ("
            << std::abs(avg_ns - expected_ns_from_tsc) / avg_ns * 100 << "%)\n";
    
    std::cout << "TSC cycles/sample vs Predicted: " 
            << tsc_cycles_per_sample << " vs " << measured_cps[ind_M][ind_N] 
            << " (difference: " << std::abs(tsc_cycles_per_sample - measured_cps[ind_M][ind_N]) << ")\n";
    
    if (core_counter.is_enabled()) {
        std::cout << "Core cycles/sample vs Predicted: " 
                << core_cycles_per_sample << " vs " << measured_cps[ind_M][ind_N] 
                << " (difference: " << std::abs(core_cycles_per_sample - measured_cps[ind_M][ind_N]) << ")\n";
        
        std::cout << "TSC vs Core cycles ratio: " 
                << (core_cycles_per_sample / tsc_cycles_per_sample) << " (>1 means core is boosting)\n";
    }

    std::cout << "\n=== CONCLUSION ===\n";
    if (core_counter.is_enabled()) {
        std::cout << "Core cycle measurement should match PMCTest 'Core cyc' results\n";
        std::cout << "Core cycles/sample: " << core_cycles_per_sample << " (compare with PMCTest)\n";
    } else {
        std::cout << "Only TSC measurement available. To enable PMC:\n";
        std::cout << "  sudo echo 0 > /proc/sys/kernel/perf_event_paranoid\n";
    }

    return 0;
}
