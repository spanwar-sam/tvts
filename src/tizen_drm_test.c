#include "tizen_drm_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <xf86drmMode.h>
#include <xf86drm.h>

static int drm_fd = -1;
static drmModeRes *resources = NULL;
static drmModeConnector *connector = NULL;
static drmModeCrtc *crtc = NULL;
static drmModePlaneRes *plane_resources = NULL;
static drmModePlane *primary_plane = NULL;
static drmModePlane *overlay_plane = NULL;
static drmModePlane *cursor_plane = NULL;
static uint32_t *format_list = NULL;
static uint32_t format_count = 0;
static uint32_t *compression_list = NULL;
static uint32_t compression_count = 0;
static uint64_t *modifier_list = NULL;
static uint32_t modifier_count = 0;

// Helper functions
static bool is_format_supported(uint32_t format) {
    for (uint32_t i = 0; i < format_count; i++) {
        if (format_list[i] == format) {
            return true;
        }
    }
    return false;
}

static bool is_compression_supported(uint32_t compression) {
    for (uint32_t i = 0; i < compression_count; i++) {
        if (compression_list[i] == compression) {
            return true;
        }
    }
    return false;
}

static bool is_modifier_supported(uint64_t modifier) {
    for (uint32_t i = 0; i < modifier_count; i++) {
        if (modifier_list[i] == modifier) {
            return true;
        }
    }
    return false;
}

static bool get_plane_by_type(uint32_t type, drmModePlane **plane) {
    if (!plane_resources) {
        plane_resources = drmModeGetPlaneResources(drm_fd);
        if (!plane_resources) {
            return false;
        }
    }

    for (uint32_t i = 0; i < plane_resources->count_planes; i++) {
        drmModePlane *p = drmModeGetPlane(drm_fd, plane_resources->planes[i]);
        if (p && p->type == type) {
            *plane = p;
            return true;
        }
        drmModeFreePlane(p);
    }
    return false;
}

static bool get_format_list(void) {
    if (format_list) {
        free(format_list);
    }
    format_count = 0;

    // Get supported formats from DRM
    drmModeConnector *conn = drmModeGetConnector(drm_fd, connector->connector_id);
    if (!conn) {
        return false;
    }

    format_count = conn->count_formats;
    format_list = malloc(sizeof(uint32_t) * format_count);
    if (!format_list) {
        drmModeFreeConnector(conn);
        return false;
    }

    memcpy(format_list, conn->formats, sizeof(uint32_t) * format_count);
    drmModeFreeConnector(conn);
    return true;
}

static bool get_compression_list(void) {
    if (compression_list) {
        free(compression_list);
    }
    compression_count = 0;

    // Get supported compression formats
    drmModeConnector *conn = drmModeGetConnector(drm_fd, connector->connector_id);
    if (!conn) {
        return false;
    }

    compression_count = conn->count_encoders;
    compression_list = malloc(sizeof(uint32_t) * compression_count);
    if (!compression_list) {
        drmModeFreeConnector(conn);
        return false;
    }

    // TODO: Get actual compression support from DRM
    compression_list[0] = DRM_COMPRESSION_NONE;
    compression_list[1] = DRM_COMPRESSION_ETC1;
    compression_list[2] = DRM_COMPRESSION_ETC2;
    compression_list[3] = DRM_COMPRESSION_ASTC;
    compression_list[4] = DRM_COMPRESSION_BC1;
    drmModeFreeConnector(conn);
    return true;
}

static bool get_modifier_list(void) {
    if (modifier_list) {
        free(modifier_list);
    }
    modifier_count = 0;

    // Get supported modifiers from DRM
    drmModeConnector *conn = drmModeGetConnector(drm_fd, connector->connector_id);
    if (!conn) {
        return false;
    }

    modifier_count = conn->count_encoders;
    modifier_list = malloc(sizeof(uint64_t) * modifier_count);
    if (!modifier_list) {
        drmModeFreeConnector(conn);
        return false;
    }

    // TODO: Get actual modifier support from DRM
    modifier_list[0] = DRM_MODIFIER_LINEAR;
    modifier_list[1] = DRM_MODIFIER_TILED;
    modifier_list[2] = DRM_MODIFIER_X_TILED;
    modifier_list[3] = DRM_MODIFIER_Y_TILED;
    modifier_list[4] = DRM_MODIFIER_XY_TILED;
    drmModeFreeConnector(conn);
    return true;
}

