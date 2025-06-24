#!/bin/bash
# Debug script to test flag parsing

echo "=== Testing Flag Detection ==="
echo "Args passed: $@"

# Source the velocitas script functions
source /scripts/velocitas-cli.sh 2>/dev/null || echo "Could not source script"

# Test flag detection
SKIP_DEPS=false
SKIP_VSS=false
VERBOSE=false

# Parse arguments manually
while [[ $# -gt 0 ]]; do
    case $1 in
        --skip-deps)
            SKIP_DEPS=true
            echo "✅ --skip-deps detected"
            shift
            ;;
        --skip-vss)
            SKIP_VSS=true
            echo "✅ --skip-vss detected"
            shift
            ;;
        --verbose)
            VERBOSE=true
            echo "✅ --verbose detected"
            shift
            ;;
        *)
            echo "Other arg: $1"
            shift
            ;;
    esac
done

echo "Final flags:"
echo "  SKIP_DEPS=$SKIP_DEPS"
echo "  SKIP_VSS=$SKIP_VSS"
echo "  VERBOSE=$VERBOSE"