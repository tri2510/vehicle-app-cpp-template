#!/bin/bash
# ============================================================================
# Quick Run Script - Mode 2 Blackbox Utility
# ============================================================================
# Purpose: Build VehicleApp.cpp and run with minimal services for testing
# Usage: 
#   cat VehicleApp.cpp | docker run -i velocitas-quick run
#   docker run -v $(pwd)/app.cpp:/input velocitas-quick run
# ============================================================================

set -e

# Configuration
WORKSPACE="/quickbuild"
BUILD_DIR="$WORKSPACE/build"
LOG_FILE="/tmp/run.log"
RUN_TIMEOUT=15

# Logging functions
log_info() {
    echo "ℹ️  [INFO] $1" | tee -a "$LOG_FILE"
}

log_success() {
    echo "✅ [SUCCESS] $1" | tee -a "$LOG_FILE"
}

log_error() {
    echo "❌ [ERROR] $1" | tee -a "$LOG_FILE"
}

log_warning() {
    echo "⚠️  [WARNING] $1" | tee -a "$LOG_FILE"
}

# Initialize run log
echo "🏃 Quick Run Started: $(date)" > "$LOG_FILE"

# Function to check if services are available
check_services() {
    log_info "🔍 Checking external service availability..."
    
    # Check if MQTT broker is available
    log_info "   Testing MQTT broker connection (127.0.0.1:1883)..."
    if ! nc -z 127.0.0.1 1883 2>/dev/null; then
        log_warning "MQTT broker not available at 127.0.0.1:1883"
        log_info "   📱 Starting in simulation mode (no external services required)"
        log_info "   💡 App will run with simulated vehicle data"
        export SDV_MQTT_ADDRESS="disabled"
    else
        log_success "MQTT broker available at 127.0.0.1:1883"
        log_info "   📡 Will connect to real MQTT broker for messaging"
    fi
    
    # Check if Vehicle Data Broker is available
    if ! nc -z 127.0.0.1 55555 2>/dev/null; then
        log_warning "Vehicle Data Broker not available at 127.0.0.1:55555"
        log_info "Starting in simulation mode (no VDB connection)"
        export SDV_VEHICLEDATABROKER_ADDRESS="disabled"
    else
        log_success "Vehicle Data Broker available at 127.0.0.1:55555"
    fi
}

# Function to run the application with timeout
run_application() {
    log_info "🚀 Running vehicle application..."
    
    cd "$WORKSPACE"
    local app_executable="$BUILD_DIR/bin/app"
    
    log_info "🔍 Checking executable location..."
    if [ ! -f "$app_executable" ]; then
        log_error "Application executable not found: $app_executable"
        log_info "Available files in build directory:"
        find "$BUILD_DIR" -type f -name "*app*" 2>/dev/null || echo "  No app files found"
        return 1
    fi
    
    # Show executable info
    local size=$(ls -lh "$app_executable" | awk '{print $5}')
    log_success "Found executable: $app_executable"
    log_info "📏 Executable size: $size"
    
    # Make executable
    log_info "🔐 Setting executable permissions..."
    chmod +x "$app_executable"
    
    log_info "⏰ Starting application with ${RUN_TIMEOUT}s timeout for demonstration..."
    log_info "💡 Watch for vehicle signal processing and app behavior"
    log_info "🔄 Application will automatically stop after ${RUN_TIMEOUT}s"
    echo ""
    echo "============================================" | tee -a "$LOG_FILE"
    echo "📱 Vehicle Application Live Output:" | tee -a "$LOG_FILE"
    echo "============================================" | tee -a "$LOG_FILE"
    
    # Run application with timeout and capture output
    local run_success=0
    
    if timeout "$RUN_TIMEOUT" "$app_executable" 2>&1 | tee -a "$LOG_FILE"; then
        run_success=1
    else
        local exit_code=$?
        if [ $exit_code -eq 124 ]; then
            log_success "Application ran for ${RUN_TIMEOUT}s (timeout reached - normal for demo)"
            run_success=1
        else
            log_error "Application exited with code: $exit_code"
            run_success=0
        fi
    fi
    
    echo "============================================" | tee -a "$LOG_FILE"
    
    return $((1 - run_success))
}

# Function to analyze application output
analyze_output() {
    log_info "Analyzing application behavior..."
    
    # Check for common patterns in the log
    if grep -q "Vehicle App.*starting" "$LOG_FILE"; then
        log_success "Application initialization detected"
    fi
    
    if grep -q "Connected to.*broker" "$LOG_FILE"; then
        log_success "Service connection established"
    fi
    
    if grep -q "Subscribed to" "$LOG_FILE"; then
        log_success "Vehicle signal subscription detected"
    fi
    
    if grep -q "ERROR\|Error\|error" "$LOG_FILE"; then
        log_warning "Errors detected in application output"
        echo "Error summary:"
        grep -i "error" "$LOG_FILE" | tail -n 5
    fi
    
    if grep -q "Signal.*received\|Speed.*detected" "$LOG_FILE"; then
        log_success "Vehicle signal processing detected"
    fi
}

