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

#include "video/tizen_video_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <linux/videodev2.h>

#define TEST_TIMEOUT 5000 // 5 seconds
#define MAX_DEVICES 16

static int video_fd = -1;
static video_device_info_t *devices = NULL;
static uint32_t device_count = 0;

// Static helper functions
static bool open_video_device(uint32_t device_index) {
    char device_name[32];
    snprintf(device_name, sizeof(device_name), "/dev/video%d", device_index);
    
    video_fd = open(device_name, O_RDWR);
    if (video_fd < 0) {
        fprintf(stderr, "Cannot open video device %s: %s\n", device_name, strerror(errno));
        return false;
    }
    
    return true;
}

static bool get_device_capabilities(uint32_t device_index, video_device_info_t *info) {
    if (!open_video_device(device_index)) {
        return false;
    }
    
    struct v4l2_capability cap;
    if (ioctl(video_fd, VIDIOC_QUERYCAP, &cap) < 0) {
        fprintf(stderr, "VIDIOC_QUERYCAP failed: %s\n", strerror(errno));
        close(video_fd);
        video_fd = -1;
        return false;
    }
    
    // Check device type
    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
        info->type = VIDEO_DEVICE_CAMERA;
    } else if (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT) {
        info->type = VIDEO_DEVICE_ENCODER;
    } else {
        info->type = VIDEO_DEVICE_CONVERTER;
    }
    
    // Get device name
    strncpy(info->name, (const char *)cap.card, sizeof(info->name));
    
    // Get supported formats
    struct v4l2_fmtdesc fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    uint32_t format_index = 0;
    while (ioctl(video_fd, VIDIOC_ENUM_FMT, &fmt) >= 0 && format_index < 16) {
        // Map V4L2 formats to our format enum
        switch (fmt.pixelformat) {
            case V4L2_PIX_FMT_RGB565:
                info->formats[format_index] = VIDEO_FORMAT_RGB565;
                break;
            case V4L2_PIX_FMT_RGB24:
                info->formats[format_index] = VIDEO_FORMAT_RGB888;
                break;
            case V4L2_PIX_FMT_RGBA32:
                info->formats[format_index] = VIDEO_FORMAT_RGBA8888;
                break;
            case V4L2_PIX_FMT_NV12:
                info->formats[format_index] = VIDEO_FORMAT_NV12;
                break;
            case V4L2_PIX_FMT_YUV420:
                info->formats[format_index] = VIDEO_FORMAT_YUV420;
                break;
            case V4L2_PIX_FMT_YUV422P:
                info->formats[format_index] = VIDEO_FORMAT_YUV422;
                break;
            case V4L2_PIX_FMT_YUYV:
                info->formats[format_index] = VIDEO_FORMAT_YUYV;
                break;
            case V4L2_PIX_FMT_UYVY:
                info->formats[format_index] = VIDEO_FORMAT_UYVY;
                break;
            case V4L2_PIX_FMT_MJPEG:
                info->formats[format_index] = VIDEO_FORMAT_MJPEG;
                break;
            case V4L2_PIX_FMT_H264:
                info->formats[format_index] = VIDEO_FORMAT_H264;
                break;
            default:
                // Unsupported format, skip
                fmt.index++;
                continue;
        }
        
        format_index++;
        fmt.index++;
    }
    
    info->format_count = format_index;
    
    // Get supported resolutions and framerates
    // For simplicity, we just set some common values
    info->min_width = 320;
    info->max_width = 1920;
    info->min_height = 240;
    info->max_height = 1080;
    
    info->framerates[0] = 15;
    info->framerates[1] = 30;
    info->framerates[2] = 60;
    info->framerate_count = 3;
    
    info->min_bitrate = 100000;    // 100 Kbps
    info->max_bitrate = 10000000;  // 10 Mbps
    
    close(video_fd);
    video_fd = -1;
    
    return true;
}

