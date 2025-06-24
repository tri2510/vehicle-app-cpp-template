#!/bin/bash
# ============================================================================
# Velocitas CLI - Unified Build, Test, and Run System
# ============================================================================
# Purpose: Single entry point for all vehicle app operations
# Usage: 
#   docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --verbose
#   docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick run 
#   docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick test --signal-validation
# ============================================================================

set -e

# Configuration
WORKSPACE="/quickbuild"
APP_SOURCE="$WORKSPACE/app/src/VehicleApp.cpp"
BUILD_DIR="$WORKSPACE/build"
LOG_FILE="/tmp/velocitas.log"
DEFAULT_TIMEOUT=60

# Global flags
VERBOSE=false
CLEAN=false
SKIP_DEPS=false
SKIP_VSS=false
FORCE_REBUILD=false
QUIET=false

# Initialize log
echo "🚀 Velocitas CLI Started: $(date)" > "$LOG_FILE"

# ============================================================================
# LOGGING FUNCTIONS
# ============================================================================

log_info() {
    local msg="ℹ️  [INFO] $1"
    if [ "$QUIET" = false ]; then
        echo "$msg" | tee -a "$LOG_FILE"
    else
        echo "$msg" >> "$LOG_FILE"
    fi
}

log_success() {
    local msg="✅ [SUCCESS] $1"
    if [ "$QUIET" = false ]; then
        echo "$msg" | tee -a "$LOG_FILE"
    else
        echo "$msg" >> "$LOG_FILE"
    fi
}

log_error() {
    local msg="❌ [ERROR] $1"
    echo "$msg" | tee -a "$LOG_FILE" >&2
}

log_warning() {
    local msg="⚠️  [WARNING] $1"
    if [ "$QUIET" = false ]; then
        echo "$msg" | tee -a "$LOG_FILE"
    else
        echo "$msg" >> "$LOG_FILE"
    fi
}

log_debug() {
    if [ "$VERBOSE" = true ]; then
        local msg="🔍 [DEBUG] $1"
        echo "$msg" | tee -a "$LOG_FILE"
    else
        echo "🔍 [DEBUG] $1" >> "$LOG_FILE"
    fi
}

# Unified logging function for commands
run_command() {
    local command="$1"
    local success_msg="$2"
    local error_msg="$3"
    
    log_debug "Executing: $command"
    
    if [ "$VERBOSE" = true ]; then
        if eval "$command" 2>&1 | tee -a "$LOG_FILE"; then
            log_success "$success_msg"
            return 0
        else
            local exit_code=$?
            log_error "$error_msg"
            return $exit_code
        fi
    else
        if eval "$command" >> "$LOG_FILE" 2>&1; then
            log_success "$success_msg"
            return 0
        else
            local exit_code=$?
            log_error "$error_msg"
            log_info "💡 Use --verbose to see detailed output"
            return $exit_code
        fi
    fi
}

# ============================================================================
# INPUT HANDLING FUNCTIONS
# ============================================================================

detect_input_source() {
    # Priority: Mounted file > Mounted directory > Stdin > Template
    # Note: No debug output here to avoid interfering with return value
    if [ -f "/app.cpp" ]; then
        echo "MOUNTED_FILE:/app.cpp"
    elif [ -f "/input/VehicleApp.cpp" ]; then
        echo "MOUNTED_DIR:/input/VehicleApp.cpp"
    elif [ -f "/input" ]; then
        echo "MOUNTED_FILE:/input"
    elif [ -p /dev/stdin ]; then
        echo "STDIN"
    else
        echo "TEMPLATE"
    fi
}

validate_cpp_file() {
    local file="$1"
    local source_name="$2"
    
    log_info "🔍 Validating C++ source: $source_name"
    
    if [ ! -f "$file" ] || [ ! -s "$file" ]; then
        log_error "Source file is empty or does not exist: $file"
        return 1
    fi
    
    local lines=$(wc -l < "$file")
    log_debug "File has $lines lines"
    
    # Check for basic VehicleApp structure
    if ! grep -q "class.*VehicleApp" "$file"; then
        log_error "Invalid VehicleApp.cpp: Missing VehicleApp class definition"
        log_info "💡 Ensure your class extends velocitas::VehicleApp"
        return 1
    fi
    
    if ! grep -q "#include.*VehicleApp.h" "$file"; then
        log_warning "Missing VehicleApp.h include - may cause build issues"
    fi
    
    # Show source info
    log_success "Source validated: $source_name ($lines lines)"
    
    # Show first few lines for confirmation
    if [ "$VERBOSE" = true ]; then
        log_debug "Source preview (first 10 lines):"
        head -10 "$file" | sed 's/^/    /' | tee -a "$LOG_FILE"
    fi
    
    return 0
}

