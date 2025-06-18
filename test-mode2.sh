#!/bin/bash

# Mode 2 Blackbox Utility Test Script
# Tests the velocitas-quick container with optional proxy configuration

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
CONTAINER_NAME="velocitas-quick"
PROXY_CONTAINER_NAME="velocitas-quick-proxy"
TEST_RESULTS_DIR="test_results"
LOG_FILE=""
USE_PROXY=false
USE_VERBOSE=false
PROXY_HOST="127.0.0.1:3128"
TEST_TIMEOUT=120

# Test counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Usage function
show_usage() {
    cat << EOF
Mode 2 Blackbox Utility Test Script

Usage: $0 [OPTIONS]

This script runs comprehensive tests for the velocitas-quick container including:
- Basic build functionality (8 tests)
- Granular commands (gen-model, compile, finalize + aliases) (5 tests)  
- Run/rerun commands (2 tests)
- Sequential granular workflow (1 test)
- Verbose build mode (1 test)

Total: 17 test cases covering all velocitas-quick functionality

OPTIONS:
    -p, --proxy         Enable proxy testing (default: false)
    --proxy-host HOST   Proxy host:port (default: 127.0.0.1:3128)
    -v, --verbose       Enable verbose build mode testing (default: false)
    -t, --timeout SEC   Test timeout in seconds (default: 120)
    -o, --output DIR    Output directory for logs (default: test_results)
    -h, --help          Show this help message

EXAMPLES:
    # Run all 17 tests without proxy
    $0

    # Run all tests with proxy
    $0 --proxy

    # Run tests with verbose mode
    $0 --verbose

    # Run tests with proxy and verbose mode
    $0 --proxy --verbose

    # Run tests with custom proxy host
    $0 --proxy --proxy-host 192.168.1.100:8080

    # Run tests with custom timeout and output directory
    $0 --proxy --verbose --timeout 180 --output my_test_results

EOF
}

# Parse command line arguments
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -p|--proxy)
                USE_PROXY=true
                shift
                ;;
            --proxy-host)
                PROXY_HOST="$2"
                shift 2
                ;;
            -v|--verbose)
                USE_VERBOSE=true
                shift
                ;;
            -t|--timeout)
                TEST_TIMEOUT="$2"
                shift 2
                ;;
            -o|--output)
                TEST_RESULTS_DIR="$2"
                shift 2
                ;;
            -h|--help)
                show_usage
                exit 0
                ;;
            *)
                echo "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done
}

# Initialize test environment
init_test_env() {
    # Create test results directory
    mkdir -p "$TEST_RESULTS_DIR"
    
    # Set log file name
    if [[ "$USE_PROXY" == "true" ]]; then
        LOG_FILE="$TEST_RESULTS_DIR/proxy_test_$(date +%Y%m%d_%H%M%S).txt"
        CONTAINER_NAME="$PROXY_CONTAINER_NAME"
    else
        LOG_FILE="$TEST_RESULTS_DIR/no_proxy_test_$(date +%Y%m%d_%H%M%S).txt"
    fi
    
    # Initialize log file
    {
        echo "🧪 Mode 2 Test Suite - $(date)"
        echo "📊 Configuration:"
        echo "   Proxy: $USE_PROXY"
        if [[ "$USE_PROXY" == "true" ]]; then
            echo "   Proxy Host: $PROXY_HOST"
        fi
        echo "   Verbose: $USE_VERBOSE"
        echo "   Container: $CONTAINER_NAME"
        echo "   Timeout: ${TEST_TIMEOUT}s"
        echo "   Output: $TEST_RESULTS_DIR"
        echo ""
        echo "📋 System Info:"
        echo "   Docker: $(docker --version)"
        echo "   OS: $(uname -a)"
        echo "   Disk: $(df -h / | tail -1)"
        echo ""
    } | tee "$LOG_FILE"
}

