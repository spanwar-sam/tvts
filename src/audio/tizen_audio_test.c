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

#include "audio/tizen_audio_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <alsa/asoundlib.h>

#define TEST_TIMEOUT 5000 // 5 seconds

static snd_pcm_t *playback_handle = NULL;
static snd_pcm_t *capture_handle = NULL;
static audio_device_info_t *devices = NULL;
static uint32_t device_count = 0;

// Static helper functions
static bool open_pcm_device(uint32_t device_index, snd_pcm_stream_t stream, snd_pcm_t **handle) {
    char device_name[64];
    snprintf(device_name, sizeof(device_name), "hw:%d,0", device_index);
    
    int err = snd_pcm_open(handle, device_name, stream, 0);
    if (err < 0) {
        fprintf(stderr, "Cannot open PCM device %s: %s\n", device_name, snd_strerror(err));
        return false;
    }
    return true;
}

static bool set_pcm_params(snd_pcm_t *handle, const audio_test_config_t *config) {
    snd_pcm_hw_params_t *params;
    
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    
    int err;
    
    // Set access type
    err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        fprintf(stderr, "Cannot set access type: %s\n", snd_strerror(err));
        return false;
    }
    
    // Set format
    snd_pcm_format_t format;
    switch (config->format) {
        case AUDIO_FORMAT_PCM_S16LE:
            format = SND_PCM_FORMAT_S16_LE;
            break;
        case AUDIO_FORMAT_PCM_S24LE:
            format = SND_PCM_FORMAT_S24_LE;
            break;
        case AUDIO_FORMAT_PCM_S32LE:
            format = SND_PCM_FORMAT_S32_LE;
            break;
        default:
            fprintf(stderr, "Unsupported format\n");
            return false;
    }
    
    err = snd_pcm_hw_params_set_format(handle, params, format);
    if (err < 0) {
        fprintf(stderr, "Cannot set format: %s\n", snd_strerror(err));
        return false;
    }
    
    // Set channels
    unsigned int channels;
    switch (config->channels) {
        case AUDIO_CHANNEL_MONO:
            channels = 1;
            break;
        case AUDIO_CHANNEL_STEREO:
            channels = 2;
            break;
        case AUDIO_CHANNEL_2_1:
            channels = 3;
            break;
        case AUDIO_CHANNEL_5_1:
            channels = 6;
            break;
        case AUDIO_CHANNEL_7_1:
            channels = 8;
            break;
        default:
            fprintf(stderr, "Unsupported channel configuration\n");
            return false;
    }
    
    err = snd_pcm_hw_params_set_channels(handle, params, channels);
    if (err < 0) {
        fprintf(stderr, "Cannot set channels: %s\n", snd_strerror(err));
        return false;
    }
    
    // Set sample rate
    unsigned int rate = config->sample_rate;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);
    if (err < 0) {
        fprintf(stderr, "Cannot set sample rate: %s\n", snd_strerror(err));
        return false;
    }
    
    // Set buffer size
    snd_pcm_uframes_t buffer_size = config->buffer_size;
    err = snd_pcm_hw_params_set_buffer_size_near(handle, params, &buffer_size);
    if (err < 0) {
        fprintf(stderr, "Cannot set buffer size: %s\n", snd_strerror(err));
        return false;
    }
    
    // Apply params
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        fprintf(stderr, "Cannot set parameters: %s\n", snd_strerror(err));
        return false;
    }
    
    return true;
}

