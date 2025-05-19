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

#ifndef TIZEN_AUDIO_TEST_H
#define TIZEN_AUDIO_TEST_H

#include <stdbool.h>
#include <stdint.h>

// Audio formats
typedef enum {
    AUDIO_FORMAT_PCM_S8,      // Signed 8-bit PCM
    AUDIO_FORMAT_PCM_U8,      // Unsigned 8-bit PCM
    AUDIO_FORMAT_PCM_S16LE,   // Signed 16-bit PCM, little-endian
    AUDIO_FORMAT_PCM_S16BE,   // Signed 16-bit PCM, big-endian
    AUDIO_FORMAT_PCM_S24LE,   // Signed 24-bit PCM, little-endian
    AUDIO_FORMAT_PCM_S24BE,   // Signed 24-bit PCM, big-endian
    AUDIO_FORMAT_PCM_S32LE,   // Signed 32-bit PCM, little-endian
    AUDIO_FORMAT_PCM_S32BE,   // Signed 32-bit PCM, big-endian
    AUDIO_FORMAT_MP3,         // MP3 format
    AUDIO_FORMAT_AAC,         // AAC format
    AUDIO_FORMAT_FLAC,        // FLAC format
    AUDIO_FORMAT_MAX
} audio_format_t;

// Audio channel configurations
typedef enum {
    AUDIO_CHANNEL_MONO,       // Mono
    AUDIO_CHANNEL_STEREO,     // Stereo
    AUDIO_CHANNEL_2_1,        // 2.1
    AUDIO_CHANNEL_5_1,        // 5.1 surround
    AUDIO_CHANNEL_7_1,        // 7.1 surround
    AUDIO_CHANNEL_MAX
} audio_channel_t;

// Audio device types
typedef enum {
    AUDIO_DEVICE_PLAYBACK,    // Playback device
    AUDIO_DEVICE_CAPTURE,     // Capture device
    AUDIO_DEVICE_BOTH,        // Both playback and capture
    AUDIO_DEVICE_MAX
} audio_device_type_t;

// Audio feature types for testing
typedef enum {
    AUDIO_FEATURE_PLAYBACK,        // Basic playback
    AUDIO_FEATURE_CAPTURE,         // Basic capture
    AUDIO_FEATURE_FORMAT_SUPPORT,  // Format support
    AUDIO_FEATURE_LATENCY,         // Latency measurement
    AUDIO_FEATURE_VOLUME,          // Volume control
    AUDIO_FEATURE_MUTE,            // Mute control
    AUDIO_FEATURE_ROUTING,         // Audio routing
    AUDIO_FEATURE_COMPRESSION,     // Compression support
    AUDIO_FEATURE_RESAMPLING,      // Resampling support
    AUDIO_FEATURE_SYNC,            // Synchronization
    AUDIO_FEATURE_INTERFERENCE,    // Interference testing
    AUDIO_FEATURE_MAX
} audio_feature_t;

// Audio device info
typedef struct {
    char name[128];               // Device name
    audio_device_type_t type;     // Device type
    uint32_t sample_rates[16];    // Supported sample rates
    uint32_t sample_rate_count;   // Number of supported sample rates
    audio_format_t formats[16];   // Supported formats
    uint32_t format_count;        // Number of supported formats
    audio_channel_t channels[8];  // Supported channel configurations
    uint32_t channel_count;       // Number of supported channel configurations
    uint32_t min_buffer_size;     // Minimum buffer size
    uint32_t max_buffer_size;     // Maximum buffer size
} audio_device_info_t;

// Audio buffer structure
typedef struct {
    void *data;                   // Buffer data
    uint32_t size;                // Buffer size in bytes
    audio_format_t format;        // Buffer format
    uint32_t sample_rate;         // Sample rate
    audio_channel_t channels;     // Channel configuration
    uint32_t frame_count;         // Number of frames
} audio_buffer_t;

// Test configuration
typedef struct {
    uint32_t sample_rate;         // Sample rate for testing
    audio_format_t format;        // Format for testing
    audio_channel_t channels;     // Channel configuration for testing
    uint32_t buffer_size;         // Buffer size for testing
    uint32_t iterations;          // Number of test iterations
    uint32_t timeout;             // Test timeout in milliseconds
} audio_test_config_t;

// Test result
typedef enum {
    AUDIO_TEST_PASS,
    AUDIO_TEST_FAIL,
    AUDIO_TEST_SKIP,
    AUDIO_TEST_ERROR
} audio_test_result_t;

// Function prototypes

// Framework initialization/cleanup
bool init_audio_test_framework(void);
void cleanup_audio_test_framework(void);

// Device enumeration and information
uint32_t get_audio_device_count(audio_device_type_t type);
bool get_audio_device_info(uint32_t device_index, audio_device_info_t *info);

// Buffer management
audio_buffer_t *create_audio_buffer(const audio_test_config_t *config);
bool fill_audio_buffer(audio_buffer_t *buffer, uint32_t pattern);
bool verify_audio_buffer(audio_buffer_t *buffer, uint32_t pattern);
void destroy_audio_buffer(audio_buffer_t *buffer);

// Feature testing
bool test_audio_playback(uint32_t device_index, const audio_test_config_t *config);
bool test_audio_capture(uint32_t device_index, const audio_test_config_t *config);
bool test_audio_format_support(uint32_t device_index, const audio_test_config_t *config);
bool test_audio_latency(uint32_t device_index, const audio_test_config_t *config, uint32_t *latency_ms);
bool test_audio_volume(uint32_t device_index, const audio_test_config_t *config);
bool test_audio_mute(uint32_t device_index, const audio_test_config_t *config);
bool test_audio_routing(uint32_t device_index, const audio_test_config_t *config);
bool test_audio_compression(uint32_t device_index, const audio_test_config_t *config);
bool test_audio_resampling(uint32_t device_index, const audio_test_config_t *config);
bool test_audio_sync(uint32_t device_index, const audio_test_config_t *config);
bool test_audio_interference(uint32_t device_index, const audio_test_config_t *config);

// Comprehensive testing
bool test_all_audio_features(uint32_t device_index, const audio_test_config_t *config);

// Utilities
const char *audio_format_to_string(audio_format_t format);
const char *audio_channel_to_string(audio_channel_t channels);
const char *audio_device_type_to_string(audio_device_type_t type);
const char *audio_feature_to_string(audio_feature_t feature);
audio_test_result_t convert_bool_to_test_result(bool result);

#endif /* TIZEN_AUDIO_TEST_H */
