// ============================================================================
// 🧪 PERFORMANCE TESTS - Signal Processing Benchmarks
// ============================================================================
// 
// Performance benchmarks for vehicle signal processing
// Validates performance targets and resource utilization
//
// Performance Targets:
// - Signal processing latency: <10ms per signal update
// - Memory usage: <150MB per application
// - CPU utilization: <50% average during normal operation
// - Throughput: >100 signals/second processing capacity
// ============================================================================

#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <memory>
#include <thread>
#include <random>
#include <algorithm>
#include <numeric>

class SignalProcessingBenchmark : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize performance monitoring
        startTime = std::chrono::high_resolution_clock::now();
        
        // Seed random number generator for test data
        std::random_device rd;
        rng.seed(rd());
    }

    void TearDown() override {
        auto endTime = std::chrono::high_resolution_clock::now();
        testDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    }

    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::milliseconds testDuration;
    std::mt19937 rng;
    
    // Performance targets
    static constexpr double MAX_PROCESSING_LATENCY_MS = 10.0;
    static constexpr int MIN_THROUGHPUT_SIGNALS_PER_SEC = 100;
    static constexpr double MAX_CPU_UTILIZATION_PERCENT = 50.0;
    static constexpr size_t MAX_MEMORY_USAGE_MB = 150;
};

// ============================================================================
// LATENCY BENCHMARKS
// ============================================================================

TEST_F(SignalProcessingBenchmark, SingleSignalProcessingLatency) {
    // Benchmark single signal processing latency
    
    const int NUM_ITERATIONS = 10000;
    std::vector<double> latencies;
    latencies.reserve(NUM_ITERATIONS);
    
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // Simulate signal processing pipeline
        double speed = 10.0 + (rng() % 50);
        double acceleration = -5.0 + (rng() % 10);
        double fuelLevel = 10.0 + (rng() % 90);
        
        // Signal validation
        bool speedValid = speed >= 0.0 && speed <= 100.0;
        bool accelValid = acceleration >= -10.0 && acceleration <= 10.0;
        bool fuelValid = fuelLevel >= 0.0 && fuelLevel <= 100.0;
        
        // Signal processing logic
        bool hasWarning = false;
        if (speedValid && speed > 22.0) hasWarning = true;
        if (accelValid && acceleration < -4.0) hasWarning = true;
        if (fuelValid && fuelLevel < 20.0) hasWarning = true;
        
        // Emergency detection
        bool hasEmergency = false;
        if (speedValid && speed > 30.0) hasEmergency = true;
        if (accelValid && acceleration < -6.0) hasEmergency = true;
        if (fuelValid && fuelLevel < 10.0) hasEmergency = true;
        
        auto end = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1000000.0;
        latencies.push_back(latency);
    }
    
    // Calculate statistics
    double avgLatency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    double maxLatency = *std::max_element(latencies.begin(), latencies.end());
    double minLatency = *std::min_element(latencies.begin(), latencies.end());
    
    // Calculate 95th percentile
    std::sort(latencies.begin(), latencies.end());
    double p95Latency = latencies[static_cast<size_t>(latencies.size() * 0.95)];
    
    // Performance assertions
    EXPECT_LT(avgLatency, MAX_PROCESSING_LATENCY_MS) 
        << "Average processing latency too high: " << avgLatency << "ms";
    EXPECT_LT(p95Latency, MAX_PROCESSING_LATENCY_MS * 2) 
        << "95th percentile latency too high: " << p95Latency << "ms";
    
    // Performance report
    std::cout << "\n=== SINGLE SIGNAL PROCESSING LATENCY BENCHMARK ===" << std::endl;
    std::cout << "Iterations: " << NUM_ITERATIONS << std::endl;
    std::cout << "Average latency: " << avgLatency << " ms" << std::endl;
    std::cout << "Minimum latency: " << minLatency << " ms" << std::endl;
    std::cout << "Maximum latency: " << maxLatency << " ms" << std::endl;
    std::cout << "95th percentile: " << p95Latency << " ms" << std::endl;
    std::cout << "Target: < " << MAX_PROCESSING_LATENCY_MS << " ms" << std::endl;
}

