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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>

// Include subsystem headers
#include "tizen_drm_test.h"
#include "audio/tizen_audio_test.h"
#include "video/tizen_video_test.h"
#include "usb/tizen_usb_test.h"
#include "report/test_report.h"

// Subsystem types
typedef enum {
    SUBSYSTEM_DRM,
    SUBSYSTEM_AUDIO,
    SUBSYSTEM_VIDEO,
    SUBSYSTEM_USB,
    SUBSYSTEM_ALL
} subsystem_type_t;

// Command line options
typedef struct {
    // Common options
    subsystem_type_t subsystem;
    char *test_name;
    uint32_t device_index;
    uint32_t width;
    uint32_t height;
    uint32_t sample_rate;
    uint32_t iterations;
    bool verbose;
    bool help;
    
    // Reporting options
    report_format_t report_format;
    char report_file[256];
    bool report_append;
    bool no_report;
    
    // USB test options
    const char *usb_device_path;
    const char *usb_test_device_class;
    uint16_t usb_vendor_id;
    uint16_t usb_product_id;
} cmd_options_t;

// Global report handle
test_report_t *g_report = NULL;

// Function to print test result for any subsystem
void print_test_result(const char *test_name, int result) {
    printf("%s: ", test_name);
    if (result) {
        printf("\033[32mPASS\033[0m\n");
    } else {
        printf("\033[31mFAIL\033[0m\n");
    }
    
    // Add to report if available
    if (g_report) {
        report_add_test_result(g_report, test_name, REPORT_SUBSYSTEM_DRM, 
                              result ? TEST_RESULT_PASS : TEST_RESULT_FAIL, 
                              0, result ? "Test passed" : "Test failed");
    }
}

// Function to print performance metrics
void print_performance_metrics(const char *test_name, uint32_t time_us) {
    printf("%s Performance: %u microseconds\n", test_name, time_us);
    
    // Add to report if available
    if (g_report) {
        report_add_time_metric(g_report, test_name, (double)time_us);
    }
}

// Function to print color metrics
void print_color_metrics(const char *test_name, uint16_t red, uint16_t green, uint16_t blue) {
    printf("%s Color Metrics: R=%u G=%u B=%u\n", test_name, red, green, blue);
}

// Function to print audio metrics
void print_audio_metrics(const char *test_name, uint32_t latency_ms) {
    printf("%s Latency: %u ms\n", test_name, latency_ms);
}

// Function to print video metrics
void print_video_metrics(const char *test_name, uint32_t fps) {
    printf("%s Frame Rate: %u FPS\n", test_name, fps);
}

// Function to print usage information
void print_usage(const char *program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  -s, --subsystem=SUBSYSTEM   Subsystem to test (drm, audio, video, usb, all)\n");
    printf("  -t, --test=TEST_NAME       Specific test to run\n");
    printf("  -d, --device=INDEX         Device index to test\n");
    printf("  -w, --width=WIDTH          Width for video/DRM tests\n");
    printf("  -h, --height=HEIGHT        Height for video/DRM tests\n");
    printf("  -r, --rate=SAMPLE_RATE     Sample rate for audio tests\n");
    printf("  -i, --iterations=COUNT     Number of test iterations\n");
    printf("  -v, --verbose              Enable verbose output\n");
    printf("  --report-format=FORMAT     Report format (text, json, html, xml, csv)\n");
    printf("  --report-file=FILE         Report file path\n");
    printf("  --report-append            Append to existing report file\n");
    printf("  --no-report                Disable report generation\n");
    printf("  --help                     Show this help message\n\n");
    printf("USB Test Options:\n");
    printf("  --usb-device-path PATH     Path to USB device (default: /dev/sda)\n");
    printf("  --usb-test-device-class CLASS  Test specific USB device class (msc, hid, audio, wireless)\n");
    printf("  --usb-vendor-id ID         Filter by USB vendor ID (hex)\n");
    printf("  --usb-product-id ID        Filter by USB product ID (hex)\n");
}

