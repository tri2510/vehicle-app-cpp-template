#!/bin/bash
# ============================================================================
# Template Validation Script - Mode 2 Blackbox Utility
# ============================================================================
# Purpose: Validate VehicleApp.cpp follows template structure and best practices
# Usage: 
#   cat VehicleApp.cpp | docker run -i velocitas-quick validate
#   docker run -v $(pwd)/app.cpp:/input velocitas-quick validate
# ============================================================================

set -e

# Configuration
LOG_FILE="/tmp/validate.log"

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

log_tip() {
    echo "💡 [TIP] $1" | tee -a "$LOG_FILE"
}

# Initialize validation log
echo "🔍 Template Validation Started: $(date)" > "$LOG_FILE"

# Function to get input file
get_input_file() {
    local temp_file="/tmp/input_vehicleapp.cpp"
    
    if [ -p /dev/stdin ]; then
        # Input from stdin
        log_info "Reading from stdin..."
        cat > "$temp_file"
    elif [ -f "/input" ]; then
        # Input from mounted file
        log_info "Reading from mounted file: /input"
        cp "/input" "$temp_file"
    elif [ -f "/input/VehicleApp.cpp" ]; then
        # Input from mounted directory
        log_info "Reading from mounted directory: /input/VehicleApp.cpp"
        cp "/input/VehicleApp.cpp" "$temp_file"
    else
        log_error "No input provided"
        return 1
    fi
    
    if [ ! -s "$temp_file" ]; then
        log_error "Input file is empty"
        return 1
    fi
    
    echo "$temp_file"
}

# Function to validate basic C++ structure
validate_cpp_structure() {
    local file="$1"
    local errors=0
    
    log_info "Validating C++ structure..."
    
    # Check for required includes
    if ! grep -q "#include.*VehicleApp.h\|#include.*sdk/VehicleApp.h" "$file"; then
        log_error "Missing required include: VehicleApp.h"
        log_tip "Add: #include \"sdk/VehicleApp.h\""
        ((errors++))
    fi
    
    if ! grep -q "#include.*Logger.h\|#include.*sdk/Logger.h" "$file"; then
        log_warning "Missing recommended include: Logger.h"
        log_tip "Add: #include \"sdk/Logger.h\" for logging functionality"
    fi
    
    if ! grep -q "#include.*Vehicle.hpp\|#include.*vehicle/Vehicle.hpp" "$file"; then
        log_warning "Missing vehicle model include: Vehicle.hpp"
        log_tip "Add: #include \"vehicle/Vehicle.hpp\" for vehicle signals"
    fi
    
    # Check for class definition
    if ! grep -q "class.*:.*public.*VehicleApp\|class.*VehicleApp" "$file"; then
        log_error "Missing VehicleApp class definition"
        log_tip "Define class inheriting from VehicleApp: class MyApp : public velocitas::VehicleApp"
        ((errors++))
    fi
    
    # Check for basic class structure
    if ! grep -q "public:\|protected:\|private:" "$file"; then
        log_warning "No access modifiers found - consider organizing class members"
    fi
    
    return $errors
}

# Function to validate Velocitas-specific patterns
validate_velocitas_patterns() {
    local file="$1"
    local errors=0
    
    log_info "Validating Velocitas patterns..."
    
    # Check for onStart method
    if ! grep -q "onStart\|void.*onStart" "$file"; then
        log_warning "Missing onStart() method"
        log_tip "Implement onStart() method for initialization"
    fi
    
    # Check for signal subscription patterns
    if grep -q "subscribeDataPoints\|subscribe.*Signal" "$file"; then
        log_success "Signal subscription detected"
    else
        log_warning "No signal subscriptions found"
        log_tip "Consider subscribing to vehicle signals in onStart()"
    fi
    
    # Check for signal handling
    if grep -q "onDataPointUpdate\|onSignal.*Update\|onSignal.*Changed" "$file"; then
        log_success "Signal handling detected"
    else
        log_warning "No signal handling methods found"
        log_tip "Implement signal handling methods for data processing"
    fi
    
    # Check for Vehicle instance usage
    if grep -q "Vehicle\." "$file"; then
        log_success "Vehicle signal access detected"
    else
        log_warning "No vehicle signal access found"
        log_tip "Access vehicle signals using Vehicle.Speed, Vehicle.Cabin.*, etc."
    fi
    
    # Check for logger usage
    if grep -q "logger()\|Logger::" "$file"; then
        log_success "Logging usage detected"
    else
        log_warning "No logging usage found"
        log_tip "Use logger().info(), logger().warn(), etc. for debugging"
    fi
    
    return $errors
}