# Helper function to build proxy arguments
get_proxy_args() {
    if [[ "$USE_PROXY" == "true" ]]; then
        echo "-e HTTP_PROXY=http://$PROXY_HOST -e HTTPS_PROXY=http://$PROXY_HOST -e http_proxy=http://$PROXY_HOST -e https_proxy=http://$PROXY_HOST"
    fi
}

# Helper function to build verbose arguments
get_verbose_args() {
    if [[ "$USE_VERBOSE" == "true" ]]; then
        echo "-e VERBOSE_BUILD=1"
    fi
}

# Helper function to build combined environment arguments
get_env_args() {
    local proxy_args=$(get_proxy_args)
    local verbose_args=$(get_verbose_args)
    local network_args=$(get_network_args)
    echo "$proxy_args $verbose_args $network_args"
}

# Helper function to build proxy build arguments
get_proxy_build_args() {
    if [[ "$USE_PROXY" == "true" ]]; then
        echo "--build-arg HTTP_PROXY=http://$PROXY_HOST --build-arg HTTPS_PROXY=http://$PROXY_HOST --build-arg http_proxy=http://$PROXY_HOST --build-arg https_proxy=http://$PROXY_HOST --network=host"
    fi
}

# Helper function to get network arguments
get_network_args() {
    if [[ "$USE_PROXY" == "true" ]]; then
        echo "--network=host"
    fi
}

# Log test start
log_test_start() {
    local test_num=$1
    local test_name=$2
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    echo -e "${BLUE}🔧 TEST $test_num: $test_name${NC}"
    echo "🔧 TEST $test_num: $test_name" >> "$LOG_FILE"
    echo "Started: $(date)" >> "$LOG_FILE"
}

# Log test result
log_test_result() {
    local test_num=$1
    local test_name=$2
    local exit_code=$3
    local duration=$4
    
    echo "Completed: $(date)" >> "$LOG_FILE"
    echo "Exit Code: $exit_code" >> "$LOG_FILE"
    echo "Duration: ${duration}s" >> "$LOG_FILE"
    echo "" >> "$LOG_FILE"
    
    if [[ $exit_code -eq 0 ]]; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "${GREEN}✅ TEST $test_num PASSED: $test_name${NC}"
        echo "✅ TEST $test_num PASSED: $test_name" >> "$LOG_FILE"
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo -e "${RED}❌ TEST $test_num FAILED: $test_name${NC}"
        echo "❌ TEST $test_num FAILED: $test_name" >> "$LOG_FILE"
    fi
    
}

# Test 1: Container Build
test_container_build() {
    log_test_start 1 "Container Build"
    
    local start_time=$(date +%s)
    local build_args=$(get_proxy_build_args)
    
    if [[ "$USE_VERBOSE" == "true" ]]; then
        # Verbose mode: show build output to console and log
        echo -e "${BLUE}🔧 Building container with verbose output...${NC}"
        if [[ "$USE_PROXY" == "true" ]]; then
            docker build -f Dockerfile.quick $build_args -t "$CONTAINER_NAME" . 2>&1 | tee -a "$LOG_FILE"
        else
            docker build -f Dockerfile.quick -t "$CONTAINER_NAME" . 2>&1 | tee -a "$LOG_FILE"
        fi
    else
        # Normal mode: hide build output
        if [[ "$USE_PROXY" == "true" ]]; then
            docker build -f Dockerfile.quick $build_args -t "$CONTAINER_NAME" . >> "$LOG_FILE" 2>&1
        else
            docker build -f Dockerfile.quick -t "$CONTAINER_NAME" . >> "$LOG_FILE" 2>&1
        fi
    fi
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 1 "Container Build" $exit_code $duration
    return $exit_code
}

# Test 2: Help Command
test_help_command() {
    log_test_start 2 "Help Command"
    
    local start_time=$(date +%s)
    
    docker run --rm "$CONTAINER_NAME" help >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 2 "Help Command" $exit_code $duration
    return $exit_code
}

# Test 3: Basic Build via Stdin
test_basic_build_stdin() {
    log_test_start 3 "Basic Build via Stdin"
    
    local start_time=$(date +%s)
    local env_args=$(get_env_args)
    
    timeout "$TEST_TIMEOUT" bash -c "cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i $env_args '$CONTAINER_NAME' build" >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 3 "Basic Build via Stdin" $exit_code $duration
    return $exit_code
}