bool init_test_framework(void) {
    // Open DRM device
    drm_fd = open("/dev/dri/card0", O_RDWR);
    if (drm_fd < 0) {
        perror("Failed to open DRM device");
        return false;
    }

    // Get DRM resources
    resources = drmModeGetResources(drm_fd);
    if (!resources) {
        perror("Failed to get DRM resources");
        close(drm_fd);
        return false;
    }

    // Find a connected connector
    for (int i = 0; i < resources->count_connectors; i++) {
        connector = drmModeGetConnector(drm_fd, resources->connectors[i]);
        if (connector && connector->connection == DRM_MODE_CONNECTED) {
            break;
        }
        drmModeFreeConnector(connector);
    }

    if (!connector) {
        fprintf(stderr, "No connected display found\n");
        drmModeFreeResources(resources);
        close(drm_fd);
        return false;
    }

    // Find a CRTC
    for (int i = 0; i < resources->count_crtcs; i++) {
        if (connector->crtc_id == resources->crtcs[i]) {
            crtc = drmModeGetCrtc(drm_fd, resources->crtcs[i]);
            break;
        }
    }

    if (!crtc) {
        fprintf(stderr, "No CRTC found\n");
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(drm_fd);
        return false;
    }

    // Get plane resources
    plane_resources = drmModeGetPlaneResources(drm_fd);
    if (!plane_resources) {
        fprintf(stderr, "Failed to get plane resources\n");
        cleanup_test_framework();
        return false;
    }

    // Get primary plane
    if (!get_plane_by_type(DRM_PLANE_TYPE_PRIMARY, &primary_plane)) {
        fprintf(stderr, "Failed to get primary plane\n");
        cleanup_test_framework();
        return false;
    }

    // Get overlay plane
    if (!get_plane_by_type(DRM_PLANE_TYPE_OVERLAY, &overlay_plane)) {
        fprintf(stderr, "Failed to get overlay plane\n");
        cleanup_test_framework();
        return false;
    }

    // Get cursor plane
    if (!get_plane_by_type(DRM_PLANE_TYPE_CURSOR, &cursor_plane)) {
        fprintf(stderr, "Failed to get cursor plane\n");
        cleanup_test_framework();
        return false;
    }

    // Get supported formats
    if (!get_format_list()) {
        fprintf(stderr, "Failed to get supported formats\n");
        cleanup_test_framework();
        return false;
    }

    // Get supported compression formats
    if (!get_compression_list()) {
        fprintf(stderr, "Failed to get supported compression formats\n");
        cleanup_test_framework();
        return false;
    }

    // Get supported modifiers
    if (!get_modifier_list()) {
        fprintf(stderr, "Failed to get supported modifiers\n");
        cleanup_test_framework();
        return false;
    }

    return true;
}

void cleanup_test_framework(void) {
    if (primary_plane) {
        drmModeFreePlane(primary_plane);
    }
    if (overlay_plane) {
        drmModeFreePlane(overlay_plane);
    }
    if (cursor_plane) {
        drmModeFreePlane(cursor_plane);
    }
    if (plane_resources) {
        drmModeFreePlaneResources(plane_resources);
    }
    if (format_list) {
        free(format_list);
    }
    if (compression_list) {
        free(compression_list);
    }
    if (modifier_list) {
        free(modifier_list);
    }
    if (crtc) {
        drmModeFreeCrtc(crtc);
    }
    if (connector) {
        drmModeFreeConnector(connector);
    }
    if (resources) {
        drmModeFreeResources(resources);
    }
    if (drm_fd >= 0) {
        close(drm_fd);
    }
}

drm_buffer_t *create_drm_buffer(const test_config_t *config) {
    drm_buffer_t *buf = malloc(sizeof(drm_buffer_t));
    if (!buf) {
        return NULL;
    }

    memset(buf, 0, sizeof(drm_buffer_t));
    buf->width = config->width;
    buf->height = config->height;
    buf->format = config->format;
    buf->modifier = config->modifier;
    buf->compression = config->compression;

    // Create GEM buffer
    uint32_t handle;
    if (drmPrimeFDToHandle(drm_fd, -1, &handle) < 0) {
        free(buf);
        return NULL;
    }
    buf->handle = handle;

    // Get buffer size
    uint64_t size;
    if (drmIoctl(drm_fd, DRM_IOCTL_MODE_GETFB2, &size) < 0) {
        drmDestroyPropertyBlob(drm_fd, handle);
        free(buf);
        return NULL;
    }
    buf->size = size;

    // Map buffer
    buf->map = mmap(NULL, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED, drm_fd, 0);
    if (buf->map == MAP_FAILED) {
        drmDestroyPropertyBlob(drm_fd, handle);
        free(buf);
        return NULL;
    }

    return buf;
}