# Function to validate template areas
validate_template_areas() {
    local file="$1"
    local warnings=0
    
    log_info "Checking template modification areas..."
    
    # Check for template area markers
    if grep -q "🔧 TEMPLATE AREA\|TEMPLATE AREA" "$file"; then
        log_success "Template area markers found"
        
        # List template areas found
        log_info "Template areas detected:"
        grep -n "🔧 TEMPLATE AREA\|TEMPLATE AREA" "$file" | while read -r line; do
            echo "   Line $(echo "$line" | cut -d: -f1): $(echo "$line" | cut -d: -f2-)" | tee -a "$LOG_FILE"
        done
    else
        log_warning "No template area markers found"
        log_tip "Look for template areas marked with '🔧 TEMPLATE AREA' comments"
        ((warnings++))
    fi
    
    return $warnings
}

# Function to check code quality
validate_code_quality() {
    local file="$1"
    local warnings=0
    
    log_info "Checking code quality..."
    
    # Check for memory management
    if grep -q "new\|malloc" "$file" && ! grep -q "delete\|free\|smart_ptr\|unique_ptr\|shared_ptr" "$file"; then
        log_warning "Manual memory allocation detected without cleanup"
        log_tip "Consider using smart pointers for automatic memory management"
        ((warnings++))
    fi
    
    # Check for hardcoded values
    if grep -q '"[^"]*\.[^"]*"\|[0-9]\{3,\}' "$file"; then
        log_warning "Hardcoded values detected"
        log_tip "Consider using constants or configuration for magic numbers/strings"
        ((warnings++))
    fi
    
    # Check for error handling
    if ! grep -q "try\|catch\|exception" "$file"; then
        log_warning "No error handling detected"
        log_tip "Consider adding try-catch blocks for robust error handling"
        ((warnings++))
    fi
    
    # Check for TODO/FIXME comments
    if grep -q "TODO\|FIXME\|XXX" "$file"; then
        log_warning "TODO/FIXME comments found"
        log_info "Remaining work items:"
        grep -n "TODO\|FIXME\|XXX" "$file" | while read -r line; do
            echo "   $(echo "$line" | cut -d: -f1-)" | tee -a "$LOG_FILE"
        done
        ((warnings++))
    fi
    
    return $warnings
}

# Function to validate syntax (basic)
validate_syntax() {
    local file="$1"
    local errors=0
    
    log_info "Performing basic syntax validation..."
    
    # Check for balanced braces
    local open_braces=$(grep -o "{" "$file" | wc -l)
    local close_braces=$(grep -o "}" "$file" | wc -l)
    
    if [ "$open_braces" -ne "$close_braces" ]; then
        log_error "Unbalanced braces: $open_braces opening, $close_braces closing"
        ((errors++))
    else
        log_success "Braces balanced ($open_braces pairs)"
    fi
    
    # Check for balanced parentheses
    local open_parens=$(grep -o "(" "$file" | wc -l)
    local close_parens=$(grep -o ")" "$file" | wc -l)
    
    if [ "$open_parens" -ne "$close_parens" ]; then
        log_error "Unbalanced parentheses: $open_parens opening, $close_parens closing"
        ((errors++))
    else
        log_success "Parentheses balanced ($open_parens pairs)"
    fi
    
    # Check for semicolons in class/function definitions
    if grep -q "class.*;" "$file"; then
        log_warning "Semicolon found after class definition (should be class Name {)"
        ((errors++))
    fi
    
    return $errors
}