# Test 4: Validation Command
test_validation_command() {
    log_test_start 4 "Validation Command"
    
    local start_time=$(date +%s)
    local proxy_args=$(get_proxy_args)
    
    timeout "$TEST_TIMEOUT" bash -c "cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i $proxy_args '$CONTAINER_NAME' validate" >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 4 "Validation Command" $exit_code $duration
    return $exit_code
}

# Test 5: Custom VSS Support
test_custom_vss() {
    log_test_start 5 "Custom VSS Support"
    
    local start_time=$(date +%s)
    local proxy_args=$(get_proxy_args)
    
    timeout "$TEST_TIMEOUT" bash -c "cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i $proxy_args -e VSS_SPEC_URL=https://raw.githubusercontent.com/COVESA/vehicle_signal_specification/main/spec/VehicleSignalSpecification.json '$CONTAINER_NAME' build" >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 5 "Custom VSS Support" $exit_code $duration
    return $exit_code
}

# Test 6: Error Handling
test_error_handling() {
    log_test_start 6 "Error Handling"
    
    local start_time=$(date +%s)
    local proxy_args=$(get_proxy_args)
    
    timeout "$TEST_TIMEOUT" bash -c "echo 'invalid cpp code' | docker run --rm -i $proxy_args '$CONTAINER_NAME' build" >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    # For error handling test, we expect failure (exit code 1)
    if [[ $exit_code -eq 1 ]]; then
        exit_code=0  # Convert expected failure to success
    else
        exit_code=1  # Unexpected success or other error
    fi
    
    log_test_result 6 "Error Handling" $exit_code $duration
    return $exit_code
}

# Test 7: File Mount Input
test_file_mount_input() {
    log_test_start 7 "File Mount Input"
    
    local start_time=$(date +%s)
    local proxy_args=$(get_proxy_args)
    
    timeout "$TEST_TIMEOUT" bash -c "docker run --rm -v \$(pwd)/templates/app/src/VehicleApp.template.cpp:/input $proxy_args '$CONTAINER_NAME' build" >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 7 "File Mount Input" $exit_code $duration
    return $exit_code
}

# Test 8: Directory Mount Input
test_directory_mount_input() {
    log_test_start 8 "Directory Mount Input"
    
    local start_time=$(date +%s)
    local proxy_args=$(get_proxy_args)
    
    timeout "$TEST_TIMEOUT" bash -c "docker run --rm -v \$(pwd)/templates/app/src/VehicleApp.template.cpp:/input/VehicleApp.cpp $proxy_args '$CONTAINER_NAME' build" >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 8 "Directory Mount Input" $exit_code $duration
    return $exit_code
}

# Test 9: Granular Command - gen-model
test_granular_gen_model() {
    log_test_start 9 "Granular Command - gen-model"
    
    local start_time=$(date +%s)
    local proxy_args=$(get_proxy_args)
    
    timeout "$TEST_TIMEOUT" bash -c "docker run --rm $proxy_args '$CONTAINER_NAME' gen-model" >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 9 "Granular Command - gen-model" $exit_code $duration
    return $exit_code
}

# Test 10: Granular Command - model (alias)
test_granular_model_alias() {
    log_test_start 10 "Granular Command - model (alias)"
    
    local start_time=$(date +%s)
    local proxy_args=$(get_proxy_args)
    
    timeout "$TEST_TIMEOUT" bash -c "docker run --rm $proxy_args '$CONTAINER_NAME' model" >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 10 "Granular Command - model (alias)" $exit_code $duration
    return $exit_code
}

# Test 11: Granular Command - compile
test_granular_compile() {
    log_test_start 11 "Granular Command - compile"
    
    local start_time=$(date +%s)
    local env_args=$(get_env_args)
    
    timeout "$TEST_TIMEOUT" bash -c "cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i $env_args '$CONTAINER_NAME' compile" >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 11 "Granular Command - compile" $exit_code $duration
    return $exit_code
}

