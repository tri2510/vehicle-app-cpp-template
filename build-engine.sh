#!/bin/bash
set -e

echo "🔨 Building Velocitas Engine..."

if [ "$1" = "--proxy" ]; then
    echo "🌐 Building with proxy support..."
    docker build -f Dockerfile.engine \
        --build-arg HTTP_PROXY=http://127.0.0.1:3128 \
        --build-arg HTTPS_PROXY=http://127.0.0.1:3128 \
        --build-arg http_proxy=http://127.0.0.1:3128 \
        --build-arg https_proxy=http://127.0.0.1:3128 \
        --network=host \
        -t velocitas-engine .
else
    echo "🏗️  Building with standard configuration..."
    docker build -f Dockerfile.engine -t velocitas-engine .
fi

echo "✅ Engine built successfully!"
echo ""
echo "💡 Usage:"
echo "  ./build-engine.sh         # Standard build"
echo "  ./build-engine.sh --proxy # Build with proxy support"
echo ""
echo "🚀 Quick test with example:"
echo "# Standard:"
echo "docker run --rm --network=host -v \$(pwd)/examples/minimal-app:/input velocitas-engine build-run"
echo ""
echo "# With proxy:"
echo "docker run --rm --network=host \\"
echo "  -v \$(pwd)/examples/minimal-app:/input \\"
echo "  -e HTTP_PROXY=http://127.0.0.1:3128 \\"
echo "  -e HTTPS_PROXY=http://127.0.0.1:3128 \\"
echo "  -e http_proxy=http://127.0.0.1:3128 \\"
echo "  -e https_proxy=http://127.0.0.1:3128 \\"
echo "  velocitas-engine build-run"
echo ""
echo "📖 Full documentation: ENGINE.md"