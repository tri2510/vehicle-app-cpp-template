# Vehicle App Test Instructions

## Overview
This document provides step-by-step instructions for testing the Velocitas C++ Vehicle App with interactive Docker monitoring. The test validates the complete workflow from building to signal injection without using timers.

## Prerequisites
- Docker installed and running
- Docker-compose installed
- No proxy requirements (machine configured to run without proxy)

## Test Procedure

### Step 1: Clean Docker Environment

```bash
# Stop all running containers
docker stop $(docker ps -aq) 2>/dev/null || true

# Prune all Docker containers, images and volumes
docker system prune -af --volumes
```

**Expected Result:** Clean Docker environment with all previous containers and images removed.

### Step 2: Build Enhanced Docker Container

```bash
# Build the velocitas-quick container without proxy
docker build -f Dockerfile.quick -t velocitas-quick .
```

**Monitor Progress:**
```bash
# Check build completion
docker images | grep velocitas-quick
```

**Expected Result:** 
```
velocitas-quick    latest    [IMAGE_ID]    [TIME]    3.28GB
```

### Step 3: Install Docker-Compose (if needed)

```bash
# Check if docker-compose is available
docker-compose --version

# If not available, install it
sudo apt update
sudo apt install -y docker-compose
```

**Expected Result:**
```
docker-compose version 1.29.2, build unknown
```

### Step 4: Start KUKSA Databroker Service

```bash
# Start KUKSA databroker using docker-compose
docker-compose -f docker-compose.dev.yml up -d vehicledatabroker
```

**Monitor Status:**
```bash
# Check running containers
docker ps | grep velocitas

# Check databroker logs
docker logs velocitas-vdb --tail 5
```

**Expected Result:**
```
[INFO] Listening on 0.0.0.0:55555
[INFO] TLS is not enabled
[INFO] Authorization is not enabled.
```

### Step 5: Build Vehicle App with Optimization

```bash
# Build vehicle app with performance optimization flags
docker run --rm --network host \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  -v $(pwd)/scripts/velocitas-cli.sh:/scripts/velocitas-cli.sh \
  velocitas-quick build --skip-deps --skip-vss --verbose
```

**Expected Result:**
```
✅ [SUCCESS] C++ compilation completed successfully
📍 Executable: /quickbuild/build/bin/app
📏 Size: 14M
```

### Step 6: Run Vehicle Application

```bash
# Remove any existing vehicle app container
docker rm -f vehicle-app 2>/dev/null || true

# Run vehicle application for 60 seconds
docker run -d --network host --name vehicle-app \
  -e SDV_VEHICLEDATABROKER_ADDRESS=127.0.0.1:55555 \
  -e HTTP_PROXY= -e HTTPS_PROXY= -e http_proxy= -e https_proxy= \
  -v $(pwd)/templates/app/src/VehicleApp.cpp:/app.cpp \
  velocitas-quick run 60
```

**Monitor Application Status:**
```bash
# Check container status (should show healthy)
docker ps | grep vehicle-app

# Monitor application logs in real-time
docker logs vehicle-app --follow
```

**Expected Logs:**
```
INFO  : 📡 Connecting to Vehicle Data Broker...
INFO  : ✅ Application initialized successfully
INFO  : 🚀 Vehicle Application started!
INFO  : ✅ Signal subscriptions completed
INFO  : 🔄 Waiting for vehicle signals...
INFO  : App is running.
```

### Step 7: Inject Vehicle Signal and Verify

**Open a second terminal** for signal injection while monitoring the first terminal.

```bash
# Inject speed signal to test vehicle app response
echo "setValue Vehicle.Speed 65.0" | docker run --rm -i --network host \
  -e HTTP_PROXY= -e HTTPS_PROXY= -e http_proxy= -e https_proxy= \
  ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
```

**Monitor Vehicle App Response:**
In the first terminal monitoring logs, you should see:
```bash
# Check vehicle app received and processed the signal
docker logs vehicle-app --tail 20
```