// Function to parse command line options
cmd_options_t parse_options(int argc, char *argv[]) {
    cmd_options_t options = {
        .subsystem = SUBSYSTEM_ALL,
        .test_name = NULL,
        .device_index = 0,
        .width = 1280,
        .height = 720,
        .sample_rate = 44100,
        .iterations = 1,
        .verbose = false,
        .help = false,
        .report_format = REPORT_FORMAT_TEXT,
        .report_append = false,
        .no_report = false,
        .usb_device_path = "/dev/sda",
        .usb_test_device_class = NULL,
        .usb_vendor_id = 0,
        .usb_product_id = 0
    };
    
    // Set default report file
    strcpy(options.report_file, "test_report.txt");

    static struct option long_options[] = {
        {"subsystem", required_argument, 0, 's'},
        {"test", required_argument, 0, 't'},
        {"device", required_argument, 0, 'd'},
        {"width", required_argument, 0, 'w'},
        {"height", required_argument, 0, 'h'},
        {"rate", required_argument, 0, 'r'},
        {"iterations", required_argument, 0, 'i'},
        {"verbose", no_argument, 0, 'v'},
        {"report-format", required_argument, 0, 0},
        {"report-file", required_argument, 0, 0},
        {"report-append", no_argument, 0, 0},
        {"no-report", no_argument, 0, 0},
        {"help", no_argument, 0, 0},
        {"usb-device-path", required_argument, 0, 0},
        {"usb-test-device-class", required_argument, 0, 0},
        {"usb-vendor-id", required_argument, 0, 0},
        {"usb-product-id", required_argument, 0, 0},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "s:t:d:w:h:r:i:v", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                if (strcmp(long_options[option_index].name, "help") == 0) {
                    options.help = true;
                } else if (strcmp(long_options[option_index].name, "report-format") == 0) {
                    if (strcmp(optarg, "text") == 0) {
                        options.report_format = REPORT_FORMAT_TEXT;
                    } else if (strcmp(optarg, "json") == 0) {
                        options.report_format = REPORT_FORMAT_JSON;
                    } else if (strcmp(optarg, "html") == 0) {
                        options.report_format = REPORT_FORMAT_HTML;
                    } else if (strcmp(optarg, "xml") == 0) {
                        options.report_format = REPORT_FORMAT_XML;
                    } else if (strcmp(optarg, "csv") == 0) {
                        options.report_format = REPORT_FORMAT_CSV;
                    } else {
                        fprintf(stderr, "Unknown report format: %s\n", optarg);
                    }
                } else if (strcmp(long_options[option_index].name, "report-file") == 0) {
                    strncpy(options.report_file, optarg, sizeof(options.report_file) - 1);
                } else if (strcmp(long_options[option_index].name, "report-append") == 0) {
                    options.report_append = true;
                } else if (strcmp(long_options[option_index].name, "no-report") == 0) {
                    options.no_report = true;
                } else if (strcmp(long_options[option_index].name, "usb-device-path") == 0) {
                    options.usb_device_path = optarg;
                } else if (strcmp(long_options[option_index].name, "usb-test-device-class") == 0) {
                    options.usb_test_device_class = optarg;
                } else if (strcmp(long_options[option_index].name, "usb-vendor-id") == 0) {
                    options.usb_vendor_id = (uint16_t)strtoul(optarg, NULL, 16);
                } else if (strcmp(long_options[option_index].name, "usb-product-id") == 0) {
                    options.usb_product_id = (uint16_t)strtoul(optarg, NULL, 16);
                }
                break;
            case 's':
                if (strcmp(optarg, "drm") == 0) {
                    options.subsystem = SUBSYSTEM_DRM;
                } else if (strcmp(optarg, "audio") == 0) {
                    options.subsystem = SUBSYSTEM_AUDIO;
                } else if (strcmp(optarg, "video") == 0) {
                    options.subsystem = SUBSYSTEM_VIDEO;
                } else if (strcmp(optarg, "usb") == 0) {
                    options.subsystem = SUBSYSTEM_USB;
                } else if (strcmp(optarg, "all") == 0) {
                    options.subsystem = SUBSYSTEM_ALL;
                } else {
                    fprintf(stderr, "Unknown subsystem: %s\n", optarg);
                }
                break;
            case 't':
                options.test_name = optarg;
                break;
            case 'd':
                options.device_index = atoi(optarg);
                break;
            case 'w':
                options.width = atoi(optarg);
                break;
            case 'h':
                options.height = atoi(optarg);
                break;
            case 'r':
                options.sample_rate = atoi(optarg);
                break;
            case 'i':
                options.iterations = atoi(optarg);
                break;
            case 'v':
                options.verbose = true;
                break;
            case '?':
                // getopt_long already printed an error message
                break;
            default:
                abort();
        }
    }

    return options;
}

// Helper function to get elapsed milliseconds between two timevals
uint32_t get_elapsed_ms(struct timeval *start, struct timeval *end) {
    return (end->tv_sec - start->tv_sec) * 1000 + (end->tv_usec - start->tv_usec) / 1000;
}