prepare_source() {
    log_info "📝 Preparing source code..."
    log_debug "🔍 Detecting input source..."
    
    local input_source=$(detect_input_source)
    local source_type=$(echo "$input_source" | cut -d: -f1)
    local source_path=$(echo "$input_source" | cut -d: -f2)
    
    log_debug "Source type: $source_type, path: $source_path"
    
    # Ensure source directory exists
    mkdir -p "$(dirname "$APP_SOURCE")"
    
    case "$source_type" in
        "MOUNTED_FILE")
            log_info "📁 Using mounted file: $source_path"
            cp "$source_path" "$APP_SOURCE"
            if validate_cpp_file "$APP_SOURCE" "$source_path"; then
                return 0
            else
                return 1
            fi
            ;;
        "MOUNTED_DIR")
            log_info "📁 Using file from mounted directory: $source_path"
            cp "$source_path" "$APP_SOURCE"
            if validate_cpp_file "$APP_SOURCE" "$source_path"; then
                return 0
            else
                return 1
            fi
            ;;
        "STDIN")
            log_info "📥 Reading from stdin..."
            cat > "$APP_SOURCE"
            if [ ! -s "$APP_SOURCE" ]; then
                log_error "No input received from stdin"
                return 1
            fi
            if validate_cpp_file "$APP_SOURCE" "stdin"; then
                return 0
            else
                return 1
            fi
            ;;
        "TEMPLATE")
            log_info "📋 Using built-in template"
            if [ -f "$WORKSPACE/templates/app/src/VehicleApp.cpp" ]; then
                cp "$WORKSPACE/templates/app/src/VehicleApp.cpp" "$APP_SOURCE"
            else
                log_error "Built-in template not found"
                return 1
            fi
            if validate_cpp_file "$APP_SOURCE" "built-in template"; then
                return 0
            else
                return 1
            fi
            ;;
        *)
            log_error "Unknown input source type: $source_type"
            return 1
            ;;
    esac
}

# ============================================================================
# BUILD FUNCTIONS
# ============================================================================

clean_workspace() {
    if [ "$CLEAN" = true ]; then
        log_info "🧹 Cleaning workspace..."
        
        if [ -d "$BUILD_DIR" ]; then
            rm -rf "$BUILD_DIR"
            log_success "Removed old build directory"
        fi
        
        # Clean old binaries
        find "$WORKSPACE" -name "app" -type f -delete 2>/dev/null || true
        log_success "Removed old binaries"
    fi
}

prepare_build_environment() {
    log_info "🔧 Preparing build environment..."
    
    cd "$WORKSPACE"
    export PATH="/home/vscode/.local/bin:$PATH"
    
    # Verify workspace structure
    if [ ! -f "app/AppManifest.json" ]; then
        log_error "Missing AppManifest.json in workspace"
        return 1
    fi
    
    if [ ! -f "conanfile.txt" ]; then
        log_error "Missing conanfile.txt in workspace"
        return 1
    fi
    
    log_success "Build environment ready"
}