// Framework initialization/cleanup
bool init_video_test_framework(void) {
    // Discover available devices
    device_count = 0;
    
    // Count devices first
    for (int i = 0; i < MAX_DEVICES; i++) {
        char device_name[32];
        snprintf(device_name, sizeof(device_name), "/dev/video%d", i);
        
        int fd = open(device_name, O_RDWR);
        if (fd >= 0) {
            close(fd);
            device_count++;
        }
    }
    
    if (device_count == 0) {
        fprintf(stderr, "No video devices found\n");
        return false;
    }
    
    // Allocate device info array
    devices = malloc(device_count * sizeof(video_device_info_t));
    if (!devices) {
        fprintf(stderr, "Memory allocation failed\n");
        return false;
    }
    
    // Fill device info
    uint32_t device_index = 0;
    for (int i = 0; i < MAX_DEVICES && device_index < device_count; i++) {
        char device_name[32];
        snprintf(device_name, sizeof(device_name), "/dev/video%d", i);
        
        int fd = open(device_name, O_RDWR);
        if (fd >= 0) {
            close(fd);
            // Get device capabilities
            if (!get_device_capabilities(i, &devices[device_index])) {
                fprintf(stderr, "Failed to get capabilities for %s\n", device_name);
                continue;
            }
            device_index++;
        }
    }
    
    return true;
}

void cleanup_video_test_framework(void) {
    if (video_fd >= 0) {
        close(video_fd);
        video_fd = -1;
    }
    
    if (devices) {
        free(devices);
        devices = NULL;
    }
    
    device_count = 0;
}

// Device enumeration and information
uint32_t get_video_device_count(video_device_type_t type) {
    uint32_t count = 0;
    
    for (uint32_t i = 0; i < device_count; i++) {
        if (devices[i].type == type || type == VIDEO_DEVICE_MAX) {
            count++;
        }
    }
    
    return count;
}

bool get_video_device_info(uint32_t device_index, video_device_info_t *info) {
    if (device_index >= device_count || !info) {
        return false;
    }
    
    memcpy(info, &devices[device_index], sizeof(video_device_info_t));
    return true;
}

// Buffer management
video_buffer_t *create_video_buffer(const video_test_config_t *config) {
    if (!config) {
        return NULL;
    }
    
    video_buffer_t *buffer = malloc(sizeof(video_buffer_t));
    if (!buffer) {
        return NULL;
    }
    
    // Calculate buffer size based on format
    uint32_t bytes_per_pixel;
    switch (config->format) {
        case VIDEO_FORMAT_RGB565:
            bytes_per_pixel = 2;
            break;
        case VIDEO_FORMAT_RGB888:
            bytes_per_pixel = 3;
            break;
        case VIDEO_FORMAT_RGBA8888:
        case VIDEO_FORMAT_ARGB8888:
            bytes_per_pixel = 4;
            break;
        case VIDEO_FORMAT_NV12:
        case VIDEO_FORMAT_YUV420:
            bytes_per_pixel = 3/2; // 12 bits per pixel
            break;
        case VIDEO_FORMAT_YUV422:
        case VIDEO_FORMAT_YUYV:
        case VIDEO_FORMAT_UYVY:
            bytes_per_pixel = 2;
            break;
        default:
            // Default to RGB888
            bytes_per_pixel = 3;
            break;
    }
    
    buffer->width = config->width;
    buffer->height = config->height;
    buffer->format = config->format;
    buffer->stride = config->width * bytes_per_pixel;
    buffer->framerate = config->framerate;
    buffer->timestamp = 0;
    
    // For planar formats, sizing is more complex, but simplified here
    buffer->size = buffer->stride * buffer->height;
    buffer->data = malloc(buffer->size);
    
    if (!buffer->data) {
        free(buffer);
        return NULL;
    }
    
    return buffer;
}

bool fill_video_buffer(video_buffer_t *buffer, uint32_t pattern) {
    if (!buffer || !buffer->data) {
        return false;
    }
    
    // Simple pattern fill
    memset(buffer->data, pattern & 0xFF, buffer->size);
    return true;
}

