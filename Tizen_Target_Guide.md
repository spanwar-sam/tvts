# Tizen DRM Test Suite - Target Build and Testing Guide

This guide provides step-by-step instructions for building and testing the DRM Test Suite on Tizen targets.

## Prerequisites

### Software Requirements
- Tizen SDK >= 7.0
- GBS (Graphical Build System) >= 0.14
- Tizen Studio >= 4.0
- Tizen target device or emulator
- Tizen DRM development packages

### Hardware Requirements
- Tizen device with DRM support
- USB cable for device connection
- Debug mode enabled on target device

## Building for Tizen Target

### 1. Set Up Build Environment
```bash
# Initialize GBS environment
gbs env --setup

# Create project directory
gbs build --create-project
```

### 2. Prepare Source Code
```bash
# Create source tarball
make dist

# Copy to GBS workspace
cp tizen_drm_test_suite.spec ~/tizen-workspace/tizen-drm-test-suite/
cp tizen-drm-test-suite-1.0.0.tar.gz ~/tizen-workspace/tizen-drm-test-suite/
```

### 3. Configure Target
```bash
# Set target architecture
export ARCH=armv7l

# Set target profile
export PROFILE=mobile
```

### 4. Build Package
```bash
# Build package
gbs build --include-all --arch $ARCH --profile $PROFILE

# Build with debug symbols (optional)
gbs build --include-all --arch $ARCH --profile $PROFILE --debug
```

## Installing on Target

### 1. Connect Target Device
```bash
# List connected targets
sdb devices

# Ensure device is in debug mode
sdb shell "setprop debug.drm.enable 1"
```

### 2. Install Package
```bash
# Install package
sdb push /path/to/package.rpm /tmp/
sdb shell "rpm -Uvh /tmp/package.rpm"
```

## Running Tests

### 1. Basic Test Execution
```bash
# Run test suite
sdb shell "/usr/bin/test_suite"

# View test results
sdb shell "cat /tmp/test_results.log"
```

### 2. Detailed Test Execution
```bash
# Run specific tests
sdb shell "/usr/bin/test_suite --test buffer_sharing"
sdb shell "/usr/bin/test_suite --test format_conversion"

# Run with verbose output
sdb shell "/usr/bin/test_suite --verbose"
```

## Test Cases

### 1. Buffer Sharing Tests
```bash
# Test GEM buffer sharing
sdb shell "/usr/bin/test_suite --test buffer_sharing --format ARGB32"

# Test DMA-BUF sharing
sdb shell "/usr/bin/test_suite --test dma_buf_sharing"
```

### 2. Format Conversion Tests
```bash
# Test RGB to YUV conversion
sdb shell "/usr/bin/test_suite --test format_conversion --src_format ARGB32 --dst_format NV12"

# Test YUV to RGB conversion
sdb shell "/usr/bin/test_suite --test format_conversion --src_format NV12 --dst_format ARGB32"
```

### 3. Plane Configuration Tests
```bash
# Test primary plane
sdb shell "/usr/bin/test_suite --test plane_config --plane primary"

# Test overlay plane
sdb shell "/usr/bin/test_suite --test plane_config --plane overlay"
```

### 4. Performance Tests
```bash
# Run buffer sharing performance test
sdb shell "/usr/bin/test_suite --test performance --test_case buffer_sharing"

# Run format conversion performance test
sdb shell "/usr/bin/test_suite --test performance --test_case format_conversion"
```

## Debugging

### 1. Enable DRM Debug Logs
```bash
# Enable DRM debug logs
sdb shell "setprop debug.drm.log 1"

# View DRM logs
sdb shell "logcat -s DRM"
```

### 2. Enable Test Suite Debug Logs
```bash
# Run with debug output
sdb shell "/usr/bin/test_suite --debug"

# View test logs
sdb shell "cat /tmp/test_debug.log"
```

### 3. Memory Debugging
```bash
# Run with memory profiling
sdb shell "valgrind --tool=memcheck /usr/bin/test_suite"

# View memory report
sdb shell "cat /tmp/memcheck_report.txt"
```

## Troubleshooting

### Common Issues

1. **Build Errors**
   - Check if all build dependencies are installed
   - Verify GBS environment is properly set up
   - Check for architecture-specific issues

2. **Test Failures**
   - Check DRM driver version compatibility
   - Verify DRM permissions
   - Check for memory allocation failures

3. **Performance Issues**
   - Monitor CPU usage
   - Check for buffer leaks
   - Verify synchronization primitives

### Debugging Tips

1. **Check DRM Capabilities**
```bash
sdb shell "cat /sys/class/drm/card0/device/drm/caps"
```

2. **Verify Buffer Formats**
```bash
sdb shell "/usr/bin/test_suite --test buffer_formats"
```

3. **Check Memory Usage**
```bash
sdb shell "top -b -n 1 | grep test_suite"
```

## Performance Measurement

### 1. Buffer Sharing Performance
```bash
# Measure DMA-BUF sharing latency
sdb shell "/usr/bin/test_suite --test performance --test_case dma_buf_sharing"

# Measure GEM buffer sharing latency
sdb shell "/usr/bin/test_suite --test performance --test_case gem_sharing"
```

### 2. Format Conversion Performance
```bash
# Measure RGB to YUV conversion time
sdb shell "/usr/bin/test_suite --test performance --test_case rgb_to_yuv"

# Measure YUV to RGB conversion time
sdb shell "/usr/bin/test_suite --test performance --test_case yuv_to_rgb"
```

### 3. Plane Configuration Performance
```bash
# Measure plane configuration time
sdb shell "/usr/bin/test_suite --test performance --test_case plane_config"
```

## Best Practices

1. **Test Environment**
   - Run tests in a clean environment
   - Disable unnecessary services
   - Use dedicated test partitions

2. **Performance Testing**
   - Run multiple iterations
   - Measure average performance
   - Consider thermal effects

3. **Memory Management**
   - Check for memory leaks
   - Verify buffer allocation
   - Monitor memory usage

## Security Considerations

1. **DRM Permissions**
   - Verify DRM device permissions
   - Check for buffer access rights
   - Validate DMA-BUF sharing permissions

2. **Data Protection**
   - Ensure secure buffer sharing
   - Validate format conversions
   - Check for buffer overflows

## Known Limitations

1. **Target Compatibility**
   - Some tests may require specific DRM driver versions
   - Performance metrics may vary across devices
   - Buffer formats may differ between targets

2. **Resource Constraints**
   - Memory limitations on target devices
   - CPU/GPU performance variations
   - Thermal management considerations

## Support and Feedback

For issues and feedback:
- GitHub Issues: https://github.com/tizen/tizen-drm-test-suite/issues
- Tizen Forums: https://forums.tizen.org
- Email: tizen-dev@lists.tizen.org

## License

This guide is licensed under the Apache-2.0 license.
