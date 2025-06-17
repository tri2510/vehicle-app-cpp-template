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

OPTIONS:
    -p, --proxy         Enable proxy testing (default: false)
    --proxy-host HOST   Proxy host:port (default: 127.0.0.1:3128)
    -t, --timeout SEC   Test timeout in seconds (default: 120)
    -o, --output DIR    Output directory for logs (default: test_results)
    -h, --help          Show this help message

EXAMPLES:
    # Run tests without proxy
    $0

    # Run tests with proxy
    $0 --proxy

    # Run tests with custom proxy host
    $0 --proxy --proxy-host 192.168.1.100:8080

    # Run tests with custom timeout and output directory
    $0 --proxy --timeout 180 --output my_test_results

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

# Helper function to build proxy build arguments
get_proxy_build_args() {
    if [[ "$USE_PROXY" == "true" ]]; then
        echo "--build-arg HTTP_PROXY=http://$PROXY_HOST --build-arg HTTPS_PROXY=http://$PROXY_HOST --build-arg http_proxy=http://$PROXY_HOST --build-arg https_proxy=http://$PROXY_HOST --network=host"
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
    
    if [[ "$USE_PROXY" == "true" ]]; then
        docker build -f Dockerfile.quick $build_args -t "$CONTAINER_NAME" . >> "$LOG_FILE" 2>&1
    else
        docker build -f Dockerfile.quick -t "$CONTAINER_NAME" . >> "$LOG_FILE" 2>&1
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
    local proxy_args=$(get_proxy_args)
    
    timeout "$TEST_TIMEOUT" bash -c "cat templates/app/src/VehicleApp.template.cpp | docker run --rm -i $proxy_args '$CONTAINER_NAME' build" >> "$LOG_FILE" 2>&1
    
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

# Run all tests
run_all_tests() {
    echo -e "${YELLOW}🚀 Starting Mode 2 Test Suite...${NC}"
    echo ""
    
    # Check if VehicleApp template exists
    if [[ ! -f "templates/app/src/VehicleApp.template.cpp" ]]; then
        echo -e "${RED}❌ ERROR: templates/app/src/VehicleApp.template.cpp not found${NC}"
        exit 1
    fi
    
    # Run tests
    test_container_build || true
    test_help_command || true
    test_basic_build_stdin || true
    test_validation_command || true
    test_custom_vss || true
    test_error_handling || true
    test_file_mount_input || true
    test_directory_mount_input || true
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