configure_vss() {
    if [ "$SKIP_VSS" = true ]; then
        log_info "⏭️  Skipping VSS configuration (--skip-vss flag)"
        return 0
    fi
    
    log_info "🚗 Configuring Vehicle Signal Specification..."
    
    local vss_updated=false
    local manifest_file="$WORKSPACE/app/AppManifest.json"
    
    # Handle custom VSS
    if [ -n "$VSS_SPEC_FILE" ] && [ -f "$VSS_SPEC_FILE" ]; then
        log_info "Using custom VSS file: $VSS_SPEC_FILE"
        cp "$VSS_SPEC_FILE" "$WORKSPACE/custom-vss.json"
        
        if command -v jq >/dev/null 2>&1; then
            jq --arg vss_path "file:///quickbuild/custom-vss.json" \
               '.interfaces[0].config.src = $vss_path' \
               "$manifest_file" > "$manifest_file.tmp" && \
               mv "$manifest_file.tmp" "$manifest_file"
            vss_updated=true
        fi
        
    elif [ -n "$VSS_SPEC_URL" ]; then
        log_info "Using custom VSS URL: $VSS_SPEC_URL"
        
        if [[ "$VSS_SPEC_URL" =~ ^https?:// ]]; then
            if command -v jq >/dev/null 2>&1; then
                jq --arg vss_url "$VSS_SPEC_URL" \
                   '.interfaces[0].config.src = $vss_url' \
                   "$manifest_file" > "$manifest_file.tmp" && \
                   mv "$manifest_file.tmp" "$manifest_file"
                vss_updated=true
            fi
        else
            log_error "Invalid VSS URL format: $VSS_SPEC_URL"
            return 1
        fi
    fi
    
    if [ "$vss_updated" = true ]; then
        log_success "VSS configuration updated"
        local vss_source=$(grep -o '"src": "[^"]*"' "$manifest_file" | cut -d'"' -f4)
        log_info "VSS source: $vss_source"
    else
        log_info "Using default VSS 4.0 specification"
    fi
}

generate_model() {
    if [ "$SKIP_VSS" = true ]; then
        log_info "⏭️  Skipping vehicle model generation (--skip-vss flag)"
        return 0
    fi
    
    log_info "🏗️  Generating vehicle model from VSS..."
    
    cd "$WORKSPACE"
    
    if ! run_command "velocitas exec vehicle-signal-interface download-vspec" \
                     "VSS specification downloaded" \
                     "Failed to download VSS specification"; then
        return 1
    fi
    
    if ! run_command "velocitas exec vehicle-signal-interface generate-model" \
                     "Vehicle model generated successfully" \
                     "Failed to generate vehicle model"; then
        return 1
    fi
}

install_dependencies() {
    if [ "$SKIP_DEPS" = true ]; then
        log_info "⏭️  Skipping dependency installation (--skip-deps flag)"
        log_info "💡 Using pre-cached dependencies"
        return 0
    fi
    
    log_info "📦 Installing/verifying dependencies..."
    
    # Check for proxy configuration
    if [ -n "$HTTP_PROXY" ] || [ -n "$HTTPS_PROXY" ]; then
        log_info "🌐 Using proxy configuration for dependency downloads"
        log_info "   HTTP_PROXY: ${HTTP_PROXY:-not set}"
        log_info "   HTTPS_PROXY: ${HTTPS_PROXY:-not set}"
    else
        log_warning "⚠️  No proxy configuration detected"
        log_info "💡 If you're behind a corporate firewall, consider using proxy settings:"
        log_info "   docker run -e HTTP_PROXY=http://proxy:3128 -e HTTPS_PROXY=http://proxy:3128 ..."
    fi
    
    cd "$WORKSPACE"
    
    if ! run_command "velocitas exec build-system install" \
                     "Dependencies installed/verified successfully" \
                     "Dependency installation had issues"; then
        if [ -z "$HTTP_PROXY" ] && [ -z "$HTTPS_PROXY" ]; then
            log_warning "❗ Dependency issues detected - this may be due to missing proxy configuration"
            log_info "💡 Try with proxy: docker run -e HTTP_PROXY=http://proxy:3128 ..."
        else
            log_warning "❗ Dependency issues detected despite proxy configuration"
        fi
        log_info "🔄 Continuing with build using cached dependencies..."
    fi
}

compile_application() {
    log_info "⚙️  Compiling C++ application..."
    log_info "   🎯 Target: Release mode (optimized)"
    log_info "   ⏱️  Expected time: 60-90 seconds"
    
    cd "$WORKSPACE"
    
    local build_cmd="velocitas exec build-system build -r"
    # Note: build-system doesn't support --verbose flag, our verbosity is handled by run_command
    
    if ! run_command "$build_cmd" \
                     "C++ compilation completed successfully" \
                     "Build compilation failed"; then
        
        log_error "Build failed - showing build log"
        echo ""
        echo "📋 Build Log (last 30 lines):"
        tail -n 30 "$LOG_FILE"
        echo ""
        echo "💡 Common fixes:"
        echo "   - Check syntax errors in your C++ code"
        echo "   - Ensure all #include directives are correct"
        echo "   - Verify Vehicle signal names match VSS specification"
        echo "   - Use --verbose flag for detailed build output"
        return 1
    fi
    
    # Verify build output
    verify_build_output
}

verify_build_output() {
    log_info "🔍 Verifying build output..."
    
    local executable_path="$BUILD_DIR/bin/app"
    
    if [ ! -f "$executable_path" ]; then
        log_error "Build reported success but executable not found"
        log_info "Expected: $executable_path"
        log_info "Available files in build directory:"
        find "$BUILD_DIR" -name "*app*" -type f 2>/dev/null || echo "  No app executable found"
        return 1
    fi
    
    # Check if executable is recent (within last 5 minutes)
    local file_age=$(stat -c %Y "$executable_path" 2>/dev/null || echo "0")
    local current_time=$(date +%s)
    local age_seconds=$((current_time - file_age))
    
    if [ $age_seconds -gt 300 ]; then
        log_error "Executable found but appears old (${age_seconds}s)"
        log_error "This may indicate a failed build using cached binary"
        return 1
    fi
    
    # Display executable info
    local size=$(ls -lh "$executable_path" | awk '{print $5}')
    local permissions=$(ls -l "$executable_path" | awk '{print $1}')
    
    log_success "Build verification completed"
    log_info "📍 Executable: $executable_path"
    log_info "📏 Size: $size"
    log_info "🔐 Permissions: $permissions"
    
    # Make executable
    chmod +x "$executable_path"
}

# ============================================================================
# RUN FUNCTIONS
# ============================================================================

check_runtime_services() {
    log_info "🔍 Checking runtime environment..."
    
    # Check MQTT broker (only if not already configured)
    if [ -z "$SDV_MQTT_ADDRESS" ] || [ "$SDV_MQTT_ADDRESS" = "127.0.0.1:1883" ]; then
        if ! nc -z 127.0.0.1 1883 2>/dev/null; then
            log_warning "MQTT broker not available (127.0.0.1:1883)"
            log_info "   📱 App will run in simulation mode"
            export SDV_MQTT_ADDRESS="disabled"
        else
            log_success "MQTT broker available (127.0.0.1:1883)"
        fi
    else
        log_info "🌐 Using configured MQTT address: $SDV_MQTT_ADDRESS"
    fi
    
    # Check Vehicle Data Broker (only if not already configured)
    if [ -z "$SDV_VEHICLEDATABROKER_ADDRESS" ] || [ "$SDV_VEHICLEDATABROKER_ADDRESS" = "127.0.0.1:55555" ]; then
        if ! nc -z 127.0.0.1 55555 2>/dev/null; then
            log_warning "Vehicle Data Broker not available (127.0.0.1:55555)"
            log_info "   📡 App will run in simulation mode"
            export SDV_VEHICLEDATABROKER_ADDRESS="disabled"
        else
            log_success "Vehicle Data Broker available (127.0.0.1:55555)"
        fi
    else
        log_info "🌐 Using configured VDB address: $SDV_VEHICLEDATABROKER_ADDRESS"
    fi
}

run_application() {
    local timeout=${1:-$DEFAULT_TIMEOUT}
    
    log_info "🚀 Starting vehicle application..."
    log_info "   ⏰ Runtime: ${timeout}s (for demonstration)"
    log_info "   💡 Watch for signal processing and app behavior"
    
    cd "$WORKSPACE"
    local executable="$BUILD_DIR/bin/app"
    
    if [ ! -f "$executable" ]; then
        log_error "Application executable not found: $executable"
        return 1
    fi
    
    local size=$(ls -lh "$executable" | awk '{print $5}')
    log_info "📍 Running: $executable ($size)"
    
    echo ""
    echo "============================================" | tee -a "$LOG_FILE"
    echo "📱 Vehicle Application Live Output:" | tee -a "$LOG_FILE"
    echo "============================================" | tee -a "$LOG_FILE"
    
    local run_success=0
    if timeout "$timeout" "$executable" 2>&1 | tee -a "$LOG_FILE"; then
        run_success=1
    else
        local exit_code=$?
        if [ $exit_code -eq 124 ]; then
            log_success "Application completed ${timeout}s demonstration run"
            run_success=1
        else
            log_error "Application exited with code: $exit_code"
            run_success=0
        fi
    fi
    
    echo "============================================" | tee -a "$LOG_FILE"
    
    return $((1 - run_success))
}

# ============================================================================
# TEST FUNCTIONS
# ============================================================================

create_test_network() {
    log_info "🌐 Setting up test network..."
    
    # Create isolated test network
    if ! docker network create velocitas-test 2>/dev/null; then
        log_info "Test network already exists, removing and recreating..."
        docker network rm velocitas-test 2>/dev/null || true
        docker network create velocitas-test
    fi
    
    log_success "Test network 'velocitas-test' ready"
}

start_kuksa_server() {
    log_info "🚗 Starting KUKSA Vehicle Data Broker..."
    
    # Stop existing container
    docker stop kuksa-server 2>/dev/null || true
    docker rm kuksa-server 2>/dev/null || true
    
    # Start KUKSA server
    if ! docker run -d \
        --name kuksa-server \
        --network velocitas-test \
        -p 55555:55555 \
        ghcr.io/eclipse-kuksa/kuksa-databroker:latest; then
        log_error "Failed to start KUKSA server"
        return 1
    fi
    
    # Wait for server to be ready
    log_info "⏳ Waiting for KUKSA server to be ready..."
    local attempts=0
    while [ $attempts -lt 30 ]; do
        if nc -z 127.0.0.1 55555 2>/dev/null; then
            log_success "KUKSA server ready (127.0.0.1:55555)"
            return 0
        fi
        sleep 1
        attempts=$((attempts + 1))
    done
    
    log_error "KUKSA server failed to start within 30 seconds"
    docker logs kuksa-server
    return 1
}

start_test_client() {
    log_info "🧪 Starting KUKSA test client..."
    
    # Stop existing container
    docker stop kuksa-client 2>/dev/null || true
    docker rm kuksa-client 2>/dev/null || true
    
    # Start test client in background
    docker run -d \
        --name kuksa-client \
        --network velocitas-test \
        ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main \
        sleep 3600  # Keep container running
    
    log_success "KUKSA test client ready"
}

run_signal_tests() {
    log_info "📡 Running signal validation tests..."
    
    # Test script for signal injection
    cat > /tmp/signal_test.py << 'EOF'
import asyncio
import time
from kuksa_client.grpc import VSSClient
from kuksa_client.grpc import Datapoint

async def test_signals():
    print("🔌 Connecting to KUKSA server...")
    async with VSSClient('127.0.0.1', 55555) as client:
        print("✅ Connected to KUKSA server")
        
        # Test 1: Speed signal
        print("🚗 Testing speed signal...")
        await client.set_current_values({'Vehicle.Speed': Datapoint(value=65.0)})
        await asyncio.sleep(2)
        
        # Test 2: High speed alert
        print("⚠️  Testing high speed alert...")
        await client.set_current_values({'Vehicle.Speed': Datapoint(value=85.0)})
        await asyncio.sleep(2)
        
        # Test 3: Temperature signal
        print("🌡️  Testing temperature signal...")
        await client.set_current_values({'Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature': Datapoint(value=30.0)})
        await asyncio.sleep(2)
        
        # Test 4: Fuel level warning
        print("⛽ Testing fuel level warning...")
        await client.set_current_values({'Vehicle.Powertrain.FuelSystem.Level': Datapoint(value=15.0)})
        await asyncio.sleep(2)
        
        # Test 5: Multi-signal test
        print("🔄 Testing multiple signals simultaneously...")
        await client.set_current_values({
            'Vehicle.Speed': Datapoint(value=75.0),
            'Vehicle.Cabin.HVAC.Station.Row1.Left.Temperature': Datapoint(value=28.0),
            'Vehicle.Powertrain.FuelSystem.Level': Datapoint(value=45.0)
        })
        await asyncio.sleep(3)
        
        print("✅ All signal tests completed")

if __name__ == "__main__":
    asyncio.run(test_signals())
EOF
    
    # Run signal tests
    log_info "🧪 Injecting test signals..."
    if docker exec kuksa-client python3 -c "$(cat /tmp/signal_test.py)" 2>&1 | tee -a "$LOG_FILE"; then
        log_success "Signal injection tests completed"
    else
        log_error "Signal injection tests failed"
        return 1
    fi
    
    # Allow time for signal processing
    sleep 5
}

run_test_suite() {
    local test_type="$1"
    
    log_info "🧪 Starting test suite: $test_type"
    
    case "$test_type" in
        "signal-validation")
            create_test_network
            start_kuksa_server
            start_test_client
            
            # Build and run app in test mode
            log_info "🏗️  Building application for testing..."
            if ! build_application; then
                log_error "Build failed - cannot run tests"
                cleanup_test_environment
                return 1
            fi
            
            # Start app in background with test network
            log_info "🚀 Starting application in test mode..."
            docker run -d \
                --name test-vehicle-app \
                --network velocitas-test \
                -v "$BUILD_DIR/bin/app:/app" \
                --env SDV_VEHICLEDATABROKER_ADDRESS="kuksa-server:55555" \
                alpine:latest \
                timeout 30 /app
            
            # Run signal tests
            run_signal_tests
            
            # Analyze results
            analyze_test_results
            
            cleanup_test_environment
            ;;
        "build-validation")
            log_info "🔨 Testing build process..."
            if build_application; then
                log_success "Build validation passed"
            else
                log_error "Build validation failed"
                return 1
            fi
            ;;
        "full-suite")
            run_test_suite "build-validation"
            run_test_suite "signal-validation"
            ;;
        *)
            log_error "Unknown test type: $test_type"
            return 1
            ;;
    esac
}

