# Tizen Vendor Test Suite

## Project Overview

The Tizen Vendor Test Suite is a comprehensive C-based framework designed to validate hardware drivers and subsystems on Tizen-based platforms. It provides a unified testing approach for multiple driver subsystems, including DRM (Direct Rendering Manager), Audio (ALSA), Video (V4L2), and USB. The framework is designed to be extensible, allowing for easy addition of new test cases and support for different platforms and targets.

**Key Features:**
- **Modular Architecture**: Independent test modules for DRM, Audio, Video, and USB
- **Cross-Platform**: Supports Linux x86, Tizen 8.0, and Tizen 9.0
- **Comprehensive Testing**:
  - DRM: KMS, buffer management, and mode setting
  - Audio: Playback, capture, and device enumeration
  - Video: Capture, format support, and encoding
  - USB: Mass storage, HID, audio, and wireless devices
- **Flexible Build System**: Target-specific configurations and selective subsystem building
- **Detailed Reporting**: Multiple output formats (HTML, JSON, XML, CSV)
- **Performance Metrics**: Frame rates, latency measurements, and throughput analysis
- **Extensible**: Easy to add new test cases and device support

## Requirements

### Software Prerequisites

#### Development Environment
- GCC compiler (version 4.8 or higher)
- GNU Make (version 3.81 or higher)
- pkg-config (version 0.26 or higher)
- CMake (version 3.10 or higher)
- Git

#### Base Dependencies
- libc6-dev (development files for C standard library)
- libdrm-dev (DRM library development files)
- udev (device manager)
- Python 3 (for test scripts)

#### Subsystem-specific Dependencies

**DRM Subsystem:**
- libdrm-dev (>= 2.4.100)
- libxf86drm-dev (>= 1.0.0)
- libdrm-tests (for reference tests)

**Audio Subsystem:**
- libasound2-dev (>= 1.1.8) - ALSA development files
- pulseaudio-utils (for audio device control)

**Video Subsystem:**
- libv4l-dev (>= 1.16.0) - Video4Linux development files
- v4l-utils (>= 1.16.0) - Video4Linux utilities

**USB Subsystem:**
- libusb-1.0-0-dev (>= 1.0.0) - USB library development files
- libscsi-dev - SCSI command support
- sg3-utils - SCSI generic utilities
- usbutils - USB device utilities

### Hardware Requirements

**DRM Testing:**
- GPU with DRM/KMS support
- Multi-monitor setup (for extended display testing)
- Hardware cursor support (recommended)

**Audio Testing:**
- Sound card compatible with ALSA
- Audio input/output devices
- Loopback device (for capture/playback testing)

**Video Testing:**
- V4L2-compatible camera
- Supported formats: MJPEG, H.264, YUYV, etc.
- Minimum resolution: 640x480

**USB Testing:**
- USB 2.0/3.0 ports
- Various USB devices for testing (storage, HID, audio, etc.)
- USB hubs (for stress testing)

## System Architecture

The Tizen Vendor Test Suite follows a modular architecture designed for flexibility and extensibility. The system is organized into the following key components:

### Core Components

1. **Test Framework Core**
   - Command-line interface with intuitive argument parsing
   - Plugin-based test discovery and execution
   - Configuration management with support for profiles
   - Resource allocation and cleanup handling
   - Signal handling and timeout management

2. **Subsystem Modules**
   - **DRM Module**:
     - KMS/atomic mode setting validation
     - Buffer management and sharing
     - CRTC/Plane/Connector testing
     - Frame buffer and format verification
   - **Audio Module**:
     - Playback and capture testing
     - Format and sample rate validation
     - Channel and period size verification
     - Device enumeration and selection
   - **Video Module**:
     - Capture device validation
     - Format and resolution testing
     - Streaming performance analysis
     - Control interface verification
   - **USB Module**:
     - Device enumeration and identification
     - Class-specific testing (HID, Mass Storage, Audio)
     - Transfer speed and reliability testing
     - Power management validation

3. **Reporting Engine**
   - Structured test result collection
   - Performance metrics gathering (latency, throughput, FPS)
   - Multi-format report generation (HTML, JSON, XML, CSV, JUnit)
   - Result comparison and trend analysis
   - Log aggregation and filtering

4. **Build System**
   - Cross-platform compilation support
   - Dependency resolution and checking
   - Target-specific toolchain configuration
   - Installer package generation