// Framework initialization/cleanup
bool init_audio_test_framework(void) {
    // Discover available devices
    int card = -1;
    int device = -1;
    device_count = 0;
    
    // Count devices first
    while (snd_card_next(&card) >= 0 && card >= 0) {
        device_count++;
    }
    
    if (device_count == 0) {
        fprintf(stderr, "No sound cards found\n");
        return false;
    }
    
    // Allocate device info array
    devices = malloc(device_count * sizeof(audio_device_info_t));
    if (!devices) {
        fprintf(stderr, "Memory allocation failed\n");
        return false;
    }
    
    // Fill device info
    card = -1;
    int device_index = 0;
    
    while (snd_card_next(&card) >= 0 && card >= 0) {
        char name[64];
        snprintf(name, sizeof(name), "hw:%d", card);
        
        // Get device info
        snd_ctl_t *handle;
        int err = snd_ctl_open(&handle, name, 0);
        if (err < 0) {
            fprintf(stderr, "Control open error: %s\n", snd_strerror(err));
            continue;
        }
        
        // Get device name
        snd_ctl_card_info_t *info;
        snd_ctl_card_info_alloca(&info);
        err = snd_ctl_card_info(handle, info);
        if (err < 0) {
            fprintf(stderr, "Control info error: %s\n", snd_strerror(err));
            snd_ctl_close(handle);
            continue;
        }
        
        strncpy(devices[device_index].name, snd_ctl_card_info_get_name(info), sizeof(devices[device_index].name));
        
        // Check device type
        snd_pcm_t *pcm_handle;
        err = snd_pcm_open(&pcm_handle, name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
        if (err >= 0) {
            devices[device_index].type = AUDIO_DEVICE_PLAYBACK;
            snd_pcm_close(pcm_handle);
        }
        
        err = snd_pcm_open(&pcm_handle, name, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
        if (err >= 0) {
            devices[device_index].type = devices[device_index].type == AUDIO_DEVICE_PLAYBACK ? 
                                         AUDIO_DEVICE_BOTH : AUDIO_DEVICE_CAPTURE;
            snd_pcm_close(pcm_handle);
        }
        
        // Get supported sample rates and formats (simplified for brevity)
        devices[device_index].sample_rates[0] = 44100;
        devices[device_index].sample_rates[1] = 48000;
        devices[device_index].sample_rate_count = 2;
        
        devices[device_index].formats[0] = AUDIO_FORMAT_PCM_S16LE;
        devices[device_index].formats[1] = AUDIO_FORMAT_PCM_S24LE;
        devices[device_index].format_count = 2;
        
        devices[device_index].channels[0] = AUDIO_CHANNEL_MONO;
        devices[device_index].channels[1] = AUDIO_CHANNEL_STEREO;
        devices[device_index].channel_count = 2;
        
        devices[device_index].min_buffer_size = 1024;
        devices[device_index].max_buffer_size = 65536;
        
        snd_ctl_close(handle);
        device_index++;
    }
    
    return true;
}

void cleanup_audio_test_framework(void) {
    if (playback_handle) {
        snd_pcm_close(playback_handle);
        playback_handle = NULL;
    }
    
    if (capture_handle) {
        snd_pcm_close(capture_handle);
        capture_handle = NULL;
    }
    
    if (devices) {
        free(devices);
        devices = NULL;
    }
    
    device_count = 0;
}

// Device enumeration and information
uint32_t get_audio_device_count(audio_device_type_t type) {
    uint32_t count = 0;
    
    for (uint32_t i = 0; i < device_count; i++) {
        if (devices[i].type == type || 
            devices[i].type == AUDIO_DEVICE_BOTH ||
            type == AUDIO_DEVICE_BOTH) {
            count++;
        }
    }
    
    return count;
}

bool get_audio_device_info(uint32_t device_index, audio_device_info_t *info) {
    if (device_index >= device_count || !info) {
        return false;
    }
    
    memcpy(info, &devices[device_index], sizeof(audio_device_info_t));
    return true;
}

// Buffer management
audio_buffer_t *create_audio_buffer(const audio_test_config_t *config) {
    if (!config) {
        return NULL;
    }
    
    audio_buffer_t *buffer = malloc(sizeof(audio_buffer_t));
    if (!buffer) {
        return NULL;
    }
    
    // Calculate buffer size in bytes
    uint32_t channels;
    switch (config->channels) {
        case AUDIO_CHANNEL_MONO: channels = 1; break;
        case AUDIO_CHANNEL_STEREO: channels = 2; break;
        case AUDIO_CHANNEL_2_1: channels = 3; break;
        case AUDIO_CHANNEL_5_1: channels = 6; break;
        case AUDIO_CHANNEL_7_1: channels = 8; break;
        default: channels = 2; break;
    }
    
    uint32_t bytes_per_sample;
    switch (config->format) {
        case AUDIO_FORMAT_PCM_S8:
        case AUDIO_FORMAT_PCM_U8:
            bytes_per_sample = 1;
            break;
        case AUDIO_FORMAT_PCM_S16LE:
        case AUDIO_FORMAT_PCM_S16BE:
            bytes_per_sample = 2;
            break;
        case AUDIO_FORMAT_PCM_S24LE:
        case AUDIO_FORMAT_PCM_S24BE:
            bytes_per_sample = 3;
            break;
        case AUDIO_FORMAT_PCM_S32LE:
        case AUDIO_FORMAT_PCM_S32BE:
            bytes_per_sample = 4;
            break;
        default:
            bytes_per_sample = 2;
            break;
    }
    
    buffer->size = config->buffer_size * channels * bytes_per_sample;
    buffer->data = malloc(buffer->size);
    if (!buffer->data) {
        free(buffer);
        return NULL;
    }
    
    buffer->format = config->format;
    buffer->sample_rate = config->sample_rate;
    buffer->channels = config->channels;
    buffer->frame_count = config->buffer_size;
    
    return buffer;
}

bool fill_audio_buffer(audio_buffer_t *buffer, uint32_t pattern) {
    if (!buffer || !buffer->data) {
        return false;
    }
    
    // Simple pattern fill
    memset(buffer->data, pattern & 0xFF, buffer->size);
    return true;
}

bool verify_audio_buffer(audio_buffer_t *buffer, uint32_t pattern) {
    if (!buffer || !buffer->data) {
        return false;
    }
    
    // Simple pattern verification
    uint8_t *data = (uint8_t *)buffer->data;
    for (uint32_t i = 0; i < buffer->size; i++) {
        if (data[i] != (pattern & 0xFF)) {
            return false;
        }
    }
    
    return true;
}

void destroy_audio_buffer(audio_buffer_t *buffer) {
    if (buffer) {
        if (buffer->data) {
            free(buffer->data);
        }
        free(buffer);
    }
}

// Feature testing
bool test_audio_playback(uint32_t device_index, const audio_test_config_t *config) {
    if (!open_pcm_device(device_index, SND_PCM_STREAM_PLAYBACK, &playback_handle)) {
        return false;
    }
    
    if (!set_pcm_params(playback_handle, config)) {
        snd_pcm_close(playback_handle);
        playback_handle = NULL;
        return false;
    }
    
    // Create test buffer
    audio_buffer_t *buffer = create_audio_buffer(config);
    if (!buffer) {
        snd_pcm_close(playback_handle);
        playback_handle = NULL;
        return false;
    }
    
    // Fill buffer with test pattern
    if (!fill_audio_buffer(buffer, 0x55)) {
        destroy_audio_buffer(buffer);
        snd_pcm_close(playback_handle);
        playback_handle = NULL;
        return false;
    }
    
    // Play buffer
    snd_pcm_uframes_t frames = buffer->frame_count;
    int err = snd_pcm_writei(playback_handle, buffer->data, frames);
    if (err < 0) {
        fprintf(stderr, "Write error: %s\n", snd_strerror(err));
        destroy_audio_buffer(buffer);
        snd_pcm_close(playback_handle);
        playback_handle = NULL;
        return false;
    }
    
    // Wait for playback to complete
    snd_pcm_drain(playback_handle);
    
    // Cleanup
    destroy_audio_buffer(buffer);
    snd_pcm_close(playback_handle);
    playback_handle = NULL;
    
    return true;
}

// Comprehensive testing
bool test_all_audio_features(uint32_t device_index, const audio_test_config_t *config) {
    bool result = true;
    
    result &= test_audio_playback(device_index, config);
    
    // Add calls to other test functions here
    
    return result;
}

// Utilities
const char *audio_format_to_string(audio_format_t format) {
    switch (format) {
        case AUDIO_FORMAT_PCM_S8: return "PCM_S8";
        case AUDIO_FORMAT_PCM_U8: return "PCM_U8";
        case AUDIO_FORMAT_PCM_S16LE: return "PCM_S16LE";
        case AUDIO_FORMAT_PCM_S16BE: return "PCM_S16BE";
        case AUDIO_FORMAT_PCM_S24LE: return "PCM_S24LE";
        case AUDIO_FORMAT_PCM_S24BE: return "PCM_S24BE";
        case AUDIO_FORMAT_PCM_S32LE: return "PCM_S32LE";
        case AUDIO_FORMAT_PCM_S32BE: return "PCM_S32BE";
        case AUDIO_FORMAT_MP3: return "MP3";
        case AUDIO_FORMAT_AAC: return "AAC";
        case AUDIO_FORMAT_FLAC: return "FLAC";
        default: return "UNKNOWN";
    }
}

const char *audio_channel_to_string(audio_channel_t channels) {
    switch (channels) {
        case AUDIO_CHANNEL_MONO: return "MONO";
        case AUDIO_CHANNEL_STEREO: return "STEREO";
        case AUDIO_CHANNEL_2_1: return "2.1";
        case AUDIO_CHANNEL_5_1: return "5.1";
        case AUDIO_CHANNEL_7_1: return "7.1";
        default: return "UNKNOWN";
    }
}

const char *audio_device_type_to_string(audio_device_type_t type) {
    switch (type) {
        case AUDIO_DEVICE_PLAYBACK: return "PLAYBACK";
        case AUDIO_DEVICE_CAPTURE: return "CAPTURE";
        case AUDIO_DEVICE_BOTH: return "BOTH";
        default: return "UNKNOWN";
    }
}

const char *audio_feature_to_string(audio_feature_t feature) {
    switch (feature) {
        case AUDIO_FEATURE_PLAYBACK: return "PLAYBACK";
        case AUDIO_FEATURE_CAPTURE: return "CAPTURE";
        case AUDIO_FEATURE_FORMAT_SUPPORT: return "FORMAT_SUPPORT";
        case AUDIO_FEATURE_LATENCY: return "LATENCY";
        case AUDIO_FEATURE_VOLUME: return "VOLUME";
        case AUDIO_FEATURE_MUTE: return "MUTE";
        case AUDIO_FEATURE_ROUTING: return "ROUTING";
        case AUDIO_FEATURE_COMPRESSION: return "COMPRESSION";
        case AUDIO_FEATURE_RESAMPLING: return "RESAMPLING";
        case AUDIO_FEATURE_SYNC: return "SYNC";
        case AUDIO_FEATURE_INTERFERENCE: return "INTERFERENCE";
        default: return "UNKNOWN";
    }
}

audio_test_result_t convert_bool_to_test_result(bool result) {
    return result ? AUDIO_TEST_PASS : AUDIO_TEST_FAIL;
}