analyze_test_results() {
    log_info "📊 Analyzing test results..."
    
    # Check app logs for signal processing
    local app_logs=$(docker logs test-vehicle-app 2>&1)
    
    if echo "$app_logs" | grep -q "Speed.*received\|Speed.*km/h"; then
        log_success "✓ Speed signal processing detected"
    else
        log_warning "✗ Speed signal processing not detected"
    fi
    
    if echo "$app_logs" | grep -q "HIGH SPEED\|Speed.*alert"; then
        log_success "✓ Speed alert functionality working"
    else
        log_warning "✗ Speed alert functionality not detected"
    fi
    
    if echo "$app_logs" | grep -q "Temperature\|Temp"; then
        log_success "✓ Temperature signal processing detected"
    else
        log_warning "✗ Temperature signal processing not detected"
    fi
    
    if echo "$app_logs" | grep -q "Fuel\|fuel"; then
        log_success "✓ Fuel level processing detected"
    else
        log_warning "✗ Fuel level processing not detected"
    fi
    
    # Show summary
    local total_tests=4
    local passed_tests=$(echo "$app_logs" | grep -c "✓" || echo "0")
    
    log_info "📈 Test Summary: $passed_tests/$total_tests tests passed"
}

cleanup_test_environment() {
    log_info "🧹 Cleaning up test environment..."
    
    # Stop and remove containers
    docker stop test-vehicle-app kuksa-client kuksa-server 2>/dev/null || true
    docker rm test-vehicle-app kuksa-client kuksa-server 2>/dev/null || true
    
    # Remove test network
    docker network rm velocitas-test 2>/dev/null || true
    
    log_success "Test environment cleaned up"
}