### Architecture Diagram

```
+----------------------------------+
|       Command Line Interface     |
|  (Argument Parsing & Execution)  |
+----------------------------------+
                 |
                 v
+----------------------------------+
|       Test Framework Core        |
|----------------------------------|
| - Test Discovery & Scheduling    |
| - Resource Management           |
| - Result Aggregation            |
| - Error Handling                |
+----------------------------------+
          /        |        \
         /         |         \
        v          v          v
+-----------+ +-----------+ +-----------+
|  DRM     | |  Audio    | |  Video    |
|  Module   | |  Module   | |  Module   |
+-----------+ +-----------+ +-----------+
        \         |         /
         \        |        /
          v       v       v
+----------------------------------+
|        Reporting Engine          |
+----------------------------------+
                 |
                 v
+----------------------------------+
|         Report Formats           |
| (TEXT, HTML, JSON, XML, CSV)     |
+----------------------------------+
```

## Design Details

### Subsystem Design

#### DRM Subsystem

The DRM subsystem is designed to test the Direct Rendering Manager functionality for graphics and display. It includes tests for:

- Buffer sharing and management
- Format conversion capabilities
- Plane configuration (primary, overlay, cursor)
- CRTC configuration and properties
- Connector validation
- Mode setting operations
- VBLANK handling and sync primitives

Key data structures include:
- `drm_buffer_t`: Represents a DRM buffer with properties
- `drm_plane_t`: Represents a display plane
- `drm_crtc_t`: Represents a CRTC (display controller)
- `drm_connector_t`: Represents a display connector

#### Audio Subsystem

The Audio subsystem tests ALSA functionality for sound input/output operations. It includes tests for:

- Audio playback capabilities
- Audio capture functionality
- Format support (sampling rates, bit depths)
- Latency measurement
- Volume control and muting
- Channel routing
- Compression support

Key data structures include:
- `audio_buffer_t`: Represents an audio data buffer
- `audio_device_info_t`: Contains device capabilities
- `audio_test_config_t`: Configuration for audio tests

#### Video Subsystem

The Video subsystem validates V4L2 functionality for camera and video capture. It includes tests for:

- Video capture capabilities
- Format support (RGB, YUV, compressed formats)
- Resolution and framerate support
- Encoding/decoding performance
- Format conversion and scaling
- Streaming functionality

Key data structures include:
- `video_buffer_t`: Represents a video frame buffer
- `video_device_info_t`: Contains device capabilities
- `video_test_config_t`: Configuration for video tests

#### USB Subsystem

The USB subsystem tests various USB device classes. It includes tests for:

- Mass Storage devices (0x08)
- HID devices (0x03)
- Audio devices (0x01)
- Wireless devices (0xE0)
- Vendor-specific devices (0xFF)

Key data structures include:
- `usb_test_config_t`: Configuration for USB tests
  - `run_mass_storage_tests`: Enable/disable mass storage tests
  - `run_hid_tests`: Enable/disable HID device tests
  - `run_audio_tests`: Enable/disable USB audio tests
  - `run_wireless_tests`: Enable/disable wireless device tests
  - `test_device_path`: Path to USB device
  - `vendor_id`: USB vendor ID (0 for any)
  - `product_id`: USB product ID (0 for any)

### Reporting System

The reporting system provides comprehensive test result documentation. Features include:

- Multiple output formats (TEXT, HTML, JSON, XML, CSV)
- Test result tracking with pass/fail status
- Performance metrics collection
- Timestamp and duration recording
- System information inclusion
- Summary generation

Key data structures include:
- `test_report_t`: Main report container
- `test_result_entry_t`: Individual test result
- `perf_metric_entry_t`: Performance measurement

## Project Structure