# Function to display run summary
run_summary() {
    log_info "Run Summary:"
    echo "============================================" | tee -a "$LOG_FILE"
    echo "🏃 Application Runtime: ${RUN_TIMEOUT}s" | tee -a "$LOG_FILE"
    
    # Count log entries
    local info_count=$(grep -c "\[INFO\]" "$LOG_FILE" || echo "0")
    local success_count=$(grep -c "\[SUCCESS\]" "$LOG_FILE" || echo "0")
    local warning_count=$(grep -c "\[WARNING\]" "$LOG_FILE" || echo "0")
    local error_count=$(grep -c "\[ERROR\]" "$LOG_FILE" || echo "0")
    
    echo "📊 Log Summary:" | tee -a "$LOG_FILE"
    echo "   - Info messages: $info_count" | tee -a "$LOG_FILE"
    echo "   - Successes: $success_count" | tee -a "$LOG_FILE"
    echo "   - Warnings: $warning_count" | tee -a "$LOG_FILE"
    echo "   - Errors: $error_count" | tee -a "$LOG_FILE"
    
    echo "🕐 Completed: $(date)" | tee -a "$LOG_FILE"
    echo "============================================" | tee -a "$LOG_FILE"
}

# Function to provide usage help
show_help() {
    echo "Quick Run Script - Mode 2 Blackbox Utility"
    echo ""
    echo "This script builds and runs your VehicleApp.cpp with minimal services."
    echo ""
    echo "Usage:"
    echo "  cat VehicleApp.cpp | docker run -i velocitas-quick run"
    echo "  docker run -v \$(pwd)/app.cpp:/input velocitas-quick run"
    echo "  docker run -v \$(pwd):/input velocitas-quick run"
    echo ""
    echo "Features:"
    echo "  - Builds your VehicleApp.cpp using fixed templates"
    echo "  - Runs application for ${RUN_TIMEOUT} seconds"
    echo "  - Works with or without external MQTT/VDB services"
    echo "  - Provides detailed runtime analysis"
    echo ""
    echo "Service Requirements (optional):"
    echo "  - MQTT Broker at 127.0.0.1:1883"
    echo "  - Vehicle Data Broker at 127.0.0.1:55555"
    echo ""
    echo "Note: If services are not available, app runs in simulation mode."
}

# Main execution flow
main() {
    echo ""
    log_info "🏃 Velocitas C++ Quick Build & Run Utility"
    log_info "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    log_info "🚀 Build your VehicleApp.cpp and run it with live output"
    echo ""
    
    # Step 1: Build the application if not already built
    if [ -f "$BUILD_DIR/bin/app" ]; then
        log_info "🔧 STEP 1/5: Found existing application..."
        log_success "✅ Using pre-built application!"
        log_info "🏃 Proceeding to run the vehicle application..."
    else
        log_info "🔧 STEP 1/5: Building application..."
        log_info "   This may take 60-90 seconds for compilation..."
        if ! /scripts/quick-build.sh build; then
            log_error "Build failed - cannot run application"
            log_info "💡 Check build output above for compilation errors"
            exit 1
        fi
        log_success "✅ Build completed successfully!"
        log_info "🏃 Proceeding to run the vehicle application..."
    fi
    echo ""
    
    # Step 2: Check service availability
    log_info "🔧 STEP 2/5: Checking service environment..."
    check_services
    echo ""
    
    # Step 3: Run the application
    log_info "🔧 STEP 3/5: Running vehicle application..."
    if run_application; then
        echo ""
        log_success "✅ Application completed ${RUN_TIMEOUT}s demonstration run"
    else
        echo ""
        log_error "❌ Application run failed"
        return 1
    fi
    echo ""
    
    # Step 4: Analyze output
    log_info "🔧 STEP 4/5: Analyzing application behavior..."
    analyze_output
    echo ""
    
    # Step 5: Display summary
    log_info "🔧 STEP 5/5: Generating run summary..."
    run_summary
    
    log_success "Quick run completed successfully!"
}

# Handle script execution
case "${1:-run}" in
    "run")
        main
        ;;
    "help"|"--help"|"-h")
        show_help
        ;;
    *)
        log_error "Unknown command: $1"
        echo "Use 'help' for usage information"
        exit 1
        ;;
esac