# Function to display validation summary
validation_summary() {
    local total_errors="$1"
    local total_warnings="$2"
    local file_size="$3"
    
    log_info "Validation Summary:"
    echo "============================================" | tee -a "$LOG_FILE"
    echo "📋 File Analysis:" | tee -a "$LOG_FILE"
    echo "   - File size: $file_size bytes" | tee -a "$LOG_FILE"
    echo "   - Lines: $(wc -l < "$input_file")" | tee -a "$LOG_FILE"
    echo "   - Errors: $total_errors" | tee -a "$LOG_FILE"
    echo "   - Warnings: $total_warnings" | tee -a "$LOG_FILE"
    
    if [ "$total_errors" -eq 0 ] && [ "$total_warnings" -eq 0 ]; then
        echo "🎉 Perfect! No issues found." | tee -a "$LOG_FILE"
    elif [ "$total_errors" -eq 0 ]; then
        echo "✅ Good! Only minor warnings." | tee -a "$LOG_FILE"
    else
        echo "❌ Issues found that need attention." | tee -a "$LOG_FILE"
    fi
    
    echo "🕐 Completed: $(date)" | tee -a "$LOG_FILE"
    echo "============================================" | tee -a "$LOG_FILE"
}

# Function to provide validation help
show_help() {
    echo "Template Validation Script - Mode 2 Blackbox Utility"
    echo ""
    echo "This script validates VehicleApp.cpp for template compliance and best practices."
    echo ""
    echo "Usage:"
    echo "  cat VehicleApp.cpp | docker run -i velocitas-quick validate"
    echo "  docker run -v \$(pwd)/app.cpp:/input velocitas-quick validate"
    echo "  docker run -v \$(pwd):/input velocitas-quick validate"
    echo ""
    echo "Validation Checks:"
    echo "  ✅ C++ structure and syntax"
    echo "  ✅ Velocitas framework patterns"
    echo "  ✅ Template area compliance"
    echo "  ✅ Code quality guidelines"
    echo "  ✅ Common pitfalls and issues"
    echo ""
    echo "Exit Codes:"
    echo "  0 - Validation passed (no errors)"
    echo "  1 - Validation failed (errors found)"
    echo "  2 - Validation warnings only"
}

# Main execution flow
main() {
    log_info "Template Validation Mode - VehicleApp.cpp Analysis"
    
    # Get input file
    local input_file
    if ! input_file=$(get_input_file); then
        log_error "Failed to get input file"
        exit 1
    fi
    
    local file_size=$(stat -c%s "$input_file")
    log_success "Input file loaded ($(numfmt --to=iec-i --suffix=B --format="%.1f" $file_size))"
    
    # Run validation checks
    local total_errors=0
    local total_warnings=0
    
    # C++ structure validation
    if ! validate_cpp_structure "$input_file"; then
        ((total_errors += $?))
    fi
    
    # Velocitas patterns validation
    if ! validate_velocitas_patterns "$input_file"; then
        ((total_warnings += $?))
    fi
    
    # Template areas validation
    if ! validate_template_areas "$input_file"; then
        ((total_warnings += $?))
    fi
    
    # Code quality validation
    if ! validate_code_quality "$input_file"; then
        ((total_warnings += $?))
    fi
    
    # Syntax validation
    if ! validate_syntax "$input_file"; then
        ((total_errors += $?))
    fi
    
    # Display summary
    validation_summary "$total_errors" "$total_warnings" "$file_size"
    
    # Cleanup
    rm -f "$input_file"
    
    # Exit with appropriate code
    if [ "$total_errors" -gt 0 ]; then
        log_error "Validation failed with $total_errors errors"
        exit 1
    elif [ "$total_warnings" -gt 0 ]; then
        log_warning "Validation completed with $total_warnings warnings"
        exit 2
    else
        log_success "Validation passed successfully!"
        exit 0
    fi
}

# Handle script execution
case "${1:-validate}" in
    "validate")
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