bool verify_video_buffer(video_buffer_t *buffer, uint32_t pattern) {
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

void destroy_video_buffer(video_buffer_t *buffer) {
    if (buffer) {
        if (buffer->data) {
            free(buffer->data);
        }
        free(buffer);
    }
}

// Feature testing
bool test_video_capture(uint32_t device_index, const video_test_config_t *config) {
    if (!open_video_device(device_index)) {
        return false;
    }
    
    // Set format
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = config->width;
    fmt.fmt.pix.height = config->height;
    
    // Map format to V4L2
    switch (config->format) {
        case VIDEO_FORMAT_RGB565:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;
            break;
        case VIDEO_FORMAT_RGB888:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
            break;
        case VIDEO_FORMAT_RGBA8888:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;
            break;
        case VIDEO_FORMAT_NV12:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
            break;
        case VIDEO_FORMAT_YUV420:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
            break;
        case VIDEO_FORMAT_YUV422:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV422P;
            break;
        case VIDEO_FORMAT_YUYV:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
            break;
        case VIDEO_FORMAT_UYVY:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
            break;
        default:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
            break;
    }
    
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    
    if (ioctl(video_fd, VIDIOC_S_FMT, &fmt) < 0) {
        fprintf(stderr, "VIDIOC_S_FMT failed: %s\n", strerror(errno));
        close(video_fd);
        video_fd = -1;
        return false;
    }
    
    // Set framerate
    struct v4l2_streamparm parm;
    memset(&parm, 0, sizeof(parm));
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = config->framerate;
    
    if (ioctl(video_fd, VIDIOC_S_PARM, &parm) < 0) {
        fprintf(stderr, "VIDIOC_S_PARM failed: %s\n", strerror(errno));
        close(video_fd);
        video_fd = -1;
        return false;
    }
    
    // Request buffers
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    
    if (ioctl(video_fd, VIDIOC_REQBUFS, &req) < 0) {
        fprintf(stderr, "VIDIOC_REQBUFS failed: %s\n", strerror(errno));
        close(video_fd);
        video_fd = -1;
        return false;
    }
    
    // Map buffers
    struct v4l2_buffer buf;
    void *buffers[4];
    for (uint32_t i = 0; i < req.count; i++) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        
        if (ioctl(video_fd, VIDIOC_QUERYBUF, &buf) < 0) {
            fprintf(stderr, "VIDIOC_QUERYBUF failed: %s\n", strerror(errno));
            close(video_fd);
            video_fd = -1;
            return false;
        }
        
        buffers[i] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, video_fd, buf.m.offset);
        if (buffers[i] == MAP_FAILED) {
            fprintf(stderr, "mmap failed: %s\n", strerror(errno));
            close(video_fd);
            video_fd = -1;
            return false;
        }
    }
    
    // Queue buffers
    for (uint32_t i = 0; i < req.count; i++) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        
        if (ioctl(video_fd, VIDIOC_QBUF, &buf) < 0) {
            fprintf(stderr, "VIDIOC_QBUF failed: %s\n", strerror(errno));
            
            // Unmap buffers
            for (uint32_t j = 0; j < i; j++) {
                munmap(buffers[j], buf.length);
            }
            
            close(video_fd);
            video_fd = -1;
            return false;
        }
    }
    
    // Start streaming
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(video_fd, VIDIOC_STREAMON, &type) < 0) {
        fprintf(stderr, "VIDIOC_STREAMON failed: %s\n", strerror(errno));
        
        // Unmap buffers
        for (uint32_t i = 0; i < req.count; i++) {
            munmap(buffers[i], buf.length);
        }
        
        close(video_fd);
        video_fd = -1;
        return false;
    }
    
    // Capture one frame
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    
    if (ioctl(video_fd, VIDIOC_DQBUF, &buf) < 0) {
        fprintf(stderr, "VIDIOC_DQBUF failed: %s\n", strerror(errno));
        
        // Stop streaming
        ioctl(video_fd, VIDIOC_STREAMOFF, &type);
        
        // Unmap buffers
        for (uint32_t i = 0; i < req.count; i++) {
            munmap(buffers[i], buf.length);
        }
        
        close(video_fd);
        video_fd = -1;
        return false;
    }
    
    // Process the captured frame (simplified)
    printf("Captured frame with size: %d bytes\n", buf.bytesused);
    
    // Requeue the buffer
    if (ioctl(video_fd, VIDIOC_QBUF, &buf) < 0) {
        fprintf(stderr, "VIDIOC_QBUF failed: %s\n", strerror(errno));
    }
    
    // Stop streaming
    if (ioctl(video_fd, VIDIOC_STREAMOFF, &type) < 0) {
        fprintf(stderr, "VIDIOC_STREAMOFF failed: %s\n", strerror(errno));
    }
    
    // Unmap buffers
    for (uint32_t i = 0; i < req.count; i++) {
        munmap(buffers[i], buf.length);
    }
    
    // Cleanup
    close(video_fd);
    video_fd = -1;
    
    return true;
}

