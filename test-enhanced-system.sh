#!/bin/bash
# ============================================================================
# Test Enhanced Build System
# ============================================================================
# Purpose: Validate the new unified CLI and enhanced features
# Usage: ./test-enhanced-system.sh
# ============================================================================

set -e

echo "🧪 Testing Enhanced Vehicle App Build System"
echo "============================================"

# Test 1: Help system
echo ""
echo "📖 Test 1: Help system"
echo "Testing: docker run --rm velocitas-quick help"
if docker run --rm velocitas-quick help | grep -q "Velocitas CLI"; then
    echo "✅ Help system working"
else
    echo "❌ Help system failed"
    exit 1
fi

# Test 2: File mounting validation
echo ""
echo "📁 Test 2: File mounting"
echo "Testing: File mounting with verbose flag"
if docker run --rm -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp velocitas-quick validate | grep -q "Source validated"; then
    echo "✅ File mounting working"
else
    echo "❌ File mounting failed"
    exit 1
fi

# Test 3: Build with new CLI
echo ""
echo "🏗️  Test 3: Build system"
echo "Testing: Build with verbose flag"
if docker run --rm -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp velocitas-quick build --verbose | grep -q "Build completed"; then
    echo "✅ Build system working"
else
    echo "❌ Build system failed"
    exit 1
fi

# Test 4: SDV Example validation
echo ""
echo "🚗 Test 4: SDV Example"
echo "Testing: FleetManagementSDV example validation"
if docker run --rm -v $(pwd)/examples/FleetManagementSDV.cpp:/app.cpp velocitas-quick validate | grep -q "Source validated"; then
    echo "✅ SDV example validation working"
else
    echo "❌ SDV example validation failed"
    exit 1
fi

echo ""
echo "🎉 All tests passed! Enhanced build system is working correctly."
echo ""
echo "🚀 Quick Start with Enhanced System:"
echo "  # Build with file mounting:"
echo "  docker run -v \$(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp velocitas-quick build --verbose"
echo ""
echo "  # Try SDV Fleet Management example:"
echo "  docker run -v \$(pwd)/examples/FleetManagementSDV.cpp:/app.cpp velocitas-quick build --verbose"
echo ""
echo "  # Test with signal validation:"
echo "  docker run -v \$(pwd)/examples/FleetManagementSDV.cpp:/app.cpp velocitas-quick test signal-validation"