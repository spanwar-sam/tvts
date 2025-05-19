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

#ifndef TEST_REPORT_H
#define TEST_REPORT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

// Report format types
typedef enum {
    REPORT_FORMAT_TEXT,       // Plain text format
    REPORT_FORMAT_JSON,       // JSON format
    REPORT_FORMAT_HTML,       // HTML format
    REPORT_FORMAT_XML,        // XML format
    REPORT_FORMAT_CSV,        // CSV format
    REPORT_FORMAT_MAX
} report_format_t;

// Test result types
typedef enum {
    TEST_RESULT_PASS,         // Test passed
    TEST_RESULT_FAIL,         // Test failed
    TEST_RESULT_SKIP,         // Test skipped
    TEST_RESULT_ERROR,        // Test error
    TEST_RESULT_MAX
} test_result_t;

// Test report severity levels
typedef enum {
    REPORT_LEVEL_INFO,        // Informational message
    REPORT_LEVEL_WARNING,     // Warning message
    REPORT_LEVEL_ERROR,       // Error message
    REPORT_LEVEL_DEBUG,       // Debug message
    REPORT_LEVEL_MAX
} report_level_t;

// Subsystem types
typedef enum {
    REPORT_SUBSYSTEM_DRM,     // DRM subsystem
    REPORT_SUBSYSTEM_AUDIO,   // Audio subsystem
    REPORT_SUBSYSTEM_VIDEO,   // Video subsystem
    REPORT_SUBSYSTEM_OTHER,   // Other subsystem
    REPORT_SUBSYSTEM_MAX
} report_subsystem_t;

// Performance metric types
typedef enum {
    METRIC_TIME_US,           // Time in microseconds
    METRIC_THROUGHPUT,        // Throughput in bytes/sec
    METRIC_LATENCY_MS,        // Latency in milliseconds
    METRIC_FRAME_RATE,        // Frame rate in FPS
    METRIC_MAX
} metric_type_t;

// Test result entry structure
typedef struct test_result_entry {
    char test_name[128];                  // Test name
    report_subsystem_t subsystem;         // Subsystem
    test_result_t result;                 // Test result
    uint32_t duration_ms;                 // Test duration in milliseconds
    char message[256];                    // Additional message
    time_t timestamp;                     // Test timestamp
    struct test_result_entry *next;       // Next entry in linked list
} test_result_entry_t;

// Performance metric entry structure
typedef struct perf_metric_entry {
    char metric_name[128];                // Metric name
    metric_type_t type;                   // Metric type
    double value;                         // Metric value
    char units[32];                       // Metric units
    struct perf_metric_entry *next;       // Next entry in linked list
} perf_metric_entry_t;

// Test report configuration structure
typedef struct {
    char report_file[256];                // Report file path
    report_format_t format;               // Report format
    bool append;                          // Append to existing file
    bool include_timestamp;               // Include timestamp in report
    bool include_system_info;             // Include system information
    bool include_performance_metrics;     // Include performance metrics
    report_level_t min_level;             // Minimum log level to include
} report_config_t;

// Test report structure
typedef struct {
    report_config_t config;               // Report configuration
    char title[128];                      // Report title
    char description[256];                // Report description
    time_t start_time;                    // Test start time
    time_t end_time;                      // Test end time
    uint32_t total_tests;                 // Total number of tests
    uint32_t passed_tests;                // Number of passed tests
    uint32_t failed_tests;                // Number of failed tests
    uint32_t skipped_tests;               // Number of skipped tests
    uint32_t error_tests;                 // Number of test errors
    test_result_entry_t *test_results;    // Test result entries
    perf_metric_entry_t *perf_metrics;    // Performance metric entries
    FILE *report_file;                    // Report file handle
} test_report_t;

// Function prototypes

// Report initialization/cleanup
test_report_t *report_create(const char *title, const char *description, const report_config_t *config);
void report_destroy(test_report_t *report);

// Test result reporting
void report_add_test_result(test_report_t *report, const char *test_name, report_subsystem_t subsystem, 
                            test_result_t result, uint32_t duration_ms, const char *message);
void report_add_info(test_report_t *report, const char *format, ...);
void report_add_warning(test_report_t *report, const char *format, ...);
void report_add_error(test_report_t *report, const char *format, ...);
void report_add_debug(test_report_t *report, const char *format, ...);

// Performance metric reporting
void report_add_metric(test_report_t *report, const char *metric_name, metric_type_t type, 
                      double value, const char *units);
void report_add_time_metric(test_report_t *report, const char *metric_name, double microseconds);
void report_add_throughput_metric(test_report_t *report, const char *metric_name, double bytes_per_sec);
void report_add_latency_metric(test_report_t *report, const char *metric_name, double milliseconds);
void report_add_frame_rate_metric(test_report_t *report, const char *metric_name, double fps);

// Report generation
bool report_generate(test_report_t *report);
bool report_generate_summary(test_report_t *report);
void report_print_summary(test_report_t *report, FILE *output);

// Helper functions
const char *report_format_to_string(report_format_t format);
const char *test_result_to_string(test_result_t result);
const char *report_level_to_string(report_level_t level);
const char *report_subsystem_to_string(report_subsystem_t subsystem);
const char *metric_type_to_string(metric_type_t type);

#endif /* TEST_REPORT_H */