// Comprehensive testing
bool test_all_video_features(uint32_t device_index, const video_test_config_t *config) {
    bool result = true;
    
    result &= test_video_capture(device_index, config);
    
    // Add calls to other test functions here
    
    return result;
}

// Utilities
const char *video_format_to_string(video_format_t format) {
    switch (format) {
        case VIDEO_FORMAT_RGB565: return "RGB565";
        case VIDEO_FORMAT_RGB888: return "RGB888";
        case VIDEO_FORMAT_RGBA8888: return "RGBA8888";
        case VIDEO_FORMAT_ARGB8888: return "ARGB8888";
        case VIDEO_FORMAT_NV12: return "NV12";
        case VIDEO_FORMAT_YUV420: return "YUV420";
        case VIDEO_FORMAT_YUV422: return "YUV422";
        case VIDEO_FORMAT_YUYV: return "YUYV";
        case VIDEO_FORMAT_UYVY: return "UYVY";
        case VIDEO_FORMAT_MJPEG: return "MJPEG";
        case VIDEO_FORMAT_H264: return "H264";
        case VIDEO_FORMAT_H265: return "H265";
        case VIDEO_FORMAT_VP8: return "VP8";
        case VIDEO_FORMAT_VP9: return "VP9";
        default: return "UNKNOWN";
    }
}

const char *video_device_type_to_string(video_device_type_t type) {
    switch (type) {
        case VIDEO_DEVICE_CAMERA: return "CAMERA";
        case VIDEO_DEVICE_ENCODER: return "ENCODER";
        case VIDEO_DEVICE_DECODER: return "DECODER";
        case VIDEO_DEVICE_CONVERTER: return "CONVERTER";
        case VIDEO_DEVICE_SCALER: return "SCALER";
        default: return "UNKNOWN";
    }
}

const char *video_feature_to_string(video_feature_t feature) {
    switch (feature) {
        case VIDEO_FEATURE_CAPTURE: return "CAPTURE";
        case VIDEO_FEATURE_ENCODING: return "ENCODING";
        case VIDEO_FEATURE_DECODING: return "DECODING";
        case VIDEO_FEATURE_FORMAT_SUPPORT: return "FORMAT_SUPPORT";
        case VIDEO_FEATURE_RESOLUTION: return "RESOLUTION";
        case VIDEO_FEATURE_FRAMERATES: return "FRAMERATES";
        case VIDEO_FEATURE_CONVERSION: return "CONVERSION";
        case VIDEO_FEATURE_SCALING: return "SCALING";
        case VIDEO_FEATURE_ROTATION: return "ROTATION";
        case VIDEO_FEATURE_COMPRESSION: return "COMPRESSION";
        case VIDEO_FEATURE_STREAMING: return "STREAMING";
        case VIDEO_FEATURE_SYNC: return "SYNC";
        default: return "UNKNOWN";
    }
}

video_test_result_t convert_bool_to_test_result(bool result) {
    return result ? VIDEO_TEST_PASS : VIDEO_TEST_FAIL;
}
