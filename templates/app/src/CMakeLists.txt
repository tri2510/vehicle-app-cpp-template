# Copyright (c) 2022-2025 Contributors to the Eclipse Foundation
#
# This program and the accompanying materials are made available under the
# terms of the Apache License, Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0.
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.
#
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME "app")

add_executable(${TARGET_NAME}
    VehicleApp.cpp
)

target_include_directories(${TARGET_NAME}
    PRIVATE
    .
)

target_link_libraries(${TARGET_NAME}
    ${SERVICE_LIBS}
    vehicle-app-sdk::vehicle-app-sdk
    vehicle-model::vehicle-model
    nlohmann_json::nlohmann_json
)