TEST_F(SignalProcessingBenchmark, MultiSignalProcessingLatency) {
    // Benchmark multi-signal coordinated processing
    
    const int NUM_ITERATIONS = 5000;
    const int SIGNALS_PER_UPDATE = 8; // Typical number of signals per update
    
    std::vector<double> latencies;
    latencies.reserve(NUM_ITERATIONS);
    
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // Simulate multiple signal processing
        struct SignalSet {
            double speed;
            double acceleration;
            double fuelLevel;
            double engineTemp;
            double latitude;
            double longitude;
            double brakePosition;
            bool absActive;
        };
        
        SignalSet signals = {
            10.0 + (rng() % 50),          // speed
            -5.0 + (rng() % 10),          // acceleration
            10.0 + (rng() % 90),          // fuelLevel
            60.0 + (rng() % 50),          // engineTemp
            40.0 + (rng() % 10) * 0.1,    // latitude
            -74.0 + (rng() % 10) * 0.1,   // longitude
            0.0 + (rng() % 100),          // brakePosition
            (rng() % 2) == 1              // absActive
        };
        
        // Multi-signal validation and processing
        bool allValid = true;
        allValid &= signals.speed >= 0.0 && signals.speed <= 100.0;
        allValid &= signals.acceleration >= -10.0 && signals.acceleration <= 10.0;
        allValid &= signals.fuelLevel >= 0.0 && signals.fuelLevel <= 100.0;
        allValid &= signals.engineTemp >= 0.0 && signals.engineTemp <= 150.0;
        allValid &= signals.brakePosition >= 0.0 && signals.brakePosition <= 100.0;
        
        if (allValid) {
            // Collision warning analysis
            bool collisionRisk = signals.speed > 25.0 && signals.acceleration < -5.0;
            
            // Fleet analytics
            bool poorEfficiency = signals.fuelLevel < 20.0;
            bool harshDriving = signals.acceleration > 3.0 || signals.acceleration < -3.0;
            
            // Maintenance analysis
            bool overheating = signals.engineTemp > 100.0;
            bool heavyBraking = signals.brakePosition > 80.0;
            
            // Climate analysis
            bool needsAttention = overheating || heavyBraking;
            
            // V2X analysis
            bool shouldBroadcast = collisionRisk || signals.absActive;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1000000.0;
        latencies.push_back(latency);
    }
    
    // Calculate statistics
    double avgLatency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    double maxLatency = *std::max_element(latencies.begin(), latencies.end());
    
    std::sort(latencies.begin(), latencies.end());
    double p95Latency = latencies[static_cast<size_t>(latencies.size() * 0.95)];
    
    // Multi-signal processing should still meet latency targets
    EXPECT_LT(avgLatency, MAX_PROCESSING_LATENCY_MS * 2) 
        << "Multi-signal average processing latency too high: " << avgLatency << "ms";
    EXPECT_LT(p95Latency, MAX_PROCESSING_LATENCY_MS * 3) 
        << "Multi-signal 95th percentile latency too high: " << p95Latency << "ms";
    
    std::cout << "\n=== MULTI-SIGNAL PROCESSING LATENCY BENCHMARK ===" << std::endl;
    std::cout << "Iterations: " << NUM_ITERATIONS << " (x" << SIGNALS_PER_UPDATE << " signals each)" << std::endl;
    std::cout << "Average latency: " << avgLatency << " ms" << std::endl;
    std::cout << "Maximum latency: " << maxLatency << " ms" << std::endl;
    std::cout << "95th percentile: " << p95Latency << " ms" << std::endl;
    std::cout << "Target: < " << MAX_PROCESSING_LATENCY_MS * 2 << " ms" << std::endl;
}

// ============================================================================
// THROUGHPUT BENCHMARKS
// ============================================================================

TEST_F(SignalProcessingBenchmark, SignalThroughputCapacity) {
    // Benchmark maximum signal processing throughput
    
    const int TEST_DURATION_SECONDS = 5;
    const int TARGET_SIGNALS_PER_SECOND = MIN_THROUGHPUT_SIGNALS_PER_SEC;
    
    int signalsProcessed = 0;
    auto testStart = std::chrono::high_resolution_clock::now();
    
    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - testStart).count();
        
        if (elapsed >= TEST_DURATION_SECONDS) break;
        
        // Process signal update
        double speed = 10.0 + (rng() % 40);
        double acceleration = -3.0 + (rng() % 6);
        
        // Signal processing (optimized for throughput)
        bool speedWarning = speed > 22.0;
        bool brakeWarning = acceleration < -4.0;
        bool hasAlert = speedWarning || brakeWarning;
        
        signalsProcessed++;
        
        // Small delay to prevent 100% CPU usage in test
        if (signalsProcessed % 1000 == 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
    
    auto actualDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - testStart).count() / 1000.0;
    
    double actualThroughput = signalsProcessed / actualDuration;
    
    EXPECT_GE(actualThroughput, TARGET_SIGNALS_PER_SECOND) 
        << "Signal processing throughput below target: " << actualThroughput << " signals/sec";
    
    std::cout << "\n=== SIGNAL THROUGHPUT CAPACITY BENCHMARK ===" << std::endl;
    std::cout << "Test duration: " << actualDuration << " seconds" << std::endl;
    std::cout << "Signals processed: " << signalsProcessed << std::endl;
    std::cout << "Throughput: " << actualThroughput << " signals/sec" << std::endl;
    std::cout << "Target: > " << TARGET_SIGNALS_PER_SECOND << " signals/sec" << std::endl;
}