```
c_vendor_test_suite/
├── .github/                    # GitHub Actions workflows
│   └── workflows/
│       ├── build.yml          # CI build workflow
│       └── test.yml           # Test execution workflow
│
├── cmake/                     # CMake modules
│   ├── FindDRM.cmake         # DRM library detection
│   ├── FindALSA.cmake        # ALSA library detection
│   └── FindV4L2.cmake        # V4L2 library detection
│
├── docs/                     # Documentation
│   ├── architecture.md       # Architecture details
│   ├── development.md        # Development guide
│   └── testing.md            # Testing methodology
│
├── include/                  # Public headers
│   ├── tizen_drm_test.h      # DRM subsystem API
│   ├── audio/                # Audio subsystem headers
│   │   ├── tizen_audio_test.h
│   │   └── audio_test_utils.h
│   ├── video/                # Video subsystem headers
│   │   ├── tizen_video_test.h
│   │   └── video_test_utils.h
│   ├── usb/                  # USB subsystem headers
│   │   ├── tizen_usb_test.h
│   │   └── usb_test_utils.h
│   └── report/               # Reporting system
│       └── test_report.h
│
├── src/                     # Source files
│   ├── main/                 # Main application
│   │   └── test_main.c       # Entry point
│   ├── audio/                # Audio implementation
│   │   ├── tizen_audio_test.c
│   │   └── audio_tests/
│   ├── video/                # Video implementation
│   │   ├── tizen_video_test.c
│   │   └── video_tests/
│   ├── usb/                  # USB implementation
│   │   ├── tizen_usb_test.c
│   │   └── usb_tests/
│   └── report/               # Reporting implementation
│       └── test_report.c
│
├── tests/                   # Test cases
│   ├── unit/                # Unit tests
│   │   ├── test_audio.c
│   │   ├── test_video.c
│   │   └── test_usb.c
│   └── integration/         # Integration tests
│       ├── test_drm.sh
│       └── test_usb.sh
│
├── scripts/                 # Utility scripts
│   ├── build.sh             # Build script
│   ├── run_tests.sh         # Test runner
│   └── generate_report.py   # Report generator
│
├── .clang-format           # Code style configuration
├── .gitignore               # Git ignore rules
├── CMakeLists.txt           # Main CMake file
├── Makefile                # Main Makefile
├── README.md               # This documentation
└── VERSION                 # Version information
```

## Building the Test Suite

### Prerequisites

1. **Install Build Dependencies**

   ```bash
   # Base dependencies
   sudo apt-get update
   sudo apt-get install -y build-essential cmake git pkg-config
   
   # DRM dependencies
   sudo apt-get install -y libdrm-dev libdrm-tests libkmod-dev
   
   # Audio dependencies
   sudo apt-get install -y libasound2-dev pulseaudio-utils
   
   # Video dependencies
   sudo apt-get install -y libv4l-dev v4l-utils
   
   # USB dependencies
   sudo apt-get install -y libusb-1.0-0-dev libusb-1.0-0 usbutils
   sudo apt-get install -y libscsi-dev sg3-utils
   
   # Python (for test scripts)
   sudo apt-get install -y python3 python3-pip
   ```

2. **Clone the Repository**

   ```bash
   git clone https://github.com/your-org/tizen-vendor-test-suite.git
   cd tizen-vendor-test-suite
   ```

### Build Options

| Option | Description | Default |
|--------|-------------|---------|
| `BUILD_DRM` | Build DRM tests | ON |
| `BUILD_AUDIO` | Build Audio tests | ON |
| `BUILD_VIDEO` | Build Video tests | ON |
| `BUILD_USB` | Build USB tests | ON |
| `BUILD_TESTS` | Build test cases | ON |
| `BUILD_DOCS` | Build documentation | OFF |
| `ENABLE_COVERAGE` | Enable code coverage | OFF |
| `ENABLE_ASAN` | Enable Address Sanitizer | OFF |
| `ENABLE_UBSAN` | Enable Undefined Behavior Sanitizer | OFF |

### Building with CMake

```bash
# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DBUILD_DRM=ON \
         -DBUILD_AUDIO=ON \
         -DBUILD_VIDEO=ON \
         -DBUILD_USB=ON

# Build the project
make -j$(nproc)

# Install (optional)
sudo make install
```

### Building with Make

```bash
# Build all components
make all

# Build specific components
make drm audio video usb

# Install (optional)
sudo make install

# Clean build artifacts
make clean
```

### Cross-Compiling for Tizen

```bash
# For Tizen 8.0
make tizen8

# For Tizen 9.0
make tizen9

# For specific architecture
make ARCH=arm64
```

### Build Artifacts

| File | Description |
|------|-------------|
| `bin/tizen-vendor-test` | Main test executable |
| `lib/libtizen_vendor_test.so` | Shared library |
| `include/tizen_*.h` | Public headers |
| `share/tizen-vendor-test/` | Resource files |

### Packaging

To create a distribution package:
```bash
make dist
```

## Installation