# ============================================================================
# MAIN BUILD FUNCTION
# ============================================================================

build_application() {
    log_info "🚀 Building vehicle application..."
    
    # Step 1: Prepare source
    log_info "📝 STEP 1/6: Preparing source code..."
    if prepare_source; then
        log_success "Source ready: File mounted successfully"
    else
        log_error "Source preparation failed"
        return 1
    fi
    echo ""
    
    # Step 2: Clean workspace
    log_info "🧹 STEP 2/6: Workspace preparation..."
    clean_workspace
    prepare_build_environment
    echo ""
    
    # Step 3: Configure VSS
    if [ "$SKIP_VSS" = true ]; then
        log_info "🚗 STEP 3/6: Vehicle Signal Specification..."
        log_info "⏭️  Skipping VSS configuration (--skip-vss flag)"
    else
        log_info "🚗 STEP 3/6: Vehicle Signal Specification..."
        configure_vss
    fi
    echo ""
    
    # Step 4: Generate model
    if [ "$SKIP_VSS" = true ]; then
        log_info "🏗️  STEP 4/6: Vehicle model generation..."
        log_info "⏭️  Skipping vehicle model generation (--skip-vss flag)"
    else
        log_info "🏗️  STEP 4/6: Vehicle model generation..."
        if [ ! -d "$WORKSPACE/app/vehicle_model" ] || [ "$FORCE_REBUILD" = true ]; then
            generate_model
        else
            log_info "✅ Vehicle model exists, skipping (use --force to regenerate)"
        fi
    fi
    echo ""
    
    # Step 5: Install dependencies
    if [ "$SKIP_DEPS" = true ]; then
        log_info "📦 STEP 5/6: Dependencies..."
        log_info "⏭️  Skipping dependency installation (--skip-deps flag)"
        log_info "💡 Using pre-cached dependencies"
    else
        log_info "📦 STEP 5/6: Dependencies..."
        install_dependencies
    fi
    echo ""
    
    # Step 6: Compile
    log_info "⚙️  STEP 6/6: Compilation..."
    compile_application
    
    return $?
}

