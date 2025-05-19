/*
 * Copyright (C) 2025 Sumit Panwar <sumit.panwar@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef TIZEN_VIDEO_TEST_H
#define TIZEN_VIDEO_TEST_H

#include <stdbool.h>
#include <stdint.h>

// Video formats
typedef enum {
    VIDEO_FORMAT_RGB565,      // RGB565 (16-bit)
    VIDEO_FORMAT_RGB888,      // RGB888 (24-bit)
    VIDEO_FORMAT_RGBA8888,    // RGBA8888 (32-bit)
    VIDEO_FORMAT_ARGB8888,    // ARGB8888 (32-bit)
    VIDEO_FORMAT_NV12,        // NV12 (YUV 4:2:0)
    VIDEO_FORMAT_YUV420,      // YUV420 planar
    VIDEO_FORMAT_YUV422,      // YUV422 planar
    VIDEO_FORMAT_YUYV,        // YUYV (YUV 4:2:2 interleaved)
    VIDEO_FORMAT_UYVY,        // UYVY (YUV 4:2:2 interleaved)
    VIDEO_FORMAT_MJPEG,       // Motion JPEG
    VIDEO_FORMAT_H264,        // H.264
    VIDEO_FORMAT_H265,        // H.265
    VIDEO_FORMAT_VP8,         // VP8
    VIDEO_FORMAT_VP9,         // VP9
    VIDEO_FORMAT_MAX
} video_format_t;

// Video devices
typedef enum {
    VIDEO_DEVICE_CAMERA,      // Camera device
    VIDEO_DEVICE_ENCODER,     // Video encoder
    VIDEO_DEVICE_DECODER,     // Video decoder
    VIDEO_DEVICE_CONVERTER,   // Format converter
    VIDEO_DEVICE_SCALER,      // Video scaler
    VIDEO_DEVICE_MAX
} video_device_type_t;

// Video feature types for testing
typedef enum {
    VIDEO_FEATURE_CAPTURE,          // Basic video capture
    VIDEO_FEATURE_ENCODING,         // Video encoding
    VIDEO_FEATURE_DECODING,         // Video decoding
    VIDEO_FEATURE_FORMAT_SUPPORT,   // Format support
    VIDEO_FEATURE_RESOLUTION,       // Resolution support
    VIDEO_FEATURE_FRAMERATES,       // Framerate support
    VIDEO_FEATURE_CONVERSION,       // Format conversion
    VIDEO_FEATURE_SCALING,          // Video scaling
    VIDEO_FEATURE_ROTATION,         // Video rotation
    VIDEO_FEATURE_COMPRESSION,      // Compression support
    VIDEO_FEATURE_STREAMING,        // Streaming support
    VIDEO_FEATURE_SYNC,             // Synchronization
    VIDEO_FEATURE_MAX
} video_feature_t;

// Video buffer structure
typedef struct {
    void *data;               // Buffer data
    uint32_t size;            // Buffer size in bytes
    video_format_t format;    // Buffer format
    uint32_t width;           // Width in pixels
    uint32_t height;          // Height in pixels
    uint32_t stride;          // Stride in bytes
    uint32_t framerate;       // Framerate (fps)
    uint64_t timestamp;       // Timestamp in microseconds
} video_buffer_t;

// Video device info
typedef struct {
    char name[128];                // Device name
    video_device_type_t type;      // Device type
    video_format_t formats[16];    // Supported formats
    uint32_t format_count;         // Number of supported formats
    uint32_t min_width;            // Minimum width
    uint32_t max_width;            // Maximum width
    uint32_t min_height;           // Minimum height
    uint32_t max_height;           // Maximum height
    uint32_t framerates[16];       // Supported framerates
    uint32_t framerate_count;      // Number of supported framerates
    uint32_t min_bitrate;          // Minimum bitrate
    uint32_t max_bitrate;          // Maximum bitrate
} video_device_info_t;

// Test configuration
typedef struct {
    uint32_t width;                // Width for testing
    uint32_t height;               // Height for testing
    video_format_t format;         // Format for testing
    uint32_t framerate;            // Framerate for testing
    uint32_t bitrate;              // Bitrate for testing
    uint32_t duration;             // Test duration in seconds
    uint32_t iterations;           // Number of test iterations
    uint32_t timeout;              // Test timeout in milliseconds
} video_test_config_t;

// Test result
typedef enum {
    VIDEO_TEST_PASS,
    VIDEO_TEST_FAIL,
    VIDEO_TEST_SKIP,
    VIDEO_TEST_ERROR
} video_test_result_t;

// Function prototypes

// Framework initialization/cleanup
bool init_video_test_framework(void);
void cleanup_video_test_framework(void);

// Device enumeration and information
uint32_t get_video_device_count(video_device_type_t type);
bool get_video_device_info(uint32_t device_index, video_device_info_t *info);

// Buffer management
video_buffer_t *create_video_buffer(const video_test_config_t *config);
bool fill_video_buffer(video_buffer_t *buffer, uint32_t pattern);
bool verify_video_buffer(video_buffer_t *buffer, uint32_t pattern);
void destroy_video_buffer(video_buffer_t *buffer);

// Feature testing
bool test_video_capture(uint32_t device_index, const video_test_config_t *config);
bool test_video_encoding(uint32_t device_index, const video_test_config_t *config);
bool test_video_decoding(uint32_t device_index, const video_test_config_t *config);
bool test_video_format_support(uint32_t device_index, const video_test_config_t *config);
bool test_video_resolution(uint32_t device_index, const video_test_config_t *config);
bool test_video_framerates(uint32_t device_index, const video_test_config_t *config);
bool test_video_conversion(uint32_t device_index, const video_test_config_t *config, video_format_t target_format);
bool test_video_scaling(uint32_t device_index, const video_test_config_t *config, uint32_t target_width, uint32_t target_height);
bool test_video_rotation(uint32_t device_index, const video_test_config_t *config, uint32_t rotation_angle);
bool test_video_compression(uint32_t device_index, const video_test_config_t *config);
bool test_video_streaming(uint32_t device_index, const video_test_config_t *config);
bool test_video_sync(uint32_t device_index, const video_test_config_t *config);

// Performance testing
bool test_video_capture_performance(uint32_t device_index, const video_test_config_t *config, uint32_t *avg_fps);
bool test_video_encoding_performance(uint32_t device_index, const video_test_config_t *config, uint32_t *avg_fps);
bool test_video_decoding_performance(uint32_t device_index, const video_test_config_t *config, uint32_t *avg_fps);

// Comprehensive testing
bool test_all_video_features(uint32_t device_index, const video_test_config_t *config);

// Utilities
const char *video_format_to_string(video_format_t format);
const char *video_device_type_to_string(video_device_type_t type);
const char *video_feature_to_string(video_feature_t feature);
video_test_result_t convert_bool_to_test_result(bool result);

#endif /* TIZEN_VIDEO_TEST_H */