void destroy_drm_buffer(drm_buffer_t *buf) {
    if (buf->map) {
        munmap(buf->map, buf->size);
    }
    if (buf->handle > 0) {
        drmDestroyPropertyBlob(drm_fd, buf->handle);
    }
    free(buf);
}

bool fill_drm_buffer(drm_buffer_t *buf, uint32_t color) {
    if (!buf || !buf->map) {
        return false;
    }

    uint32_t *data = (uint32_t *)buf->map;
    for (uint32_t i = 0; i < (buf->width * buf->height); i++) {
        data[i] = color;
    }

    return true;
}

bool verify_drm_buffer(drm_buffer_t *buf, uint32_t expected_color) {
    if (!buf || !buf->map) {
        return false;
    }

    uint32_t *data = (uint32_t *)buf->map;
    for (uint32_t i = 0; i < (buf->width * buf->height); i++) {
        if (data[i] != expected_color) {
            return false;
        }
    }

    return true;
}

bool export_gem_handle(drm_buffer_t *buf, uint32_t *handle) {
    if (!buf || !handle) {
        return false;
    }

    *handle = buf->handle;
    return true;
}

drm_buffer_t *import_gem_handle(uint32_t handle) {
    if (drm_fd < 0) {
        return NULL;
    }

    uint64_t size;
    if (drmIoctl(drm_fd, DRM_IOCTL_MODE_GETFB2, &size) < 0) {
        return NULL;
    }

    drm_buffer_t *buf = malloc(sizeof(drm_buffer_t));
    if (!buf) {
        return NULL;
    }

    memset(buf, 0, sizeof(drm_buffer_t));
    buf->handle = handle;
    buf->size = size;

    // Map buffer
    buf->map = mmap(NULL, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED, drm_fd, 0);
    if (buf->map == MAP_FAILED) {
        free(buf);
        return NULL;
    }

    return buf;
}

int export_dma_buf(drm_buffer_t *buf, int *fd) {
    if (!buf || !fd) {
        return -1;
    }

    *fd = drmPrimeHandleToFD(drm_fd, buf->handle, 0);
    if (*fd < 0) {
        return -1;
    }

    return 0;
}

drm_buffer_t *import_dma_buf(int fd) {
    if (drm_fd < 0 || fd < 0) {
        return NULL;
    }

    uint32_t handle;
    if (drmPrimeFDToHandle(drm_fd, fd, &handle) < 0) {
        return NULL;
    }

    return import_gem_handle(handle);
}