# ============================================================================
# COMMAND HANDLERS
# ============================================================================

handle_build() {
    echo ""
    log_info "🚀 Velocitas Build System"
    log_info "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    log_info "🏗️  Building vehicle application from source"
    echo ""
    
    if build_application; then
        echo ""
        echo "🎉 Build completed successfully!" | tee -a "$LOG_FILE"
        echo "📍 Executable: $BUILD_DIR/bin/app" | tee -a "$LOG_FILE"
        echo "💡 Use 'run' command to execute your application" | tee -a "$LOG_FILE"
        return 0
    else
        echo ""
        log_error "Build failed!"
        echo "💡 Use --verbose flag for detailed output"
        echo "📋 Check build log: $LOG_FILE"
        return 1
    fi
}

handle_run() {
    local timeout=${1:-$DEFAULT_TIMEOUT}
    
    echo ""
    log_info "🏃 Velocitas Run System"
    log_info "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    log_info "🚀 Running vehicle application"
    echo ""
    
    # Check if executable exists
    local executable="$BUILD_DIR/bin/app"
    
    if [ ! -f "$executable" ]; then
        log_error "❌ Application executable not found: $executable"
        log_error "💡 Run 'build' command first to create the executable"
        log_info "   Example: docker run ... velocitas-quick build --verbose"
        return 1
    fi
    
    log_success "✅ Found existing executable"
    local size=$(ls -lh "$executable" | awk '{print $5}')
    log_info "📍 Executable: $executable ($size)"
    echo ""
    
    # Check runtime environment
    log_info "🔍 Checking runtime environment..."
    check_runtime_services
    echo ""
    
    # Run application
    log_info "🚀 Running application..."
    if run_application "$timeout"; then
        echo ""
        log_success "Application run completed successfully!"
        return 0
    else
        echo ""
        log_error "Application run failed!"
        return 1
    fi
}