// Function to run DRM tests
void run_drm_tests(const cmd_options_t *options) {
    printf("\n===== Running DRM Tests =====\n\n");

    struct timeval start_time, end_time;
    
    if (!init_test_framework()) {
        fprintf(stderr, "Failed to initialize DRM test framework\n");
        return;
    }

    // Test configurations
    test_config_t argb_config = {
        .width = options->width,
        .height = options->height,
        .format = DRM_FORMAT_ARGB32,
        .modifier = DRM_MODIFIER_LINEAR,
        .compression = DRM_COMPRESSION_NONE,
        .iterations = options->iterations
    };

    test_config_t nv12_config = {
        .width = options->width,
        .height = options->height,
        .format = DRM_FORMAT_NV12,
        .modifier = DRM_MODIFIER_TILED,
        .compression = DRM_COMPRESSION_NONE,
        .iterations = options->iterations
    };

    if (options->test_name == NULL || strcmp(options->test_name, "buffer_sharing") == 0) {
        // Buffer Sharing Tests
        gettimeofday(&start_time, NULL);
        bool result = test_buffer_sharing(&argb_config);
        gettimeofday(&end_time, NULL);
        uint32_t duration_ms = get_elapsed_ms(&start_time, &end_time);
        print_test_result("Buffer Sharing (ARGB)", result);
        
        // Add to report with duration
        if (g_report) {
            report_add_test_result(g_report, "Buffer Sharing (ARGB)", REPORT_SUBSYSTEM_DRM, 
                                result ? TEST_RESULT_PASS : TEST_RESULT_FAIL, 
                                duration_ms, result ? "Test passed" : "Test failed");
        }
        
        gettimeofday(&start_time, NULL);
        result = test_buffer_sharing(&nv12_config);
        gettimeofday(&end_time, NULL);
        duration_ms = get_elapsed_ms(&start_time, &end_time);
        print_test_result("Buffer Sharing (NV12)", result);
        
        // Add to report with duration
        if (g_report) {
            report_add_test_result(g_report, "Buffer Sharing (NV12)", REPORT_SUBSYSTEM_DRM, 
                                result ? TEST_RESULT_PASS : TEST_RESULT_FAIL, 
                                duration_ms, result ? "Test passed" : "Test failed");
        }
    }

    if (options->test_name == NULL || strcmp(options->test_name, "format_conversion") == 0) {
        // Format Conversion Tests
        print_test_result("Format Conversion (ARGB -> XRGB8888)", test_format_conversion(&argb_config, &argb_config));
        print_test_result("Format Conversion (ARGB -> YUV422)", test_format_conversion(&argb_config, &nv12_config));
    }

    if (options->test_name == NULL || strcmp(options->test_name, "performance") == 0) {
        // Performance Tests
        uint32_t avg_time;
        if (test_buffer_performance(&argb_config, &avg_time)) {
            print_performance_metrics("Buffer Sharing", avg_time);
        }
    }

    if (options->test_name == NULL || strcmp(options->test_name, "plane_config") == 0) {
        // Plane Configuration Tests
        drm_plane_t plane = {
            .id = 0,
            .type = DRM_PLANE_TYPE_PRIMARY,
            .possible_crtcs = 1
        };
        print_test_result("Primary Plane Configuration", test_plane_configuration(&plane, &argb_config));

        plane.type = DRM_PLANE_TYPE_OVERLAY;
        print_test_result("Overlay Plane Configuration", test_plane_configuration(&plane, &argb_config));

        plane.type = DRM_PLANE_TYPE_CURSOR;
        print_test_result("Cursor Plane Configuration", test_plane_configuration(&plane, &argb_config));
    }

    if (options->test_name == NULL || strcmp(options->test_name, "crtc") == 0) {
        // CRTC Tests
        drm_crtc_t crtc = {
            .id = 0,
            .x = 0,
            .y = 0,
            .width = options->width,
            .height = options->height,
            .mode = 0,
            .refresh_rate = 60
        };
        print_test_result("CRTC Configuration", test_crtc_configuration(&crtc, &argb_config));
    }

    if (options->test_name == NULL || strcmp(options->test_name, "connector") == 0) {
        // Connector Tests
        drm_connector_t connector = {
            .id = 0,
            .type = DRM_MODE_CONNECTOR_DPI,
            .connection = DRM_MODE_CONNECTED,
            .width_mm = 300,
            .height_mm = 200
        };
        print_test_result("Connector Properties", test_connector_properties(&connector));
    }

    if (options->test_name == NULL || strcmp(options->test_name, "mode") == 0) {
        // Mode Setting Tests
        drm_crtc_t crtc = {
            .id = 0,
            .x = 0,
            .y = 0,
            .width = options->width,
            .height = options->height,
            .mode = 0,
            .refresh_rate = 60
        };
        print_test_result("Mode Setting", test_mode_setting((drm_mode_t *)&crtc));
    }

    if (options->test_name == NULL || strcmp(options->test_name, "vblank") == 0) {
        // VBLANK Tests
        print_test_result("VBLANK Handling", test_vblank_handling());
    }

    if (options->test_name == NULL || strcmp(options->test_name, "sync") == 0) {
        // Sync Tests
        print_test_result("Sync Primitives", test_sync_primitives());
    }

    if (options->test_name == NULL || strcmp(options->test_name, "color") == 0) {
        // Color Management Tests
        print_test_result("Color Management", test_color_management());
    }

    if (options->test_name == NULL || strcmp(options->test_name, "cross_device") == 0) {
        // Cross-Device Tests
        print_test_result("Cross-Device Sharing", test_cross_device_sharing(&argb_config));
    }

    if (options->test_name == NULL || strcmp(options->test_name, "all") == 0) {
        // Comprehensive Test
        print_test_result("All DRM Features", test_all_features());
    }

    // Cleanup
    cleanup_test_framework();
}

