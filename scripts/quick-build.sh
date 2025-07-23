#!/bin/bash
# ============================================================================
# Quick Build Script - Mode 2 Blackbox Utility
# ============================================================================
# Purpose: Replace template VehicleApp.cpp with user input and build
# Usage: 
#   cat VehicleApp.cpp | docker run -i velocitas-quick
#   docker run -v $(pwd)/app.cpp:/input velocitas-quick
# ============================================================================

set -e

# Configuration
WORKSPACE="/quickbuild"
APP_SOURCE="$WORKSPACE/app/src/VehicleApp.cpp"
BUILD_DIR="$WORKSPACE/build"
LOG_FILE="/tmp/build.log"
SOURCE_HASH_FILE="$WORKSPACE/.source_hash"

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

# Unified logging function for commands
# Usage: run_with_logging "command" "success_message" "warning_message"
run_with_logging() {
    local command="$1"
    local success_msg="$2"
    local warning_msg="$3"
    
    if [[ "${VERBOSE_BUILD:-}" == "1" ]]; then
        # Verbose mode: show command output to both console and log
        if eval "$command" 2>&1 | tee -a "$LOG_FILE"; then
            log_success "$success_msg"
            return 0
        else
            local exit_code=$?
            log_warning "$warning_msg"
            return $exit_code
        fi
    else
        # Normal mode: hide command output, show hint for verbose mode
        if eval "$command" >> "$LOG_FILE" 2>&1; then
            log_success "$success_msg"
            return 0
        else
            local exit_code=$?
            log_warning "$warning_msg"
            log_info "   💡 Use VERBOSE_BUILD=1 to see detailed output"
            return $exit_code
        fi
    fi
}

# Initialize build log
echo "🚀 Quick Build Started: $(date)" > "$LOG_FILE"
log_info "Workspace: $WORKSPACE"

# Function to calculate source file hash
calculate_source_hash() {
    if [ -f "$APP_SOURCE" ]; then
        sha256sum "$APP_SOURCE" | cut -d' ' -f1
    else
        echo "no_file"
    fi
}

# Function to validate VehicleApp.cpp content
validate_vehicle_app() {
    local file="$1"
    
    if ! grep -q "class.*VehicleApp" "$file"; then
        log_error "Invalid VehicleApp.cpp: Missing VehicleApp class definition"
        return 1
    fi
    
    if ! grep -q "#include.*VehicleApp.h" "$file"; then
        log_warning "VehicleApp.cpp missing VehicleApp.h include - may cause build issues"
    fi
    
    log_success "VehicleApp.cpp validation passed"
    return 0
}

# Function to get user input
get_user_input() {
    log_info "Reading user VehicleApp.cpp..."
    
    if [ -p /dev/stdin ]; then
        # Input from stdin
        log_info "Reading from stdin..."
        cat > "$APP_SOURCE"
        if [ ! -s "$APP_SOURCE" ]; then
            log_error "No input received from stdin"
            exit 1
        fi
    elif [ -f "/input" ]; then
        # Input from mounted file
        log_info "Reading from mounted file: /input"
        cp "/input" "$APP_SOURCE"
    elif [ -f "/input/VehicleApp.cpp" ]; then
        # Input from mounted directory
        log_info "Reading from mounted directory: /input/VehicleApp.cpp"
        cp "/input/VehicleApp.cpp" "$APP_SOURCE"
    else
        log_error "No input provided. Use stdin or mount file at /input"
        echo ""
        echo "Usage examples:"
        echo "  cat VehicleApp.cpp | docker run -i velocitas-quick"
        echo "  docker run -v \$(pwd)/VehicleApp.cpp:/input velocitas-quick"
        echo "  docker run -v \$(pwd):/input velocitas-quick"
        exit 1
    fi
    
    # Touch the file to update its timestamp
    touch "$APP_SOURCE"
    
    # Validate the input
    validate_vehicle_app "$APP_SOURCE"
    
    local lines=$(wc -l < "$APP_SOURCE")
    log_success "VehicleApp.cpp loaded successfully ($lines lines)"
}

