#ifndef TIZEN_DRM_TEST_H
#define TIZEN_DRM_TEST_H

#include <stdbool.h>
#include <stdint.h>
#include <drm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

// Test configuration
#define TEST_WIDTH 1920
#define TEST_HEIGHT 1080
#define TEST_ITERATIONS 100
#define TEST_TIMEOUT 5000

// Color formats
typedef enum {
    DRM_FORMAT_ARGB32 = 0x34325241,
    DRM_FORMAT_NV12 = 0x3231564e,
    DRM_FORMAT_NV21 = 0x3132564e,
    DRM_FORMAT_RGB565 = 0x59524742,
    DRM_FORMAT_XRGB8888 = 0x34325258,
    DRM_FORMAT_XR24 = 0x32315258,
    DRM_FORMAT_YUV420 = 0x30325559,
    DRM_FORMAT_YUV422 = 0x32325559,
    DRM_FORMAT_YUV444 = 0x34325559,
    DRM_FORMAT_UYVY = 0x59565955,
    DRM_FORMAT_YUYV = 0x56595559,
    DRM_FORMAT_YVYU = 0x55595659,
    DRM_FORMAT_VYUY = 0x59555956
} drm_format_t;

// Buffer modifiers
typedef enum {
    DRM_MODIFIER_LINEAR = 0x0,
    DRM_MODIFIER_TILED = 0x1,
    DRM_MODIFIER_X_TILED = 0x2,
    DRM_MODIFIER_Y_TILED = 0x3,
    DRM_MODIFIER_XY_TILED = 0x4,
    DRM_MODIFIER_IMT = 0x1000000000000001ULL,
    DRM_MODIFIER_SLM = 0x1000000000000002ULL,
    DRM_MODIFIER_VESA = 0x1000000000000003ULL
} drm_modifier_t;

// Compression algorithms
typedef enum {
    DRM_COMPRESSION_NONE = 0,
    DRM_COMPRESSION_ETC1 = 1,
    DRM_COMPRESSION_ETC2 = 2,
    DRM_COMPRESSION_ASTC = 3,
    DRM_COMPRESSION_BC1 = 4,
    DRM_COMPRESSION_BC2 = 5,
    DRM_COMPRESSION_BC3 = 6,
    DRM_COMPRESSION_BC4 = 7,
    DRM_COMPRESSION_BC5 = 8,
    DRM_COMPRESSION_BC6H = 9,
    DRM_COMPRESSION_BC7 = 10
} drm_compression_t;

// Test result codes
typedef enum {
    TEST_PASS = 0,
    TEST_FAIL = 1,
    TEST_SKIP = 2,
    TEST_ERROR = 3
} test_result_t;

// Test configuration structure
typedef struct {
    uint32_t width;
    uint32_t height;
    drm_format_t format;
    drm_modifier_t modifier;
    drm_compression_t compression;
    uint32_t iterations;
} test_config_t;

// Buffer structure
typedef struct {
    uint32_t handle;
    uint32_t size;
    drm_format_t format;
    drm_modifier_t modifier;
    drm_compression_t compression;
    void *map;
    uint32_t width;
    uint32_t height;
} drm_buffer_t;

// Plane structure
typedef struct {
    uint32_t id;
    uint32_t type;
    uint32_t possible_crtcs;
    uint32_t formats[32];
    uint32_t format_count;
} drm_plane_t;

// CRTC structure
typedef struct {
    uint32_t id;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t mode;
    uint32_t refresh_rate;
} drm_crtc_t;

// Connector structure
typedef struct {
    uint32_t id;
    uint32_t type;
    uint32_t connection;
    uint32_t width_mm;
    uint32_t height_mm;
} drm_connector_t;

// DRM Feature Types
typedef enum {
    DRM_FEATURE_BUFFER_SHARING,
    DRM_FEATURE_FORMAT_CONVERSION,
    DRM_FEATURE_PLANE_CONFIG,
    DRM_FEATURE_CRTC_CONFIG,
    DRM_FEATURE_CONNECTOR,
    DRM_FEATURE_MODE,
    DRM_FEATURE_VBLANK,
    DRM_FEATURE_SYNC,
    DRM_FEATURE_COLOR,
    DRM_FEATURE_GAMMA,
    DRM_FEATURE_BRIGHTNESS,
    DRM_FEATURE_CONTRAST,
    DRM_FEATURE_SATURATION,
    DRM_FEATURE_HUE,
    DRM_FEATURE_SHARPNESS,
    DRM_FEATURE_COLOR_TEMP,
    DRM_FEATURE_COLOR_SPACE,
    DRM_FEATURE_COLOR_PROFILE,
    DRM_FEATURE_COLOR_POINT,
    DRM_FEATURE_COLOR_GAMMA,
    DRM_FEATURE_COLOR_CURVE,
    DRM_FEATURE_COLOR_MATRIX,
    DRM_FEATURE_COLOR_TRANSFER,
    DRM_FEATURE_COLOR_TONE,
    DRM_FEATURE_COLOR_LUT
} drm_feature_t;

// Test framework functions
bool init_test_framework(void);
void cleanup_test_framework(void);
drm_buffer_t *create_drm_buffer(const test_config_t *config);
void destroy_drm_buffer(drm_buffer_t *buf);
bool fill_drm_buffer(drm_buffer_t *buf, uint32_t color);
bool verify_drm_buffer(drm_buffer_t *buf, uint32_t expected_color);
bool export_gem_handle(drm_buffer_t *buf, uint32_t *handle);
drm_buffer_t *import_gem_handle(uint32_t handle);
int export_dma_buf(drm_buffer_t *buf, int *fd);
drm_buffer_t *import_dma_buf(int fd);
bool test_buffer_performance(const test_config_t *config, uint32_t *avg_time);
bool test_format_conversion(const test_config_t *src_config, const test_config_t *dst_config);
bool test_buffer_sharing(const test_config_t *config);
bool test_plane_configuration(drm_plane_t *plane, const test_config_t *config);
bool test_crtc_configuration(drm_crtc_t *crtc, const test_config_t *config);
bool test_connector_properties(drm_connector_t *connector);
bool test_mode_setting(drm_mode_t *mode);
bool test_vblank_handling(void);
bool test_sync_primitives(void);
bool test_color_management(void);
bool test_cross_device_sharing(const test_config_t *config);
bool test_all_features(void);

#endif // TIZEN_DRM_TEST_H