1. Install build dependencies:
   ```bash
   sudo apt-get update
   sudo apt-get install build-essential pkg-config \
       libdrm-dev libasound2-dev libv4l-dev \
       libusb-1.0-0-dev libscsi-dev sg3-utils udev
   ```

2. Clone the repository:
   ```bash
   git clone https://github.com/your-username/tizen-vendor-test-suite.git
   cd tizen-vendor-test-suite
   ```

3. Build the project:
   ```bash
   make
   ```

4. Install (optional):
   ```bash
   sudo make install
   ```

5. Add your user to required groups (to avoid needing sudo):
   ```bash
   sudo usermod -aG video,audio,disk,plugdev $USER
   # Log out and back in for changes to take effect
   ```

### Building for Specific Targets

For Linux x86:
```bash
make linux
```

For Tizen 8.0:
```bash
make tizen8
```

For Tizen 9.0:
```bash
make tizen9
```

## Running Tests

### Basic Usage

Run all available tests:

```bash
# Run all tests
./bin/tizen-vendor-test

# Run with verbose output
./bin/tizen-vendor-test -v

# List available tests
./bin/tizen-vendor-test --list-tests
```

### Subsystem Testing

#### DRM Testing

```bash
# Run all DRM tests
./bin/tizen-vendor-test --drm

# Test specific display connector
./bin/tizen-vendor-test --drm --connector=HDMI-A-1

# Test specific resolution
./bin/tizen-vendor-test --drm --width=1920 --height=1080
```

#### Audio Testing

```bash
# Run all audio tests
./bin/tizen-vendor-test --audio

# Test specific audio device
./bin/tizen-vendor-test --audio --device=hw:0

# Test specific sample rate
./bin/tizen-vendor-test --audio --rate=48000
```

#### Video Testing

```bash
# Run all video tests
./bin/tizen-vendor-test --video

# Test specific video device
./bin/tizen-vendor-test --video --device=/dev/video0

# Test specific format
./bin/tizen-vendor-test --video --format=YUYV
```

#### USB Testing

```bash
# Run all USB tests
./bin/tizen-vendor-test --usb

# Test specific USB device
./bin/tizen-vendor-test --usb --vid=0xVVVV --pid=0xPPPP

# Test specific USB class
./bin/tizen-vendor-test --usb --class=mass_storage
```

### Test Selection

```bash
# Run specific test by name
./bin/tizen-vendor-test --gtest_filter=DRMTest.*

# Run tests matching a pattern
./bin/tizen-vendor-test --gtest_filter=*Buffer*

# Run tests with specific tag
./bin/tizen-vendor-test --gtest_filter=*:performance_*
```

### Output Options

```bash
# Generate JUnit XML report
./bin/tizen-vendor-test --gtest_output="xml:report.xml"

# Generate HTML report
./bin/tizen-vendor-test --report-format=html --report-file=report.html

# Set log level
./bin/tizen-vendor-test --log-level=debug
```

### Running in CI/CD

Example GitHub Actions workflow:

```yaml
name: Run Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    - name: Install Dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y build-essential cmake libdrm-dev libasound2-dev libv4l-dev libusb-1.0-0-dev
    - name: Build
      run: |
        mkdir -p build
        cd build
        cmake ..
        make -j$(nproc)
    - name: Run Tests
      run: |
        cd build
        ctest --output-on-failure
```

### USB Testing

#### Device Discovery
List all connected USB devices:
```bash
lsusb
```

#### Testing Specific USB Device
Test a specific USB device by vendor and product ID:
```bash
./test_suite -s usb --usb-vendor-id 0xVVVV --usb-product-id 0xPPPP
```

Test a specific USB device by device path:
```bash
./test_suite -s usb --usb-device-path /dev/sdX
```

#### Test Categories
Run specific USB test categories:
```bash
# Test only mass storage functionality
./test_suite -s usb --usb-test mass_storage

# Test only HID devices (keyboards, mice, etc.)
./test_suite -s usb --usb-test hid

# Test only USB audio devices
./test_suite -s usb --usb-test audio

# Test only wireless devices (Bluetooth, WiFi)
./test_suite -s usb --usb-test wireless
```

#### Verbose Output
For detailed debugging information:
```bash
./test_suite -s usb -v
```

### Test Selection