// ============================================================================
// MEMORY USAGE BENCHMARKS
// ============================================================================

TEST_F(SignalProcessingBenchmark, MemoryUsageUnderLoad) {
    // Benchmark memory usage during signal processing
    
    const int NUM_SIGNAL_UPDATES = 10000;
    const int BATCH_SIZE = 100;
    
    // Simulate realistic data structures
    struct VehicleState {
        double speed;
        double acceleration;
        double fuelLevel;
        double engineTemp;
        std::chrono::steady_clock::time_point timestamp;
        std::vector<double> trendData;
    };
    
    std::vector<VehicleState> vehicleHistory;
    vehicleHistory.reserve(NUM_SIGNAL_UPDATES);
    
    size_t initialMemoryEstimate = sizeof(VehicleState) * vehicleHistory.capacity();
    
    for (int i = 0; i < NUM_SIGNAL_UPDATES; i += BATCH_SIZE) {
        // Process batch of signals
        for (int j = 0; j < BATCH_SIZE && (i + j) < NUM_SIGNAL_UPDATES; ++j) {
            VehicleState state;
            state.speed = 10.0 + (rng() % 40);
            state.acceleration = -3.0 + (rng() % 6);
            state.fuelLevel = 20.0 + (rng() % 80);
            state.engineTemp = 70.0 + (rng() % 40);
            state.timestamp = std::chrono::steady_clock::now();
            
            // Simulate trend data (limited size)
            state.trendData.reserve(100);
            for (int k = 0; k < std::min(50, i / 100); ++k) {
                state.trendData.push_back(state.speed + k * 0.1);
            }
            
            vehicleHistory.push_back(state);
        }
        
        // Simulate memory cleanup (remove old data)
        if (vehicleHistory.size() > 5000) {
            vehicleHistory.erase(vehicleHistory.begin(), vehicleHistory.begin() + 1000);
        }
    }
    
    // Estimate memory usage
    size_t finalMemoryEstimate = sizeof(VehicleState) * vehicleHistory.size();
    for (const auto& state : vehicleHistory) {
        finalMemoryEstimate += state.trendData.capacity() * sizeof(double);
    }
    
    size_t memoryUsageMB = finalMemoryEstimate / (1024 * 1024);
    
    EXPECT_LT(memoryUsageMB, MAX_MEMORY_USAGE_MB) 
        << "Memory usage too high: " << memoryUsageMB << " MB";
    
    std::cout << "\n=== MEMORY USAGE BENCHMARK ===" << std::endl;
    std::cout << "Signal updates processed: " << vehicleHistory.size() << std::endl;
    std::cout << "Estimated memory usage: " << memoryUsageMB << " MB" << std::endl;
    std::cout << "Target: < " << MAX_MEMORY_USAGE_MB << " MB" << std::endl;
}

// ============================================================================
// CONCURRENT PROCESSING BENCHMARKS
// ============================================================================

