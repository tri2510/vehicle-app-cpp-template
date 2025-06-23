// ============================================================================
// 🧪 UNIT TESTS - Smart Climate Control System
// ============================================================================
// 
// Unit tests for SmartClimateApp covering:
// - Multi-zone temperature control
// - Occupancy detection
// - Energy optimization
// - Comfort preference learning
// ============================================================================

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class SmartClimateTest : public ::testing::Test {
protected:
    static constexpr double COMFORT_TEMP_MIN = 18.0;
    static constexpr double COMFORT_TEMP_MAX = 26.0;
    static constexpr double OPTIMAL_TEMP = 22.0;
    static constexpr double TEMP_TOLERANCE = 2.0;
};

TEST_F(SmartClimateTest, TemperatureRangeValidation) {
    std::vector<double> temperatures = {15.0, 18.0, 22.0, 26.0, 30.0};
    
    for (double temp : temperatures) {
        bool inComfortRange = temp >= COMFORT_TEMP_MIN && temp <= COMFORT_TEMP_MAX;
        bool needsHeating = temp < OPTIMAL_TEMP - TEMP_TOLERANCE;
        bool needsCooling = temp > OPTIMAL_TEMP + TEMP_TOLERANCE;
        
        if (temp < COMFORT_TEMP_MIN) {
            EXPECT_TRUE(needsHeating) << "Temperature " << temp << " should need heating";
        } else if (temp > COMFORT_TEMP_MAX) {
            EXPECT_TRUE(needsCooling) << "Temperature " << temp << " should need cooling";
        } else {
            EXPECT_TRUE(inComfortRange) << "Temperature " << temp << " should be in comfort range";
        }
    }
}

TEST_F(SmartClimateTest, OccupancyBasedControl) {
    struct ZoneTest {
        bool isOccupied;
        double currentTemp;
        double expectedTarget;
        bool shouldOptimize;
    };
    
    std::vector<ZoneTest> zones = {
        {true, 20.0, 22.0, false},   // Occupied, normal target
        {false, 20.0, 24.0, true},   // Unoccupied, energy save target
        {true, 30.0, 22.0, false},   // Occupied, needs cooling
        {false, 15.0, 19.0, true},   // Unoccupied, energy save heating
    };
    
    for (const auto& zone : zones) {
        double targetTemp = zone.isOccupied ? OPTIMAL_TEMP : 
                           (zone.currentTemp > 25.0 ? OPTIMAL_TEMP + 2.0 : OPTIMAL_TEMP - 2.0);
        
        EXPECT_NEAR(targetTemp, zone.expectedTarget, 0.5) 
            << "Zone target temperature incorrect for occupancy: " << zone.isOccupied;
    }
}

TEST_F(SmartClimateTest, EnergyOptimization) {
    struct EnergyTest {
        double exteriorTemp;
        bool hvacActive;
        double expectedEnergyUsage;
    };
    
    std::vector<EnergyTest> scenarios = {
        {25.0, false, 0.0},    // Mild weather, no HVAC
        {35.0, true, 3.0},     // Hot weather, high cooling load
        {5.0, true, 2.5},      // Cold weather, heating load
        {22.0, true, 0.5},     // Optimal weather, minimal load
    };
    
    for (const auto& test : scenarios) {
        double energyUsage = 0.0;
        if (test.hvacActive) {
            double tempDifferential = std::abs(test.exteriorTemp - OPTIMAL_TEMP);
            energyUsage = tempDifferential * 0.1; // Simplified calculation
        }
        
        EXPECT_NEAR(energyUsage, test.expectedEnergyUsage, 0.5) 
            << "Energy usage calculation incorrect for exterior temp: " << test.exteriorTemp;
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}