// Function to run audio tests
void run_audio_tests(const cmd_options_t *options) {
    printf("\n===== Running Audio Tests =====\n\n");

    if (!init_audio_test_framework()) {
        fprintf(stderr, "Failed to initialize audio test framework\n");
        return;
    }

    // Test configuration
    audio_test_config_t audio_config = {
        .sample_rate = options->sample_rate,
        .format = AUDIO_FORMAT_PCM_S16LE,
        .channels = AUDIO_CHANNEL_STEREO,
        .buffer_size = 1024,
        .iterations = options->iterations,
        .timeout = 5000
    };

    // Get device count
    uint32_t device_count = get_audio_device_count(AUDIO_DEVICE_BOTH);
    printf("Found %u audio devices\n", device_count);

    if (device_count == 0) {
        fprintf(stderr, "No audio devices found\n");
        cleanup_audio_test_framework();
        return;
    }

    // Check device index
    if (options->device_index >= device_count) {
        fprintf(stderr, "Invalid device index: %u (max %u)\n", options->device_index, device_count - 1);
        cleanup_audio_test_framework();
        return;
    }

    // Get device info
    audio_device_info_t device_info;
    if (!get_audio_device_info(options->device_index, &device_info)) {
        fprintf(stderr, "Failed to get device info for device %u\n", options->device_index);
        cleanup_audio_test_framework();
        return;
    }

    printf("Testing device: %s\n", device_info.name);
    printf("Device type: %s\n", audio_device_type_to_string(device_info.type));

    if (options->test_name == NULL || strcmp(options->test_name, "playback") == 0) {
        // Playback Tests
        if (device_info.type == AUDIO_DEVICE_PLAYBACK || device_info.type == AUDIO_DEVICE_BOTH) {
            print_test_result("Audio Playback", test_audio_playback(options->device_index, &audio_config));
        } else {
            printf("Skipping playback test (device does not support playback)\n");
        }
    }

    if (options->test_name == NULL || strcmp(options->test_name, "capture") == 0) {
        // Capture Tests
        if (device_info.type == AUDIO_DEVICE_CAPTURE || device_info.type == AUDIO_DEVICE_BOTH) {
            print_test_result("Audio Capture", test_audio_capture(options->device_index, &audio_config));
        } else {
            printf("Skipping capture test (device does not support capture)\n");
        }
    }

    if (options->test_name == NULL || strcmp(options->test_name, "format") == 0) {
        // Format Support Tests
        print_test_result("Audio Format Support", test_audio_format_support(options->device_index, &audio_config));
    }

    if (options->test_name == NULL || strcmp(options->test_name, "latency") == 0) {
        // Latency Tests
        uint32_t latency_ms;
        if (test_audio_latency(options->device_index, &audio_config, &latency_ms)) {
            print_audio_metrics("Audio", latency_ms);
        } else {
            printf("Latency test failed\n");
        }
    }

    if (options->test_name == NULL || strcmp(options->test_name, "all") == 0) {
        // Comprehensive Test
        print_test_result("All Audio Features", test_all_audio_features(options->device_index, &audio_config));
    }

// Function to run video tests
void run_video_tests(const cmd_options_t *options)
{
    if (!options) return;
    
    printf("\n=== Starting Video Tests ===\n");
    
    // Initialize video test
    if (!tizen_video_test_init()) {
        fprintf(stderr, "Failed to initialize video test\n");
        return;
    }
    
    // Run tests based on options
    if (options->test_name == NULL || strcmp(options->test_name, "all") == 0 ||
        strcmp(options->test_name, "capture") == 0) {
        printf("\n[TEST] Video Capture...");
        bool result = tizen_video_test_capture(options->width, options->height, options->iterations);
        print_test_result("Video Capture Test", result);
    }
    
    // Cleanup
    tizen_video_test_cleanup();
}

// Function to run USB tests
void run_usb_tests(const cmd_options_t *options)
{
    if (!options) return;
    
    printf("\n=== Starting USB Tests ===\n");
    
    // Initialize USB test framework
    if (!usb_test_init()) {
        fprintf(stderr, "Failed to initialize USB test framework\n");
        return;
    }
    
    // Configure USB tests based on command line options
    usb_test_config_t config = {
        // Enable all test categories by default unless a specific class is specified
        .run_mass_storage_tests = (options->usb_test_device_class == NULL || 
                                 strcmp(options->usb_test_device_class, "msc") == 0),
        .run_hid_tests = (options->usb_test_device_class == NULL || 
                        strcmp(options->usb_test_device_class, "hid") == 0),
        .run_audio_tests = (options->usb_test_device_class == NULL || 
                          strcmp(options->usb_test_device_class, "audio") == 0),
        .run_wireless_tests = (options->usb_test_device_class == NULL || 
                             strcmp(options->usb_test_device_class, "wireless") == 0),
        .test_device_path = options->usb_device_path,
        .vendor_id = options->usb_vendor_id,
        .product_id = options->usb_product_id
    };
    
    if (options->verbose) {
        printf("USB Test Configuration:\n");
        printf("  Device Path: %s\n", config.test_device_path);
        if (options->usb_test_device_class) {
            printf("  Test Class: %s\n", options->usb_test_device_class);
        }
        if (options->usb_vendor_id) {
            printf("  Vendor ID: 0x%04x\n", options->usb_vendor_id);
        }
        if (options->usb_product_id) {
            printf("  Product ID: 0x%04x\n", options->usb_product_id);
        }
    }
    
    // Run USB tests
    int failed_tests = usb_test_run_all(&config);
    printf("\n=== USB Tests Completed: %d tests failed ===\n", failed_tests);
    
    // Cleanup
    usb_test_cleanup();
}

int main(int argc, char *argv[]) {
    // Parse command line options
    cmd_options_t options = parse_options(argc, argv);
    
    // Handle help option
    if (options.help) {
        print_usage(argv[0]);
        return 0;
    }
    
    // Initialize test report if enabled
    if (!options.no_report) {
        char title[256];
        snprintf(title, sizeof(title), "Tizen Vendor Test Suite - %s", 
                get_subsystem_name(options.subsystem));
                
        report_config_t report_config = {
            .format = options.report_format,
            .filename = options.report_file,
            .append = options.report_append
        };
        
        g_report = report_create(title, "Automated test execution results", &report_config);
    }
    
    // Run tests based on subsystem
    switch (options.subsystem) {
        case SUBSYSTEM_DRM:
            run_drm_tests(&options);
            break;
            
        case SUBSYSTEM_AUDIO:
            run_audio_tests(&options);
            break;
            
        case SUBSYSTEM_VIDEO:
            run_video_tests(&options);
            break;
            
        case SUBSYSTEM_USB:
            run_usb_tests(&options);
            break;
            
        case SUBSYSTEM_ALL:
            run_drm_tests(&options);
            run_audio_tests(&options);
            run_video_tests(&options);
            run_usb_tests(&options);
            break;
            
        default:
            fprintf(stderr, "Unknown subsystem\n");
            return 1;
    }

    printf("\nTests completed\n");

    // Generate the final report
    if (g_report) {
        if (report_generate(g_report)) {
            printf("Test report generated: %s\n", g_report->config.report_file);
            
            // Generate summary report
            if (report_generate_summary(g_report)) {
                printf("Test summary generated: %s.summary\n", g_report->config.report_file);
            }
            
            // Print summary to console
            report_print_summary(g_report, stdout);
        } else {
            fprintf(stderr, "Failed to generate test report\n");
        }
        
        report_destroy(g_report);
        g_report = NULL;
    }

    return 0;
}
