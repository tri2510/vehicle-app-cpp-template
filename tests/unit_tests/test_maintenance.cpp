// ============================================================================
// 🧪 UNIT TESTS - Predictive Maintenance System
// ============================================================================

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class MaintenanceTest : public ::testing::Test {
protected:
    static constexpr double ENGINE_TEMP_WARNING = 95.0;
    static constexpr double ENGINE_TEMP_CRITICAL = 105.0;
    static constexpr double HEALTH_SCORE_WARNING = 70.0;
    static constexpr double HEALTH_SCORE_CRITICAL = 50.0;
    static constexpr double MAINTENANCE_WARNING_THRESHOLD = 5000.0;
};

TEST_F(MaintenanceTest, ComponentHealthScoring) {
    struct HealthTest {
        double engineTemp;
        double engineLoad;
        double expectedHealthImpact;
        std::string severity;
    };
    
    std::vector<HealthTest> tests = {
        {85.0, 50.0, 0.0, "NORMAL"},
        {98.0, 70.0, -0.5, "WARNING"},
        {108.0, 90.0, -2.0, "CRITICAL"},
        {75.0, 30.0, 0.0, "NORMAL"},
    };
    
    for (const auto& test : tests) {
        double healthImpact = 0.0;
        
        if (test.engineTemp > ENGINE_TEMP_CRITICAL) {
            healthImpact = -2.0;
        } else if (test.engineTemp > ENGINE_TEMP_WARNING) {
            healthImpact = -0.5;
        }
        
        EXPECT_NEAR(healthImpact, test.expectedHealthImpact, 0.1) 
            << "Health impact calculation incorrect for temp: " << test.engineTemp;
    }
}

TEST_F(MaintenanceTest, PredictiveFailureDetection) {
    struct PredictionTest {
        double currentHealth;
        double degradationRate; // % per 1000km
        double currentMileage;
        double expectedFailureKm;
    };
    
    std::vector<PredictionTest> tests = {
        {80.0, 0.1, 50000.0, 250000.0}, // Good health, slow degradation
        {60.0, 0.5, 30000.0, 150000.0}, // Moderate health, faster degradation
        {40.0, 1.0, 80000.0, 120000.0}, // Poor health, fast degradation
    };
    
    for (const auto& test : tests) {
        double kmToFailure = test.currentHealth / test.degradationRate * 1000.0;
        double predictedFailureKm = test.currentMileage + kmToFailure;
        
        EXPECT_NEAR(predictedFailureKm, test.expectedFailureKm, 10000.0) 
            << "Failure prediction incorrect for health: " << test.currentHealth 
            << "%, degradation: " << test.degradationRate;
    }
}

TEST_F(MaintenanceTest, MaintenanceScheduling) {
    std::vector<double> distancesToService = {10000.0, 3000.0, 1000.0, 500.0, 0.0};
    
    for (double distance : distancesToService) {
        bool shouldSchedule = distance < MAINTENANCE_WARNING_THRESHOLD;
        bool isOverdue = distance <= 0.0;
        
        if (distance <= 0.0) {
            EXPECT_TRUE(isOverdue) << "Should be marked as overdue";
        } else if (distance < MAINTENANCE_WARNING_THRESHOLD) {
            EXPECT_TRUE(shouldSchedule) << "Should schedule maintenance";
        }
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}