# Test 12: Granular Command - build-cpp (alias)
test_granular_build_cpp_alias() {
    log_test_start 12 "Granular Command - build-cpp (alias)"
    
    local start_time=$(date +%s)
    local proxy_args=$(get_proxy_args)
    
    timeout "$TEST_TIMEOUT" bash -c "cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i $proxy_args '$CONTAINER_NAME' build-cpp" >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 12 "Granular Command - build-cpp (alias)" $exit_code $duration
    return $exit_code
}

# Test 13: Granular Command - finalize
test_granular_finalize() {
    log_test_start 13 "Granular Command - finalize"
    
    local start_time=$(date +%s)
    local proxy_args=$(get_proxy_args)
    
    timeout "$TEST_TIMEOUT" bash -c "docker run --rm $proxy_args '$CONTAINER_NAME' finalize" >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 13 "Granular Command - finalize" $exit_code $duration
    return $exit_code
}

# Test 14: Run Command (build and run)
test_run_command() {
    log_test_start 14 "Run Command (build and run)"
    
    local start_time=$(date +%s)
    local env_args=$(get_env_args)
    # Run command always needs --network=host for service connectivity
    local network_host=""
    if [[ "$USE_PROXY" != "true" ]]; then
        network_host="--network=host"
    fi
    
    timeout "$TEST_TIMEOUT" bash -c "cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i $network_host $env_args '$CONTAINER_NAME' run" >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 14 "Run Command (build and run)" $exit_code $duration
    return $exit_code
}

# Test 15: Rerun Command (pre-built template)
test_rerun_command() {
    log_test_start 15 "Rerun Command (pre-built template)"
    
    local start_time=$(date +%s)
    local env_args=$(get_env_args)
    # Rerun command always needs --network=host for service connectivity
    local network_host=""
    if [[ "$USE_PROXY" != "true" ]]; then
        network_host="--network=host"
    fi
    
    timeout "$TEST_TIMEOUT" bash -c "docker run --rm $network_host $env_args '$CONTAINER_NAME' rerun" >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 15 "Rerun Command (pre-built template)" $exit_code $duration
    return $exit_code
}

# Test 16: Granular Workflow (sequential steps)
test_granular_workflow() {
    log_test_start 16 "Granular Workflow (sequential steps)"
    
    local start_time=$(date +%s)
    local proxy_args=$(get_proxy_args)
    local workflow_success=true
    
    echo "Step 1: Generate model..." >> "$LOG_FILE"
    if ! timeout "$TEST_TIMEOUT" bash -c "docker run --rm $proxy_args '$CONTAINER_NAME' gen-model" >> "$LOG_FILE" 2>&1; then
        workflow_success=false
    fi
    
    echo "Step 2: Compile application..." >> "$LOG_FILE"
    if [[ "$workflow_success" == "true" ]]; then
        if ! timeout "$TEST_TIMEOUT" bash -c "cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i $proxy_args '$CONTAINER_NAME' compile" >> "$LOG_FILE" 2>&1; then
            workflow_success=false
        fi
    fi
    
    echo "Step 3: Finalize build..." >> "$LOG_FILE"
    if [[ "$workflow_success" == "true" ]]; then
        if ! timeout "$TEST_TIMEOUT" bash -c "docker run --rm $proxy_args '$CONTAINER_NAME' finalize" >> "$LOG_FILE" 2>&1; then
            workflow_success=false
        fi
    fi
    
    local exit_code=0
    if [[ "$workflow_success" != "true" ]]; then
        exit_code=1
    fi
    
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log_test_result 16 "Granular Workflow (sequential steps)" $exit_code $duration
    return $exit_code
}