**Expected Response:**
```
INFO  : 📡 Received vehicle signal update
INFO  : 🚗 Vehicle Speed: 234.0 km/h (65.00 m/s)
WARN  : ⚠️ HIGH SPEED WARNING: 234.0 km/h
```

### Step 8: Test Additional Signals (Optional)

```bash
# Test different speed values
echo "setValue Vehicle.Speed 5.0" | docker run --rm -i --network host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Speed 15.0" | docker run --rm -i --network host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555

echo "setValue Vehicle.Speed 25.0" | docker run --rm -i --network host ghcr.io/eclipse-kuksa/kuksa-python-sdk/kuksa-client:main grpc://127.0.0.1:55555
```

**Expected Responses:**
```
INFO: 🚶 Very slow: 18.0 km/h                    # 5.0 m/s
INFO: 🏘️  City driving speed: 54.0 km/h         # 15.0 m/s  
INFO: 🚗 Normal highway speed: 90.0 km/h        # 25.0 m/s
```

## Interactive Monitoring Commands

### Real-time Container Status
```bash
# Monitor all running containers
watch "docker ps | grep -E '(CONTAINER|velocitas|vehicle)'"

# Monitor specific container health
watch "docker ps --format 'table {{.Names}}\t{{.Status}}\t{{.Ports}}' | grep -E '(NAMES|vehicle|velocitas)'"
```

### Real-time Log Monitoring
```bash
# Follow vehicle app logs
docker logs vehicle-app --follow --tail 10

# Follow databroker logs
docker logs velocitas-vdb --follow --tail 10

# Monitor all logs simultaneously (in separate terminals)
# Terminal 1:
docker logs vehicle-app --follow

# Terminal 2:
docker logs velocitas-vdb --follow
```

### Dynamic Status Checks
```bash
# Check container status before and after actions
docker ps | grep vehicle-app && echo "--- ACTION ---" && sleep 3 && docker ps | grep vehicle-app

# Monitor resource usage
docker stats vehicle-app velocitas-vdb --no-stream

# Check network connectivity
docker exec vehicle-app netstat -tlnp | grep 55555
```

## Success Criteria

### ✅ Container Build Success
- velocitas-quick image built (≈3.28GB)
- No build errors in output

### ✅ Service Startup Success  
- KUKSA databroker running on port 55555
- Vehicle app container shows "healthy" status
- No connection errors in logs

### ✅ Signal Processing Success
- Vehicle app connects to databroker
- Signal subscriptions complete successfully
- Injected signals processed and logged
- Business logic triggered (speed warnings)

### ✅ Expected Signal Response
For speed signal `65.0 m/s`:
- **Conversion:** 234.0 km/h calculated correctly
- **Logic:** High speed warning triggered
- **Format:** Proper logging format maintained

## Troubleshooting

### Container Not Starting
```bash
# Check container status
docker ps -a | grep vehicle-app

# Check exit code and logs
docker inspect vehicle-app | grep ExitCode
docker logs vehicle-app
```

### Signal Injection Fails
```bash
# Verify databroker is accessible
curl -v telnet://127.0.0.1:55555

# Check network connectivity
docker exec vehicle-app nc -zv 127.0.0.1 55555
```

### No Signal Response
```bash
# Verify subscription status in logs
docker logs vehicle-app | grep -i subscription

# Check for connection issues
docker logs vehicle-app | grep -i error
```

## Cleanup

```bash
# Stop all test containers
docker stop vehicle-app velocitas-vdb

# Clean up using docker-compose
docker-compose -f docker-compose.dev.yml down

# Remove test containers
docker rm vehicle-app

# Optional: Clean all test artifacts
docker system prune -f
```

## Performance Benchmarks

- **Container Build Time:** 3-5 minutes (with caching: 30-60 seconds)
- **App Build Time:** 15-30 seconds (with --skip-deps --skip-vss)
- **Signal Response Time:** < 100ms
- **Memory Usage:** ~500MB runtime, ~2GB during build
- **Executable Size:** ~14MB optimized binary

---

**Note:** This test procedure validates the complete Vehicle App workflow without using timers, relying instead on interactive monitoring and real-time log observation for verification.