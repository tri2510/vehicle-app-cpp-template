// ============================================================================
// 🧪 UNIT TESTS - EV Energy Management System
// ============================================================================

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class EVEnergyTest : public ::testing::Test {
protected:
    static constexpr double BATTERY_CAPACITY_KWH = 75.0;
    static constexpr double MIN_SOC_WARNING = 20.0;
    static constexpr double MIN_SOC_CRITICAL = 10.0;
    static constexpr double ENERGY_EFFICIENCY_TARGET = 150.0; // Wh/km
};

TEST_F(EVEnergyTest, BatterySOCValidation) {
    std::vector<double> socLevels = {100.0, 50.0, 20.0, 10.0, 5.0};
    
    for (double soc : socLevels) {
        bool isWarning = soc < MIN_SOC_WARNING;
        bool isCritical = soc < MIN_SOC_CRITICAL;
        
        if (soc < MIN_SOC_CRITICAL) {
            EXPECT_TRUE(isCritical) << "SOC " << soc << "% should trigger critical alert";
        } else if (soc < MIN_SOC_WARNING) {
            EXPECT_TRUE(isWarning) << "SOC " << soc << "% should trigger warning";
            EXPECT_FALSE(isCritical) << "SOC " << soc << "% should not be critical";
        }
    }
}

TEST_F(EVEnergyTest, RangePrediction) {
    struct RangeTest {
        double soc;           // %
        double efficiency;    // Wh/km
        double expectedRange; // km
    };
    
    std::vector<RangeTest> tests = {
        {100.0, 150.0, 500.0}, // Full battery, good efficiency
        {50.0, 150.0, 250.0},  // Half battery, good efficiency
        {100.0, 200.0, 375.0}, // Full battery, poor efficiency
        {25.0, 100.0, 187.5},  // Low battery, excellent efficiency
    };
    
    for (const auto& test : tests) {
        double availableEnergy = (test.soc / 100.0) * BATTERY_CAPACITY_KWH * 1000.0; // Wh
        double calculatedRange = availableEnergy / test.efficiency; // km
        
        EXPECT_NEAR(calculatedRange, test.expectedRange, 10.0) 
            << "Range prediction incorrect for SOC: " << test.soc << "%, efficiency: " << test.efficiency;
    }
}

TEST_F(EVEnergyTest, ChargingOptimization) {
    struct ChargingTest {
        double currentSOC;
        int timeOfDay;     // 24-hour format
        bool isOffPeak;
        bool shouldCharge;
    };
    
    std::vector<ChargingTest> tests = {
        {30.0, 2, true, true},   // Low SOC, off-peak hours
        {80.0, 2, true, false},  // High SOC, off-peak hours
        {30.0, 14, false, false}, // Low SOC, peak hours (wait)
        {15.0, 14, false, true},  // Very low SOC, peak hours (urgent)
    };
    
    for (const auto& test : tests) {
        bool urgentCharging = test.currentSOC < MIN_SOC_WARNING;
        bool shouldCharge = urgentCharging || (test.isOffPeak && test.currentSOC < 80.0);
        
        EXPECT_EQ(shouldCharge, test.shouldCharge) 
            << "Charging decision incorrect for SOC: " << test.currentSOC 
            << "%, time: " << test.timeOfDay;
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}