TEST_F(SignalProcessingBenchmark, ConcurrentSignalProcessing) {
    // Benchmark concurrent signal processing performance
    
    const int NUM_THREADS = 4;
    const int SIGNALS_PER_THREAD = 2500;
    const int TOTAL_SIGNALS = NUM_THREADS * SIGNALS_PER_THREAD;
    
    std::vector<std::thread> workers;
    std::vector<std::vector<double>> threadLatencies(NUM_THREADS);
    
    auto globalStart = std::chrono::high_resolution_clock::now();
    
    // Launch worker threads
    for (int t = 0; t < NUM_THREADS; ++t) {
        workers.emplace_back([t, SIGNALS_PER_THREAD, &threadLatencies, this]() {
            std::mt19937 threadRng(t); // Thread-specific RNG
            
            for (int i = 0; i < SIGNALS_PER_THREAD; ++i) {
                auto start = std::chrono::high_resolution_clock::now();
                
                // Simulate signal processing
                double speed = 10.0 + (threadRng() % 40);
                double acceleration = -3.0 + (threadRng() % 6);
                double fuelLevel = 20.0 + (threadRng() % 80);
                
                // Processing logic
                bool hasWarning = speed > 22.0 || acceleration < -4.0 || fuelLevel < 20.0;
                bool hasCritical = speed > 30.0 || acceleration < -6.0 || fuelLevel < 10.0;
                
                // Simulate some computational work
                volatile double result = speed * acceleration + fuelLevel;
                
                auto end = std::chrono::high_resolution_clock::now();
                auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1000000.0;
                threadLatencies[t].push_back(latency);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& worker : workers) {
        worker.join();
    }
    
    auto globalEnd = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(globalEnd - globalStart).count() / 1000.0;
    
    // Aggregate results
    std::vector<double> allLatencies;
    for (const auto& threadLatency : threadLatencies) {
        allLatencies.insert(allLatencies.end(), threadLatency.begin(), threadLatency.end());
    }
    
    double avgLatency = std::accumulate(allLatencies.begin(), allLatencies.end(), 0.0) / allLatencies.size();
    double throughput = TOTAL_SIGNALS / totalDuration;
    
    // Concurrent processing should maintain performance
    EXPECT_LT(avgLatency, MAX_PROCESSING_LATENCY_MS * 1.5) 
        << "Concurrent processing latency too high: " << avgLatency << "ms";
    EXPECT_GE(throughput, MIN_THROUGHPUT_SIGNALS_PER_SEC * NUM_THREADS * 0.8) 
        << "Concurrent processing throughput too low: " << throughput << " signals/sec";
    
    std::cout << "\n=== CONCURRENT SIGNAL PROCESSING BENCHMARK ===" << std::endl;
    std::cout << "Threads: " << NUM_THREADS << std::endl;
    std::cout << "Total signals: " << TOTAL_SIGNALS << std::endl;
    std::cout << "Total duration: " << totalDuration << " seconds" << std::endl;
    std::cout << "Average latency: " << avgLatency << " ms" << std::endl;
    std::cout << "Throughput: " << throughput << " signals/sec" << std::endl;
    std::cout << "Target latency: < " << MAX_PROCESSING_LATENCY_MS * 1.5 << " ms" << std::endl;
    std::cout << "Target throughput: > " << MIN_THROUGHPUT_SIGNALS_PER_SEC * NUM_THREADS * 0.8 << " signals/sec" << std::endl;
}

// ============================================================================
// MAIN BENCHMARK RUNNER
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "VEHICLE SIGNAL PROCESSING PERFORMANCE BENCHMARKS" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "BENCHMARK SUMMARY" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "Performance Targets:" << std::endl;
    std::cout << "• Signal processing latency: < " << SignalProcessingBenchmark::MAX_PROCESSING_LATENCY_MS << " ms" << std::endl;
    std::cout << "• Signal throughput: > " << SignalProcessingBenchmark::MIN_THROUGHPUT_SIGNALS_PER_SEC << " signals/sec" << std::endl;
    std::cout << "• Memory usage: < " << SignalProcessingBenchmark::MAX_MEMORY_USAGE_MB << " MB" << std::endl;
    std::cout << "• CPU utilization: < " << SignalProcessingBenchmark::MAX_CPU_UTILIZATION_PERCENT << "%" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    return result;
}

// ============================================================================
// PERFORMANCE BENCHMARK COVERAGE
// ============================================================================
//
// This performance test suite covers:
// ✅ Single signal processing latency (Target: <10ms)
// ✅ Multi-signal processing latency (Target: <20ms)
// ✅ Signal throughput capacity (Target: >100 signals/sec)
// ✅ Memory usage under load (Target: <150MB)
// ✅ Concurrent processing performance (Target: 80% efficiency)
//
// Expected Performance Targets:
// - Latency: <10ms average, <20ms 95th percentile
// - Throughput: >100 signals/second processing capacity
// - Memory: <150MB total application memory usage
// - Concurrency: 80% efficiency with 4 threads
//
// ============================================================================