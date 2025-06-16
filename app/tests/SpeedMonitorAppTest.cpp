/**
 * Copyright (c) 2023-2025 Contributors to the Eclipse Foundation
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License, Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../src/SpeedMonitorApp.h"

using namespace speedmonitor;
using namespace testing;

class SpeedMonitorAppTest : public Test {
protected:
    void SetUp() override {
        // Setup test environment
    }

    void TearDown() override {
        // Cleanup test environment
    }
};

TEST_F(SpeedMonitorAppTest, ConstructorInitializesDefaults) {
    // Test that constructor initializes with default values
    EXPECT_NO_THROW(SpeedMonitorApp testApp);
}

TEST_F(SpeedMonitorAppTest, SpeedConversion) {
    // Test m/s to km/h conversion
    EXPECT_DOUBLE_EQ(SpeedMonitorApp::msToKmh(10.0), 36.0);  // 10 m/s = 36 km/h
    EXPECT_DOUBLE_EQ(SpeedMonitorApp::msToKmh(25.0), 90.0);  // 25 m/s = 90 km/h
    EXPECT_DOUBLE_EQ(SpeedMonitorApp::msToKmh(0.0), 0.0);    // 0 m/s = 0 km/h
}

TEST_F(SpeedMonitorAppTest, ConfigurationDefaults) {
    SpeedConfig config;
    
    // Test default configuration values (now in m/s)
    EXPECT_EQ(config.speedLimitKmh, 22.0);  // 22 m/s (approx 80 km/h)
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}