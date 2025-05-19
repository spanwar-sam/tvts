# Tizen Vendor Test Suite

## Project Overview

The Tizen Vendor Test Suite is a comprehensive C-based framework designed to validate hardware drivers and subsystems on Tizen-based platforms. It provides a unified testing approach for multiple driver subsystems, including DRM (Direct Rendering Manager), Audio (ALSA), and Video (V4L2). The framework is designed to be extensible, allowing for easy addition of new test cases and support for different platforms and targets.

**Key Features:**
- Multi-subsystem testing (DRM, Audio, Video)
- Cross-platform support (Linux x86, Tizen 8.0, Tizen 9.0)
- Flexible build system with target-specific configurations
- Automated test reporting in multiple formats
- Performance metrics collection and analysis
- Comprehensive error handling and logging

## Requirements

### Software Prerequisites

#### Development Environment
- GCC compiler (version 4.8 or higher)
- GNU Make (version 3.81 or higher)
- pkg-config (version 0.26 or higher)

#### Base Dependencies
- libc6-dev (development files for C standard library)
- libdrm-dev (DRM library development files)

#### Subsystem-specific Dependencies

**DRM Subsystem:**
- libdrm-dev (>= 2.4.100)
- libxf86drm-dev (>= 1.0.0)

**Audio Subsystem:**
- libasound2-dev (>= 1.1.8) - ALSA development files

**Video Subsystem:**
- libv4l-dev (>= 1.16.0) - Video4Linux development files

### Hardware Requirements

- For DRM testing: GPU with DRM/KMS support
- For Audio testing: Sound card compatible with ALSA
- For Video testing: Video capture device compatible with V4L2

## System Architecture

The Tizen Vendor Test Suite follows a modular architecture designed for flexibility and extensibility. The system is organized into the following key components:

### Core Components

1. **Test Framework Core**
   - Command-line interface
   - Test discovery and execution
   - Configuration management
   - Resource allocation and cleanup

2. **Subsystem Modules**
   - DRM module for graphics/display testing
   - Audio module for sound subsystem testing
   - Video module for camera/video capture testing

3. **Reporting Engine**
   - Test result collection
   - Performance metrics gathering
   - Report generation in multiple formats (TEXT, HTML, JSON, XML, CSV)

4. **Build System**
   - Platform-specific compilation
   - Configurable target architecture support
   - Dependency management

### Architecture Diagram

```
+----------------------------------+
|          Command Line            |
|          Interface (CLI)         |
+----------------------------------+
                 |
                 v
+----------------------------------+
|       Test Framework Core        |
+----------------------------------+
          /        |        \
         /         |         \
        v          v          v
+-----------+ +-----------+ +-----------+
|    DRM    | |   Audio   | |   Video   |
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

## Project Directory Structure

```
c_vendor_test_suite/
├── include/
│   ├── tizen_drm_test.h         # DRM subsystem header
│   ├── audio/
│   │   └── tizen_audio_test.h   # Audio subsystem header
│   ├── video/
│   │   └── tizen_video_test.h   # Video subsystem header
│   └── report/
│       └── test_report.h        # Reporting system header
├── src/
│   ├── test_main.c              # Main entry point
│   ├── audio/
│   │   └── tizen_audio_test.c   # Audio implementation
│   ├── video/
│   │   └── tizen_video_test.c   # Video implementation
│   └── report/
│       └── test_report.c        # Reporting implementation
├── scripts/
│   └── build_and_test.sh        # Build automation script
├── Makefile                     # Build system
├── tizen_drm_test_suite.spec.in # Spec file template
└── README.md                    # This documentation
```

## Build Instructions

### Basic Build

To build the Tizen Vendor Test Suite with default settings (Linux target with all subsystems):

```bash
make
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

### Building Specific Subsystems

For DRM subsystem only:
```bash
make drm
```

For Audio subsystem only:
```bash
make audio
```

For Video subsystem only:
```bash
make video
```

### Combined Target and Subsystem Build

For Tizen 8.0 with DRM only:
```bash
make tizen8-drm
```

For Tizen 9.0 with Audio only:
```bash
make tizen9-audio
```

### Cleaning Build

To clean build artifacts:
```bash
make clean
```

### Packaging

To create a distribution package:
```bash
make dist
```

## Execution Guide

### Basic Usage

To run all tests with default settings:
```bash
./test_suite
```

### Subsystem Selection

To test specific subsystems:
```bash
# Test DRM subsystem only
./test_suite --subsystem=drm

# Test Audio subsystem only
./test_suite --subsystem=audio

# Test Video subsystem only
./test_suite --subsystem=video
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

1. **Missing dependencies**
   - Error: `error while loading shared libraries: libdrm.so.2`
   - Solution: Install libdrm with `apt-get install libdrm2 libdrm-dev`

2. **Device access permission issues**
   - Error: `Permission denied when opening /dev/dri/card0`
   - Solution: Run with appropriate permissions (sudo) or add user to video group

3. **Build failures**
   - Check that all dependencies are installed
   - Verify target-specific configuration

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
