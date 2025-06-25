#!/bin/bash

# Fleet Speed Monitor Manual Testing Script
# Sends all required signals in rapid sequence

echo "🧪 Testing Fleet Speed Monitor - Sending complete signal set..."

# Set GPS coordinates (New York City)
echo "📍 Setting GPS coordinates..."
echo "setValue Vehicle.CurrentLocation.Latitude 40.7128" | docker run --rm -i --network host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.CurrentLocation.Longitude -74.0060" | docker run --rm -i --network host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

# Small delay to let GPS signals settle
sleep 1

# Set speed signal
echo "🚗 Setting speed signal..."
echo "setValue Vehicle.Speed 15.0" | docker run --rm -i --network host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "✅ All signals sent! Check fleet-monitor logs for processing."