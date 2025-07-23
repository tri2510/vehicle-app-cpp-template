#!/bin/bash
# ============================================================================
# Find Executable Script - Reliable Binary Location
# ============================================================================
# Purpose: Find the built executable in various possible locations
# Usage: ./find-executable.sh [workspace_path]
# ============================================================================

WORKSPACE="${1:-/quickbuild}"

# Function to find executable
find_executable() {
    local executable_paths=(
        # Most common locations first
        "$WORKSPACE/build/bin/app"
        "$WORKSPACE/build-linux-x86_64/Release/bin/app"
        "$WORKSPACE/build-linux-x86_64/Debug/bin/app"
        "$WORKSPACE/app/build/bin/app"
        "$WORKSPACE/build/app"
        "$WORKSPACE/build/src/app"
        "$WORKSPACE/build-linux-x86_64/Release/src/app"
        "$WORKSPACE/build/Release/bin/app"
        "$WORKSPACE/build/Debug/bin/app"
    )
    
    # Check predefined paths
    for path in "${executable_paths[@]}"; do
        if [ -f "$path" ] && [ -x "$path" ]; then
            echo "$path"
            return 0
        fi
    done
    
    # If not found, do recursive search
    local found=$(find "$WORKSPACE" -name "app" -type f -executable 2>/dev/null | grep -E "(build|bin)" | grep -v ".conan" | grep -v "test" | head -1)
    
    if [ -n "$found" ]; then
        echo "$found"
        return 0
    fi
    
    return 1
}

# Main execution
EXECUTABLE=$(find_executable)

if [ -n "$EXECUTABLE" ]; then
    echo "$EXECUTABLE"
    exit 0
else
    exit 1
fi