# Test 17: Verbose Build Mode
test_verbose_build_mode() {
    log_test_start 17 "Verbose Build Mode"
    
    local start_time=$(date +%s)
    local proxy_args=$(get_proxy_args)
    
    # Force verbose mode for this test regardless of USE_VERBOSE setting
    timeout "$TEST_TIMEOUT" bash -c "cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i $proxy_args -e VERBOSE_BUILD=1 '$CONTAINER_NAME' build" >> "$LOG_FILE" 2>&1
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    # Check if verbose output was actually generated
    if [[ $exit_code -eq 0 ]] && grep -q "Dependencies installed/verified successfully\|VSS specification downloaded\|C++ compilation completed successfully" "$LOG_FILE"; then
        log_test_result 17 "Verbose Build Mode" 0 $duration
        return 0
    else
        log_test_result 17 "Verbose Build Mode" 1 $duration
        return 1
    fi
}

# Run all tests
run_all_tests() {
    echo -e "${YELLOW}🚀 Starting Mode 2 Test Suite...${NC}"
    echo ""
    
    # Check if VehicleApp template exists
    if [[ ! -f "templates/app/src/VehicleApp.template.cpp" ]]; then
        echo -e "${RED}❌ ERROR: templates/app/src/VehicleApp.template.cpp not found${NC}"
        exit 1
    fi
    
    # Run basic tests
    test_container_build || true
    test_help_command || true
    test_basic_build_stdin || true
    test_validation_command || true
    test_custom_vss || true
    test_error_handling || true
    test_file_mount_input || true
    test_directory_mount_input || true
    
    # Run granular command tests
    test_granular_gen_model || true
    test_granular_model_alias || true
    test_granular_compile || true
    test_granular_build_cpp_alias || true
    test_granular_finalize || true
    
    # Run run/rerun command tests
    test_run_command || true
    test_rerun_command || true
    
    # Run granular workflow test
    test_granular_workflow || true
    
    # Run verbose mode test
    test_verbose_build_mode || true
}

# Print summary
print_summary() {
    echo ""
    echo -e "${YELLOW}📊 TEST SUMMARY${NC}"
    echo "============================================"
    echo "Total Tests: $TOTAL_TESTS"
    echo -e "Passed: ${GREEN}$PASSED_TESTS${NC}"
    echo -e "Failed: ${RED}$FAILED_TESTS${NC}"
    echo "Success Rate: $(( PASSED_TESTS * 100 / TOTAL_TESTS ))%"
    echo ""
    
    if [[ "$USE_PROXY" == "true" ]]; then
        echo -e "${BLUE}🔗 Proxy Configuration: $PROXY_HOST${NC}"
    else
        echo -e "${BLUE}🚫 No Proxy Configuration${NC}"
    fi
    
    echo -e "${BLUE}📁 Log File: $LOG_FILE${NC}"
    echo ""
    
    # Write summary to log file
    {
        echo ""
        echo "📊 TEST SUMMARY"
        echo "============================================"
        echo "Total Tests: $TOTAL_TESTS"
        echo "Passed: $PASSED_TESTS"
        echo "Failed: $FAILED_TESTS"
        echo "Success Rate: $(( PASSED_TESTS * 100 / TOTAL_TESTS ))%"
        echo "Completed: $(date)"
    } >> "$LOG_FILE"
    
    if [[ $FAILED_TESTS -gt 0 ]]; then
        echo -e "${RED}❌ Some tests failed. Check the log file for details.${NC}"
        exit 1
    else
        echo -e "${GREEN}✅ All tests passed successfully!${NC}"
        exit 0
    fi
}

# Cleanup function
cleanup() {
    echo ""
    echo -e "${YELLOW}🧹 Cleaning up...${NC}"
    
    # Remove test containers if they exist
    docker ps -a --format "table {{.Names}}" | grep -E "^test_" | xargs -r docker rm -f 2>/dev/null || true
    
    echo -e "${GREEN}✅ Cleanup completed${NC}"
}

# Signal handlers
trap cleanup EXIT
trap 'echo ""; echo -e "${RED}❌ Test interrupted${NC}"; exit 130' INT TERM

# Main execution
main() {
    parse_args "$@"
    init_test_env
    run_all_tests
    print_summary
}

# Run main function with all arguments
main "$@"