handle_test() {
    local test_type="$1"
    
    echo ""
    log_info "🧪 Velocitas Test System"
    log_info "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    log_info "🔬 Running test suite: $test_type"
    echo ""
    
    if run_test_suite "$test_type"; then
        echo ""
        log_success "Test suite completed successfully!"
        return 0
    else
        echo ""
        log_error "Test suite failed!"
        return 1
    fi
}

# ============================================================================
# HELP AND USAGE
# ============================================================================

show_help() {
    cat << 'EOF'
Velocitas CLI - Unified Build, Test, and Run System

DESCRIPTION:
    Single entry point for all vehicle app operations with enhanced control,
    file mounting support, and comprehensive testing capabilities.

USAGE:
    velocitas-cli <command> [options] [arguments]

COMMANDS:
    build                   Build vehicle application from source
    run [timeout]          Build (if needed) and run application 
    test <type>            Run test suite
    validate               Validate source code only
    clean                  Clean all build artifacts
    help                   Show this help message

TEST TYPES:
    signal-validation      Test signal injection and processing with KUKSA
    build-validation       Test build process and compilation
    full-suite            Run all tests

GLOBAL OPTIONS:
    --verbose              Show detailed command output
    --clean                Clean workspace before build
    --skip-deps            Skip dependency installation
    --skip-vss             Skip VSS model generation
    --force                Force rebuild even if unchanged
    --quiet                Suppress non-essential output

INPUT METHODS (Priority Order):
    1. Mount file:         -v $(pwd)/MyApp.cpp:/app.cpp
    2. Mount directory:    -v $(pwd)/src:/input
    3. Pipe stdin:         cat MyApp.cpp | docker run -i
    4. Built-in template:  (no input provided)

ENVIRONMENT VARIABLES:
    VSS_SPEC_FILE         Path to custom VSS JSON file
    VSS_SPEC_URL          URL to custom VSS JSON specification
    VERBOSE_BUILD         Set to 1 for verbose output (same as --verbose)

EXAMPLES:
    # Build with mounted file (recommended)
    docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --verbose

    # Run with custom timeout
    docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick run 120

    # Test signal validation
    docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick test signal-validation

    # Force clean rebuild
    docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --clean --force

    # Skip dependencies for faster build
    docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick build --skip-deps

    # Use custom VSS specification
    docker run -v $(pwd)/MyApp.cpp:/app.cpp -e VSS_SPEC_URL=https://company.com/vss.json velocitas-quick build

    # Validate code without building
    docker run -v $(pwd)/MyApp.cpp:/app.cpp velocitas-quick validate

For more information, visit: https://github.com/eclipse-velocitas/vehicle-app-cpp-template
EOF
}