bool test_buffer_performance(const test_config_t *config, uint32_t *avg_time) {
    if (!config || !avg_time) {
        return false;
    }

    uint64_t total_time = 0;
    for (uint32_t i = 0; i < config->iterations; i++) {
        struct timespec start, end;
        
        // Create buffer
        drm_buffer_t *buf = create_drm_buffer(config);
        if (!buf) {
            return false;
        }

        // Start timer
        clock_gettime(CLOCK_MONOTONIC, &start);

        // Export and import
        int fd;
        if (export_dma_buf(buf, &fd) < 0) {
            destroy_drm_buffer(buf);
            return false;
        }

        drm_buffer_t *imported = import_dma_buf(fd);
        if (!imported) {
            close(fd);
            destroy_drm_buffer(buf);
            return false;
        }

        // End timer
        clock_gettime(CLOCK_MONOTONIC, &end);
        total_time += (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

        // Cleanup
        destroy_drm_buffer(buf);
        destroy_drm_buffer(imported);
        close(fd);
    }

    *avg_time = total_time / (config->iterations * 1000000);
    return true;
}

bool test_format_conversion(const test_config_t *src_config, const test_config_t *dst_config) {
    if (!src_config || !dst_config) {
        return false;
    }

    // Create source buffer
    drm_buffer_t *src_buf = create_drm_buffer(src_config);
    if (!src_buf) {
        return false;
    }

    // Fill source buffer
    if (!fill_drm_buffer(src_buf, 0xFF0000FF)) {
        destroy_drm_buffer(src_buf);
        return false;
    }

    // Export as DMA-BUF
    int fd;
    if (export_dma_buf(src_buf, &fd) < 0) {
        destroy_drm_buffer(src_buf);
        return false;
    }

    // Import with target format
    drm_buffer_t *dst_buf = import_dma_buf(fd);
    if (!dst_buf) {
        close(fd);
        destroy_drm_buffer(src_buf);
        return false;
    }

    // Verify conversion
    bool result = verify_drm_buffer(dst_buf, 0xFF0000FF);

    // Cleanup
    destroy_drm_buffer(src_buf);
    destroy_drm_buffer(dst_buf);
    close(fd);

    return result;
}

bool test_crtc_configuration(drm_crtc_t *crtc, const test_config_t *config) {
    if (!crtc || !config) {
        return false;
    }

    // Create buffer
    drm_buffer_t *buf = create_drm_buffer(config);
    if (!buf) {
        return false;
    }

    // Fill buffer
    if (!fill_drm_buffer(buf, 0xFF0000FF)) {
        destroy_drm_buffer(buf);
        return false;
    }

    // Configure CRTC
    drmModeAtomicReq *req = drmModeAtomicAlloc();
    if (!req) {
        destroy_drm_buffer(buf);
        return false;
    }

    // Add properties
    if (!drmModeAtomicAddProperty(req, crtc->id, "ACTIVE", 1) ||
        !drmModeAtomicAddProperty(req, crtc->id, "MODE_ID", crtc->mode) ||
        !drmModeAtomicAddProperty(req, crtc->id, "X", crtc->x) ||
        !drmModeAtomicAddProperty(req, crtc->id, "Y", crtc->y) ||
        !drmModeAtomicAddProperty(req, crtc->id, "WIDTH", crtc->width) ||
        !drmModeAtomicAddProperty(req, crtc->id, "HEIGHT", crtc->height)) {
        drmModeAtomicFree(req);
        destroy_drm_buffer(buf);
        return false;
    }

    // Commit
    bool result = drmModeAtomicCommit(drm_fd, req, DRM_MODE_ATOMIC_NONBLOCK, NULL) == 0;
    
    // Cleanup
    drmModeAtomicFree(req);
    destroy_drm_buffer(buf);

    return result;
}

bool test_connector_properties(drm_connector_t *connector) {
    if (!connector) {
        return false;
    }

    // Get connector properties
    drmModeConnector *conn = drmModeGetConnector(drm_fd, connector->id);
    if (!conn) {
        return false;
    }

    // Verify properties
    bool result = true;
    result &= conn->type == connector->type;
    result &= conn->connection == connector->connection;
    result &= conn->mmWidth == connector->width_mm;
    result &= conn->mmHeight == connector->height_mm;

    // Cleanup
    drmModeFreeConnector(conn);
    return result;
}

bool test_mode_setting(drm_mode_t *mode) {
    if (!mode) {
        return false;
    }

    // Get mode info
    drmModeModeInfo *info = drmModeGetModeInfo(drm_fd, mode->id);
    if (!info) {
        return false;
    }

    // Verify mode properties
    bool result = true;
    result &= info->hdisplay == mode->width;
    result &= info->vdisplay == mode->height;
    result &= info->vrefresh == mode->refresh_rate;

    // Cleanup
    free(info);
    return result;
}

bool test_vblank_handling(void) {
    struct pollfd fds[1];
    uint32_t sequence;
    int ret;

    // Request VBLANK event
    ret = drmWaitVBlank(drm_fd, &sequence);
    if (ret < 0) {
        return false;
    }

    // Wait for VBLANK event
    fds[0].fd = drm_fd;
    fds[0].events = POLLIN;
    ret = poll(fds, 1, TEST_TIMEOUT);
    if (ret <= 0) {
        return false;
    }

    return true;
}

bool test_sync_primitives(void) {
    // Create sync object
    uint32_t sync_obj;
    if (drmSyncobjCreate(drm_fd, 0, &sync_obj) < 0) {
        return false;
    }

    // Signal sync object
    if (drmSyncobjSignal(drm_fd, &sync_obj, 1) < 0) {
        drmSyncobjDestroy(drm_fd, sync_obj);
        return false;
    }

    // Wait for sync object
    uint32_t timeout = TEST_TIMEOUT;
    if (drmSyncobjWait(drm_fd, &sync_obj, 1, timeout, NULL, NULL) < 0) {
        drmSyncobjDestroy(drm_fd, sync_obj);
        return false;
    }

    // Cleanup
    drmSyncobjDestroy(drm_fd, sync_obj);
    return true;
}

bool test_color_management(void) {
    // Test gamma ramp
    uint16_t gamma[3][4096];
    memset(gamma, 0, sizeof(gamma));

    // Set gamma ramp
    if (drmModeSetCrtcGamma(drm_fd, crtc->id, 4096, gamma[0], gamma[1], gamma[2]) < 0) {
        return false;
    }

    // Get gamma ramp
    uint16_t size;
    uint16_t red[4096], green[4096], blue[4096];
    if (drmModeGetCrtcGamma(drm_fd, crtc->id, &size, red, green, blue) < 0) {
        return false;
    }

    // Verify gamma ramp
    bool result = true;
    result &= size == 4096;
    for (uint32_t i = 0; i < 4096; i++) {
        result &= red[i] == gamma[0][i];
        result &= green[i] == gamma[1][i];
        result &= blue[i] == gamma[2][i];
    }

    return result;
}

bool test_all_features(void) {
    // Test buffer sharing
    test_config_t config = {
        .width = TEST_WIDTH,
        .height = TEST_HEIGHT,
        .format = DRM_FORMAT_ARGB32,
        .modifier = DRM_MODIFIER_LINEAR,
        .compression = DRM_COMPRESSION_NONE,
        .iterations = TEST_ITERATIONS
    };

    bool result = true;
    result &= test_buffer_sharing(&config);
    result &= test_format_conversion(&config, &config);
    result &= test_plane_configuration(primary_plane, &config);
    result &= test_crtc_configuration((drm_crtc_t *)crtc, &config);
    result &= test_connector_properties((drm_connector_t *)connector);
    result &= test_mode_setting((drm_mode_t *)crtc);
    result &= test_vblank_handling();
    result &= test_sync_primitives();
    result &= test_color_management();
    result &= test_cross_device_sharing(&config);

    return result;
}

bool test_plane_configuration(drm_plane_t *plane, const test_config_t *config) {
    if (!plane || !config) {
        return false;
    }

    // Create buffer
    drm_buffer_t *buf = create_drm_buffer(config);
    if (!buf) {
        return false;
    }

    // Fill buffer
    if (!fill_drm_buffer(buf, 0x0000FFFF)) {
        destroy_drm_buffer(buf);
        return false;
    }

    // Configure plane
    drmModeAtomicReq *req = drmModeAtomicAlloc();
    if (!req) {
        destroy_drm_buffer(buf);
        return false;
    }

    // Add properties
    drmModeAtomicAddProperty(req, plane->id, "FB_ID", buf->handle);
    drmModeAtomicAddProperty(req, plane->id, "CRTC_ID", crtc->crtc_id);
    drmModeAtomicAddProperty(req, plane->id, "CRTC_X", 0);
    drmModeAtomicAddProperty(req, plane->id, "CRTC_Y", 0);
    drmModeAtomicAddProperty(req, plane->id, "CRTC_W", config->width);
    drmModeAtomicAddProperty(req, plane->id, "CRTC_H", config->height);
    drmModeAtomicAddProperty(req, plane->id, "SRC_X", 0);
    drmModeAtomicAddProperty(req, plane->id, "SRC_Y", 0);
    drmModeAtomicAddProperty(req, plane->id, "SRC_W", config->width << 16);
    drmModeAtomicAddProperty(req, plane->id, "SRC_H", config->height << 16);

    // Commit
    bool result = drmModeAtomicCommit(drm_fd, req, DRM_MODE_ATOMIC_NONBLOCK, NULL) == 0;
    
    // Cleanup
    drmModeAtomicFree(req);
    destroy_drm_buffer(buf);

    return result;
}

bool test_cross_device_sharing(const test_config_t *config) {
    if (!config) {
        return false;
    }

    // Create buffer
    drm_buffer_t *buf = create_drm_buffer(config);
    if (!buf) {
        return false;
    }

    // Fill buffer
    if (!fill_drm_buffer(buf, 0xFF00FF00)) {
        destroy_drm_buffer(buf);
        return false;
    }

    // Export as DMA-BUF
    int fd;
    if (export_dma_buf(buf, &fd) < 0) {
        destroy_drm_buffer(buf);
        return false;
    }

    // Open second DRM device
    int second_drm_fd = open("/dev/dri/card1", O_RDWR);
    if (second_drm_fd < 0) {
        close(fd);
        destroy_drm_buffer(buf);
        return false;
    }

    // Import on second device
    uint32_t handle;
    if (drmPrimeFDToHandle(second_drm_fd, fd, &handle) < 0) {
        close(second_drm_fd);
        close(fd);
        destroy_drm_buffer(buf);
        return false;
    }

    // Verify on second device
    drm_buffer_t *imported = import_gem_handle(handle);
    if (!imported || !verify_drm_buffer(imported, 0xFF00FF00)) {
        close(second_drm_fd);
        close(fd);
        destroy_drm_buffer(buf);
        return false;
    }

    // Cleanup
    close(second_drm_fd);
    close(fd);
    destroy_drm_buffer(buf);
    destroy_drm_buffer(imported);

    return true;
}