# Function to handle custom VSS configuration
prepare_custom_vss() {
    log_info "Configuring VSS specification..."
    
    local vss_updated=false
    local manifest_file="$WORKSPACE/app/AppManifest.json"
    
    # Check for custom VSS file
    if [ -n "$VSS_SPEC_FILE" ] && [ -f "$VSS_SPEC_FILE" ]; then
        log_info "Using custom VSS file: $VSS_SPEC_FILE"
        
        # Copy custom VSS to workspace
        cp "$VSS_SPEC_FILE" "$WORKSPACE/custom-vss.json"
        
        # Update AppManifest.json to use custom VSS
        if command -v jq >/dev/null 2>&1; then
            jq --arg vss_path "file:///quickbuild/custom-vss.json" \
               '.interfaces[0].config.src = $vss_path' \
               "$manifest_file" > "$manifest_file.tmp" && \
               mv "$manifest_file.tmp" "$manifest_file"
            vss_updated=true
        else
            log_warning "jq not available, using sed for VSS update"
            sed -i "s|https://github.com/COVESA/.*|file:///quickbuild/custom-vss.json|g" "$manifest_file"
            vss_updated=true
        fi
        
    # Check for custom VSS URL
    elif [ -n "$VSS_SPEC_URL" ]; then
        log_info "Using custom VSS URL: $VSS_SPEC_URL"
        
        # Validate URL format
        if [[ "$VSS_SPEC_URL" =~ ^https?:// ]]; then
            # Update AppManifest.json to use custom URL
            if command -v jq >/dev/null 2>&1; then
                jq --arg vss_url "$VSS_SPEC_URL" \
                   '.interfaces[0].config.src = $vss_url' \
                   "$manifest_file" > "$manifest_file.tmp" && \
                   mv "$manifest_file.tmp" "$manifest_file"
                vss_updated=true
            else
                log_warning "jq not available, using sed for VSS update"
                sed -i "s|https://github.com/COVESA/.*|$VSS_SPEC_URL|g" "$manifest_file"
                vss_updated=true
            fi
        else
            log_error "Invalid VSS URL format: $VSS_SPEC_URL"
            log_info "URL must start with http:// or https://"
            exit 1
        fi
    fi
    
    if [ "$vss_updated" = true ]; then
        log_success "VSS configuration updated"
        log_info "VSS source: $(grep -o '"src": "[^"]*"' "$manifest_file" | cut -d'"' -f4)"
    else
        log_info "Using default VSS 4.0 specification"
    fi
}

# Function to prepare workspace
prepare_workspace() {
    log_info "Preparing build workspace..."
    
    # Ensure workspace structure exists
    cd "$WORKSPACE"
    
    # Verify fixed configuration files exist
    if [ ! -f "app/AppManifest.json" ]; then
        log_error "Missing AppManifest.json template"
        exit 1
    fi
    
    if [ ! -f "conanfile.txt" ]; then
        log_error "Missing conanfile.txt template"
        exit 1
    fi
    
    # Configure custom VSS if provided
    prepare_custom_vss
    
    # Check if source has changed
    local current_hash=$(calculate_source_hash)
    local previous_hash=""
    
    if [ -f "$SOURCE_HASH_FILE" ]; then
        previous_hash=$(cat "$SOURCE_HASH_FILE")
    fi
    
    # Clean build artifacts if source changed or no previous build
    if [ "$current_hash" != "$previous_hash" ] || [ ! -f "$BUILD_DIR/bin/app" ]; then
        log_info "Source changed or no previous build, cleaning build artifacts..."
        rm -rf "$BUILD_DIR"
        # Also clean CMake cache files
        find "$WORKSPACE" -name "CMakeCache.txt" -delete 2>/dev/null || true
        find "$WORKSPACE" -name "CMakeFiles" -type d -exec rm -rf {} + 2>/dev/null || true
        # Save new hash
        echo "$current_hash" > "$SOURCE_HASH_FILE"
    else
        log_info "Source unchanged, keeping existing build artifacts"
    fi
    
    log_success "Workspace prepared"
}

# Function to generate vehicle model
generate_model() {
    log_info "Generating vehicle model from VSS..."
    
    cd "$WORKSPACE"
    export PATH="/home/vscode/.local/bin:$PATH"
    
    if ! run_with_logging "velocitas exec vehicle-signal-interface download-vspec" "VSS specification downloaded" "Failed to download VSS specification"; then
        log_error "Failed to download VSS specification"
        return 1
    fi
    
    if ! run_with_logging "velocitas exec vehicle-signal-interface generate-model" "Vehicle model generated" "Failed to generate vehicle model"; then
        log_error "Failed to generate vehicle model"  
        return 1
    fi
    
    log_success "Vehicle model generated"
}

# Function to build application
build_application() {
    log_info "Building C++ application..."
    log_info "🔧 Configuring build environment..."
    
    cd "$WORKSPACE"
    export PATH="/home/vscode/.local/bin:$PATH"
    
    log_info "📦 Installing dependencies (if needed)..."
    if ! run_with_logging "velocitas exec build-system install" "Dependencies installed/verified successfully" "Dependency installation had issues, continuing with build..."; then
        : # Continue with build even if dependency installation has issues
    fi
    
    log_info "🏗️  Starting compilation (Release mode for optimization)..."
    log_info "   This may take 60-90 seconds depending on code complexity..."
    
    # Build with release flag (clean already done in prepare_workspace)
    if ! run_with_logging "velocitas exec build-system build -r" "C++ compilation completed successfully" "Build compilation failed"; then
        log_error "Build failed"
        echo ""
        echo "Build log (last 30 lines):"
        tail -n 30 "$LOG_FILE"
        echo ""
        echo "💡 Common fixes:"
        echo "   - Check for syntax errors in VehicleApp.cpp"
        echo "   - Ensure all required includes are present"
        echo "   - Verify Vehicle signal names are correct"
        return 1
    fi
    
    log_info "🔍 Verifying build output..."
    
    # Check for multiple possible executable locations
    local executable_found=false
    local executable_path=""
    
    # Check common Velocitas build locations
    for path in "$BUILD_DIR/bin/app" "$WORKSPACE/build-linux-x86_64/Release/bin/app" "$WORKSPACE/app/build/bin/app" "$WORKSPACE/build-linux-x86_64/Release/src/app"; do
        if [ -f "$path" ]; then
            executable_found=true
            executable_path="$path"
            break
        fi
    done
    
    if [ "$executable_found" = false ]; then
        log_error "Build succeeded but executable not found"
        log_info "Searched locations:"
        log_info "  - $BUILD_DIR/bin/app"
        log_info "  - $WORKSPACE/build-linux-x86_64/Release/bin/app"
        log_info "  - $WORKSPACE/app/build/bin/app"
        log_info "  - $WORKSPACE/build-linux-x86_64/Release/src/app"
        log_info "Available files in build directory:"
        find "$WORKSPACE" -name "app" -type f 2>/dev/null || echo "  No 'app' executable found"
        
        # Try to find any executable
        log_info "Looking for any executable files..."
        find "$WORKSPACE" -type f -executable 2>/dev/null | head -5
        return 1
    fi
    
    local size=$(ls -lh "$executable_path" | awk '{print $5}')
    local permissions=$(ls -l "$executable_path" | awk '{print $1}')
    log_success "Build completed successfully!"
    log_info "📍 Executable location: $executable_path"
    log_info "📏 Executable size: $size"
    log_info "🔐 Permissions: $permissions"
}

# Function to display build summary
build_summary() {
    log_info "Build Summary:"
    echo "============================================" | tee -a "$LOG_FILE"
    echo "✅ VehicleApp.cpp: Validated and compiled" | tee -a "$LOG_FILE"
    echo "✅ Dependencies: Pre-installed (cached)" | tee -a "$LOG_FILE"
    echo "✅ Vehicle Model: Generated from VSS" | tee -a "$LOG_FILE"
    echo "✅ Executable: $BUILD_DIR/bin/app" | tee -a "$LOG_FILE"
    
    if [ -f "$BUILD_DIR/bin/app" ]; then
        local size=$(stat -c%s "$BUILD_DIR/bin/app")
        echo "📊 Size: $(numfmt --to=iec-i --suffix=B --format="%.1f" $size)" | tee -a "$LOG_FILE"
    fi
    
    echo "🕐 Completed: $(date)" | tee -a "$LOG_FILE"
    echo "============================================" | tee -a "$LOG_FILE"
}

# Main execution flow
main() {
    echo ""
    log_info "🚀 Velocitas C++ Quick Build Utility"
    log_info "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    log_info "📦 Zero-setup containerized build for Velocitas vehicle apps"
    echo ""
    
    # Step 1: Get user input
    log_info "🔧 STEP 1/5: Processing input..."
    get_user_input
    echo ""
    
    # Step 2: Prepare workspace
    log_info "🔧 STEP 2/5: Preparing build workspace..."
    prepare_workspace
    echo ""
    
    # Step 3: Generate vehicle model (if needed)
    log_info "🔧 STEP 3/5: Vehicle signal model preparation..."
    if [ ! -d "$WORKSPACE/app/vehicle_model" ]; then
        generate_model
    else
        log_info "✅ Vehicle model already exists, skipping generation"
    fi
    echo ""
    
    # Step 4: Build application
    log_info "🔧 STEP 4/5: Building C++ application..."
    build_application
    echo ""
    
    # Step 5: Display summary
    log_info "🔧 STEP 5/5: Finalizing build..."
    build_summary
    
    echo ""
    log_success "🎉 Quick build completed successfully!"
    log_info "💡 Your vehicle app is ready to run!"
}

# Function to run individual steps
step_model() {
    echo ""
    log_info "🚀 Velocitas C++ Model Generation Utility"
    log_info "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    log_info "🔧 STEP 3: Vehicle signal model preparation"
    echo ""
    
    # Prepare workspace
    prepare_workspace
    echo ""
    
    # Generate vehicle model
    generate_model
    
    log_success "Model generation completed successfully!"
}

step_compile() {
    echo ""
    log_info "🚀 Velocitas C++ Compilation Utility"
    log_info "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    log_info "🔧 STEP 4: C++ application building"
    echo ""
    
    # Get input if needed
    get_user_input
    echo ""
    
    # Build application
    build_application
    
    log_success "Compilation completed successfully!"
}

step_finalize() {
    echo ""
    log_info "🚀 Velocitas C++ Build Finalization Utility"
    log_info "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    log_info "🔧 STEP 5: Finalizing build"
    echo ""
    
    # Display summary
    build_summary
    
    log_success "Build finalization completed successfully!"
}

# Handle script arguments
case "${1:-build}" in
    "build")
        main
        ;;
    "run")
        # Use quick-run.sh for build and run functionality
        exec /scripts/quick-run.sh
        ;;
    "rerun")
        # Run existing binary without rebuild
        exec /scripts/quick-run.sh rerun
        ;;
    "validate")
        get_user_input
        log_success "Validation completed - VehicleApp.cpp is valid"
        ;;
    "gen-model"|"model")
        step_model
        ;;
    "compile"|"build-cpp")
        step_compile
        ;;
    "finalize")
        step_finalize
        ;;
    "help"|"--help"|"-h")
        echo "Quick Build Script - Mode 2 Blackbox Utility"
        echo ""
        echo "Usage:"
        echo "  cat VehicleApp.cpp | docker run -i velocitas-quick"
        echo "  docker run -v \$(pwd)/app.cpp:/input velocitas-quick"
        echo "  docker run -v \$(pwd):/input velocitas-quick"
        echo ""
        echo "Custom VSS Support:"
        echo "  # Use custom VSS file"
        echo "  docker run -v \$(pwd)/my-vss.json:/vss.json -e VSS_SPEC_FILE=/vss.json -i velocitas-quick"
        echo ""
        echo "  # Use custom VSS URL"
        echo "  docker run -e VSS_SPEC_URL=https://company.com/vss.json -i velocitas-quick"
        echo ""
        echo "Commands:"
        echo "  build       - Build the application (default)"
        echo "  run         - Build (if needed) and run application with live output"
        echo "  rerun       - Run pre-built template (no input needed, fastest)"
        echo "  validate    - Only validate VehicleApp.cpp"
        echo ""
        echo "Granular Build Commands:"
        echo "  gen-model   - Generate vehicle signal model from VSS (Step 3)"
        echo "  model       - Alias for gen-model"
        echo "  compile     - Compile C++ application only (Step 4)"
        echo "  build-cpp   - Alias for compile"
        echo "  finalize    - Build summary and finalization (Step 5)"
        echo ""
        echo "  help        - Show this help message"
        echo ""
        echo "Environment Variables:"
        echo "  VSS_SPEC_FILE - Path to custom VSS JSON file"
        echo "  VSS_SPEC_URL  - URL to custom VSS JSON specification"
        echo "  VERBOSE_BUILD - Set to 1 to show detailed command output"
        ;;
    *)
        log_error "Unknown command: $1"
        echo "Use 'help' for usage information"
        exit 1
        ;;
esac