# ============================================================================
# ARGUMENT PARSING
# ============================================================================

parse_arguments() {
    command=""
    args=()
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            --verbose)
                VERBOSE=true
                shift
                ;;
            --clean)
                CLEAN=true
                shift
                ;;
            --skip-deps)
                SKIP_DEPS=true
                echo "🔧 DEBUG: Set SKIP_DEPS=true" >&2
                shift
                ;;
            --skip-vss)
                SKIP_VSS=true
                echo "🔧 DEBUG: Set SKIP_VSS=true" >&2
                shift
                ;;
            --force)
                FORCE_REBUILD=true
                shift
                ;;
            --quiet)
                QUIET=true
                shift
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            build|run|test|validate|clean|help)
                command="$1"
                shift
                ;;
            *)
                args+=("$1")
                shift
                ;;
        esac
    done
    
    # Set verbose mode from environment
    if [ "${VERBOSE_BUILD:-0}" = "1" ]; then
        VERBOSE=true
    fi
    
    echo "$command ${args[*]}"
}

# ============================================================================
# MAIN EXECUTION
# ============================================================================

main() {
    # Parse arguments directly (not in subshell to preserve variables)
    parse_arguments "$@"
    local command="$command"
    local args="$args"
    
    # Default command
    if [ -z "$command" ]; then
        command="build"
    fi
    
    # Log startup info
    log_info "🔧 Debug - Command: $command"
    log_info "🔧 Debug - Arguments: $args"
    log_info "🔧 Debug - Verbose: $VERBOSE"
    log_info "🔧 Debug - Clean: $CLEAN"
    log_info "🔧 Debug - Skip Deps: $SKIP_DEPS"
    log_info "🔧 Debug - Skip VSS: $SKIP_VSS"
    log_info "🔧 Debug - Force Rebuild: $FORCE_REBUILD"
    
    case "$command" in
        "build")
            handle_build
            ;;
        "run")
            local timeout=$(echo "$args" | awk '{print $1}')
            handle_run "${timeout:-$DEFAULT_TIMEOUT}"
            ;;
        "test")
            local test_type=$(echo "$args" | awk '{print $1}')
            if [ -z "$test_type" ]; then
                test_type="signal-validation"
            fi
            handle_test "$test_type"
            ;;
        "validate")
            log_info "🔍 Validating source code..."
            if prepare_source >/dev/null; then
                log_success "Source code validation passed"
            else
                log_error "Source code validation failed"
                exit 1
            fi
            ;;
        "clean")
            log_info "🧹 Cleaning all build artifacts..."
            CLEAN=true
            clean_workspace
            log_success "Clean completed"
            ;;
        "help")
            show_help
            ;;
        *)
            log_error "Unknown command: $command"
            echo "Use 'help' for usage information"
            exit 1
            ;;
    esac
}

# Set up signal handlers for clean shutdown
trap 'log_info "🛑 Interrupted by user"; cleanup_test_environment 2>/dev/null; exit 130' INT TERM

# Execute main function
main "$@"