To run specific tests within a subsystem:
```bash
# Test buffer sharing in DRM subsystem
./test_suite --subsystem=drm --test=buffer_sharing

# Test audio playback
./test_suite --subsystem=audio --test=playback

# Test video capture
./test_suite --subsystem=video --test=capture

# Test USB mass storage
./test_suite --subsystem=usb --test=mass_storage
```

### Device Selection

To specify which device to test:
```bash
# Test device index 1
./test_suite --device=1
```

### Configuration Options

To set test parameters:
```bash
# Set custom resolution
./test_suite --width=1920 --height=1080

# Set audio sample rate
./test_suite --rate=48000

# Set number of iterations
./test_suite --iterations=10
```

### Verbose Output

For detailed output during test execution:
```bash
./test_suite --verbose
```

## Test Report Generation

### Report Formats

The test suite can generate reports in multiple formats:

```bash
# Generate HTML report
./test_suite --report-format=html --report-file=report.html

# Generate JSON report
./test_suite --report-format=json --report-file=report.json

# Generate XML report
./test_suite --report-format=xml --report-file=report.xml

# Generate CSV report
./test_suite --report-format=csv --report-file=report.csv
```

### Report Examples

#### HTML Report Structure

HTML reports include:
- Test summary with pass/fail statistics
- Color-coded test results
- Performance metrics section
- System information
- Timestamp information

#### Summary Reports

Along with the main report, a summary file is also generated that provides a quick overview of test results.

### Automated Reporting

To incorporate test reporting into CI/CD pipelines:

```bash
# Example CI pipeline command
./test_suite --subsystem=all --report-format=html --report-file=ci_report.html
```

## Troubleshooting

### Common Issues

1. **Missing Dependencies**
   - Error: `error while loading shared libraries: libdrm.so.2` or `libusb-1.0.so.0`
   - Solution: Install the required runtime libraries:
     ```bash
     sudo apt-get install libdrm2 libasound2 libv4l-0 libusb-1.0-0
     ```

2. **Permission Denied**
   - Error: `Failed to open /dev/dri/card0` or `/dev/sdX: Permission denied`
   - Solution: Add your user to the appropriate groups:
     ```bash
     sudo usermod -aG video,audio,disk,plugdev $USER
     # Log out and back in for changes to take effect
     ```

3. **USB Device Not Found**
   - Error: `No matching USB device found`
   - Check: 
     ```bash
     # List USB devices
     lsusb
     # Check kernel messages
     dmesg | tail -20
     # Check device nodes
     ls -l /dev/sd*
     ```
   - Ensure the device is properly connected and powered

4. **SCSI Command Failures**
   - Error: `SCSI_IOCTL_SEND_COMMAND failed`
   - Solution: Load the SCSI generic module:
     ```bash
     sudo modprobe sg
     ```
   - For persistent loading, add `sg` to `/etc/modules`

5. **Debugging USB Issues**
   - Enable USB debugging in the kernel:
     ```bash
     # Check current debug level
     cat /sys/module/usbcore/parameters/level
     # Enable debugging (temporary)
     echo 2 | sudo tee /sys/module/usbcore/parameters/level
     # View kernel messages
     dmesg -w
     ```

### Debug Mode

For debugging issues:
```bash
# Build with debug symbols
CFLAGS="-g -O0" make

# Run with verbose output
./test_suite --verbose
```

## Contributing Guidelines

### Code Style

- Follow the existing code style (Linux kernel coding style)
- Use 4-space indentation for C code
- Use descriptive function and variable names
- Add comments for complex logic

### Adding New Tests

1. Create test implementation in the appropriate subsystem directory
2. Register the test in the subsystem's header file
3. Add the test to the corresponding test execution function in test_main.c
4. Update documentation to reflect the new test

### Adding New Subsystems

1. Create a new directory structure for the subsystem
2. Implement the subsystem API in header files
3. Implement the subsystem implementation
4. Update the Makefile to include the new subsystem
5. Add subsystem selection in test_main.c

## License Information

The Tizen Vendor Test Suite is licensed under the GNU Lesser General Public License (LGPL) version 2.1 or later. This means you can use, modify, and distribute the software, even in commercial applications, as long as you comply with the license terms.

```
Copyright (C) 2025 Sumit Panwar <sumit.panwar@example.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
```

## Contact Information

For questions, bug reports, or feature requests, please contact:

- **Maintainer**: Sumit Panwar
- **Email**: sumit.panwar@example.com
- **Project URL**: https://github.com/sumitpanwar/tizen-vendor-test-suite
