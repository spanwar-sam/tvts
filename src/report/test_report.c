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

#include "report/test_report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

// Helper function implementations
const char *report_format_to_string(report_format_t format) {
    switch (format) {
        case REPORT_FORMAT_TEXT: return "TEXT";
        case REPORT_FORMAT_JSON: return "JSON";
        case REPORT_FORMAT_HTML: return "HTML";
        case REPORT_FORMAT_XML: return "XML";
        case REPORT_FORMAT_CSV: return "CSV";
        default: return "UNKNOWN";
    }
}

const char *test_result_to_string(test_result_t result) {
    switch (result) {
        case TEST_RESULT_PASS: return "PASS";
        case TEST_RESULT_FAIL: return "FAIL";
        case TEST_RESULT_SKIP: return "SKIP";
        case TEST_RESULT_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

const char *report_level_to_string(report_level_t level) {
    switch (level) {
        case REPORT_LEVEL_INFO: return "INFO";
        case REPORT_LEVEL_WARNING: return "WARNING";
        case REPORT_LEVEL_ERROR: return "ERROR";
        case REPORT_LEVEL_DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

const char *report_subsystem_to_string(report_subsystem_t subsystem) {
    switch (subsystem) {
        case REPORT_SUBSYSTEM_DRM: return "DRM";
        case REPORT_SUBSYSTEM_AUDIO: return "AUDIO";
        case REPORT_SUBSYSTEM_VIDEO: return "VIDEO";
        case REPORT_SUBSYSTEM_OTHER: return "OTHER";
        default: return "UNKNOWN";
    }
}

const char *metric_type_to_string(metric_type_t type) {
    switch (type) {
        case METRIC_TIME_US: return "TIME_US";
        case METRIC_THROUGHPUT: return "THROUGHPUT";
        case METRIC_LATENCY_MS: return "LATENCY_MS";
        case METRIC_FRAME_RATE: return "FRAME_RATE";
        default: return "UNKNOWN";
    }
}

// Report creation and destruction
test_report_t *report_create(const char *title, const char *description, const report_config_t *config) {
    test_report_t *report = (test_report_t *)malloc(sizeof(test_report_t));
    if (!report) {
        return NULL;
    }
    
    memset(report, 0, sizeof(test_report_t));
    
    if (title) {
        strncpy(report->title, title, sizeof(report->title) - 1);
    } else {
        strcpy(report->title, "Vendor Test Suite Report");
    }
    
    if (description) {
        strncpy(report->description, description, sizeof(report->description) - 1);
    } else {
        strcpy(report->description, "Automated test results");
    }
    
    if (config) {
        memcpy(&report->config, config, sizeof(report_config_t));
    } else {
        // Default configuration
        strcpy(report->config.report_file, "test_report.txt");
        report->config.format = REPORT_FORMAT_TEXT;
        report->config.append = false;
        report->config.include_timestamp = true;
        report->config.include_system_info = true;
        report->config.include_performance_metrics = true;
        report->config.min_level = REPORT_LEVEL_INFO;
    }
    
    report->start_time = time(NULL);
    report->test_results = NULL;
    report->perf_metrics = NULL;
    
    // Open report file
    const char *mode = report->config.append ? "a" : "w";
    report->report_file = fopen(report->config.report_file, mode);
    if (!report->report_file) {
        free(report);
        return NULL;
    }
    
    return report;
}

void report_destroy(test_report_t *report) {
    if (!report) {
        return;
    }
    
    // Close report file
    if (report->report_file) {
        fclose(report->report_file);
        report->report_file = NULL;
    }
    
    // Free test result entries
    test_result_entry_t *result_entry = report->test_results;
    while (result_entry) {
        test_result_entry_t *next = result_entry->next;
        free(result_entry);
        result_entry = next;
    }
    
    // Free performance metric entries
    perf_metric_entry_t *metric_entry = report->perf_metrics;
    while (metric_entry) {
        perf_metric_entry_t *next = metric_entry->next;
        free(metric_entry);
        metric_entry = next;
    }
    
    free(report);
}

// Test result reporting
void report_add_test_result(test_report_t *report, const char *test_name, report_subsystem_t subsystem, 
                            test_result_t result, uint32_t duration_ms, const char *message) {
    if (!report || !test_name) {
        return;
    }
    
    // Update test counters
    report->total_tests++;
    switch (result) {
        case TEST_RESULT_PASS:
            report->passed_tests++;
            break;
        case TEST_RESULT_FAIL:
            report->failed_tests++;
            break;
        case TEST_RESULT_SKIP:
            report->skipped_tests++;
            break;
        case TEST_RESULT_ERROR:
            report->error_tests++;
            break;
        default:
            break;
    }
    
    // Create new test result entry
    test_result_entry_t *entry = (test_result_entry_t *)malloc(sizeof(test_result_entry_t));
    if (!entry) {
        return;
    }
    
    memset(entry, 0, sizeof(test_result_entry_t));
    strncpy(entry->test_name, test_name, sizeof(entry->test_name) - 1);
    entry->subsystem = subsystem;
    entry->result = result;
    entry->duration_ms = duration_ms;
    if (message) {
        strncpy(entry->message, message, sizeof(entry->message) - 1);
    }
    entry->timestamp = time(NULL);
    entry->next = NULL;
    
    // Add to linked list
    if (!report->test_results) {
        report->test_results = entry;
    } else {
        test_result_entry_t *tail = report->test_results;
        while (tail->next) {
            tail = tail->next;
        }
        tail->next = entry;
    }
    
    // Write to report file directly if it's text format
    if (report->config.format == REPORT_FORMAT_TEXT && report->report_file) {
        fprintf(report->report_file, "[%s] %s: %s (%u ms) - %s\n",
                report_subsystem_to_string(subsystem),
                test_name,
                test_result_to_string(result),
                duration_ms,
                message ? message : "");
        fflush(report->report_file);
    }
}

// Generate HTML report
static bool generate_html_report(test_report_t *report) {
    if (!report || !report->report_file) {
        return false;
    }
    
    FILE *f = report->report_file;
    
    // HTML header
    fprintf(f, "<!DOCTYPE html>\n");
    fprintf(f, "<html lang=\"en\">\n");
    fprintf(f, "<head>\n");
    fprintf(f, "  <meta charset=\"UTF-8\">\n");
    fprintf(f, "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    fprintf(f, "  <title>%s</title>\n", report->title);
    fprintf(f, "  <style>\n");
    fprintf(f, "    body { font-family: Arial, sans-serif; margin: 0; padding: 20px; }\n");
    fprintf(f, "    .header { background-color: #f4f4f4; padding: 20px; border-radius: 5px; }\n");
    fprintf(f, "    .summary { display: flex; margin: 20px 0; }\n");
    fprintf(f, "    .summary-item { padding: 10px; margin-right: 10px; border-radius: 5px; flex: 1; }\n");
    fprintf(f, "    .pass { background-color: #dff0d8; }\n");
    fprintf(f, "    .fail { background-color: #f2dede; }\n");
    fprintf(f, "    .skip { background-color: #fcf8e3; }\n");
    fprintf(f, "    .error { background-color: #f2dede; }\n");
    fprintf(f, "    table { width: 100%%; border-collapse: collapse; }\n");
    fprintf(f, "    th, td { padding: 10px; text-align: left; border-bottom: 1px solid #ddd; }\n");
    fprintf(f, "    th { background-color: #f4f4f4; }\n");
    fprintf(f, "    tr.pass { background-color: #dff0d8; }\n");
    fprintf(f, "    tr.fail { background-color: #f2dede; }\n");
    fprintf(f, "    tr.skip { background-color: #fcf8e3; }\n");
    fprintf(f, "    tr.error { background-color: #f2dede; }\n");
    fprintf(f, "  </style>\n");
    fprintf(f, "</head>\n");
    fprintf(f, "<body>\n");
    
    // Header
    fprintf(f, "  <div class=\"header\">\n");
    fprintf(f, "    <h1>%s</h1>\n", report->title);
    fprintf(f, "    <p>%s</p>\n", report->description);
    
    // Timestamp
    if (report->config.include_timestamp) {
        char start_time_str[64];
        char end_time_str[64];
        strftime(start_time_str, sizeof(start_time_str), "%Y-%m-%d %H:%M:%S", localtime(&report->start_time));
        strftime(end_time_str, sizeof(end_time_str), "%Y-%m-%d %H:%M:%S", localtime(&report->end_time));
        fprintf(f, "    <p>Start Time: %s</p>\n", start_time_str);
        fprintf(f, "    <p>End Time: %s</p>\n", end_time_str);
    }
    fprintf(f, "  </div>\n");
    
    // Summary
    fprintf(f, "  <div class=\"summary\">\n");
    fprintf(f, "    <div class=\"summary-item pass\">\n");
    fprintf(f, "      <h2>Passed</h2>\n");
    fprintf(f, "      <p>%u</p>\n", report->passed_tests);
    fprintf(f, "    </div>\n");
    fprintf(f, "    <div class=\"summary-item fail\">\n");
    fprintf(f, "      <h2>Failed</h2>\n");
    fprintf(f, "      <p>%u</p>\n", report->failed_tests);
    fprintf(f, "    </div>\n");
    fprintf(f, "    <div class=\"summary-item skip\">\n");
    fprintf(f, "      <h2>Skipped</h2>\n");
    fprintf(f, "      <p>%u</p>\n", report->skipped_tests);
    fprintf(f, "    </div>\n");
    fprintf(f, "    <div class=\"summary-item error\">\n");
    fprintf(f, "      <h2>Errors</h2>\n");
    fprintf(f, "      <p>%u</p>\n", report->error_tests);
    fprintf(f, "    </div>\n");
    fprintf(f, "    <div class=\"summary-item\">\n");
    fprintf(f, "      <h2>Total</h2>\n");
    fprintf(f, "      <p>%u</p>\n", report->total_tests);
    fprintf(f, "    </div>\n");
    fprintf(f, "  </div>\n");
    
    // Test results table
    fprintf(f, "  <h2>Test Results</h2>\n");
    fprintf(f, "  <table>\n");
    fprintf(f, "    <tr>\n");
    fprintf(f, "      <th>Subsystem</th>\n");
    fprintf(f, "      <th>Test Name</th>\n");
    fprintf(f, "      <th>Result</th>\n");
    fprintf(f, "      <th>Duration (ms)</th>\n");
    fprintf(f, "      <th>Message</th>\n");
    if (report->config.include_timestamp) {
        fprintf(f, "      <th>Timestamp</th>\n");
    }
    fprintf(f, "    </tr>\n");
    
    // Add test results
    test_result_entry_t *entry = report->test_results;
    while (entry) {
        char result_class[10];
        switch (entry->result) {
            case TEST_RESULT_PASS: strcpy(result_class, "pass"); break;
            case TEST_RESULT_FAIL: strcpy(result_class, "fail"); break;
            case TEST_RESULT_SKIP: strcpy(result_class, "skip"); break;
            case TEST_RESULT_ERROR: strcpy(result_class, "error"); break;
            default: strcpy(result_class, ""); break;
        }
        
        fprintf(f, "    <tr class=\"%s\">\n", result_class);
        fprintf(f, "      <td>%s</td>\n", report_subsystem_to_string(entry->subsystem));
        fprintf(f, "      <td>%s</td>\n", entry->test_name);
        fprintf(f, "      <td>%s</td>\n", test_result_to_string(entry->result));
        fprintf(f, "      <td>%u</td>\n", entry->duration_ms);
        fprintf(f, "      <td>%s</td>\n", entry->message);
        
        if (report->config.include_timestamp) {
            char time_str[64];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&entry->timestamp));
            fprintf(f, "      <td>%s</td>\n", time_str);
        }
        
        fprintf(f, "    </tr>\n");
        entry = entry->next;
    }
    
    fprintf(f, "  </table>\n");
    
    // Performance metrics
    if (report->config.include_performance_metrics && report->perf_metrics) {
        fprintf(f, "  <h2>Performance Metrics</h2>\n");
        fprintf(f, "  <table>\n");
        fprintf(f, "    <tr>\n");
        fprintf(f, "      <th>Metric</th>\n");
        fprintf(f, "      <th>Type</th>\n");
        fprintf(f, "      <th>Value</th>\n");
        fprintf(f, "      <th>Units</th>\n");
        fprintf(f, "    </tr>\n");
        
        perf_metric_entry_t *metric = report->perf_metrics;
        while (metric) {
            fprintf(f, "    <tr>\n");
            fprintf(f, "      <td>%s</td>\n", metric->metric_name);
            fprintf(f, "      <td>%s</td>\n", metric_type_to_string(metric->type));
            fprintf(f, "      <td>%.2f</td>\n", metric->value);
            fprintf(f, "      <td>%s</td>\n", metric->units);
            fprintf(f, "    </tr>\n");
            
            metric = metric->next;
        }
        
        fprintf(f, "  </table>\n");
    }
    
    // Footer
    fprintf(f, "  <div class=\"footer\">\n");
    fprintf(f, "    <p>Generated by Tizen Vendor Test Suite</p>\n");
    fprintf(f, "  </div>\n");
    fprintf(f, "</body>\n");
    fprintf(f, "</html>\n");
    
    return true;
}

// Performance metric reporting
void report_add_metric(test_report_t *report, const char *metric_name, metric_type_t type, 
                      double value, const char *units) {
    if (!report || !metric_name) {
        return;
    }
    
    // Create new performance metric entry
    perf_metric_entry_t *entry = (perf_metric_entry_t *)malloc(sizeof(perf_metric_entry_t));
    if (!entry) {
        return;
    }
    
    memset(entry, 0, sizeof(perf_metric_entry_t));
    strncpy(entry->metric_name, metric_name, sizeof(entry->metric_name) - 1);
    entry->type = type;
    entry->value = value;
    if (units) {
        strncpy(entry->units, units, sizeof(entry->units) - 1);
    } else {
        switch (type) {
            case METRIC_TIME_US:
                strcpy(entry->units, "µs");
                break;
            case METRIC_THROUGHPUT:
                strcpy(entry->units, "B/s");
                break;
            case METRIC_LATENCY_MS:
                strcpy(entry->units, "ms");
                break;
            case METRIC_FRAME_RATE:
                strcpy(entry->units, "fps");
                break;
            default:
                strcpy(entry->units, "");
                break;
        }
    }
    entry->next = NULL;
    
    // Add to linked list
    if (!report->perf_metrics) {
        report->perf_metrics = entry;
    } else {
        perf_metric_entry_t *tail = report->perf_metrics;
        while (tail->next) {
            tail = tail->next;
        }
        tail->next = entry;
    }
    
    // Write to report file directly if it's text format
    if (report->config.format == REPORT_FORMAT_TEXT && report->report_file) {
        fprintf(report->report_file, "METRIC: %s = %.2f %s\n",
                metric_name, value, entry->units);
        fflush(report->report_file);
    }
}

// Convenience functions for specific metric types
void report_add_time_metric(test_report_t *report, const char *metric_name, double microseconds) {
    report_add_metric(report, metric_name, METRIC_TIME_US, microseconds, "µs");
}

void report_add_throughput_metric(test_report_t *report, const char *metric_name, double bytes_per_sec) {
    report_add_metric(report, metric_name, METRIC_THROUGHPUT, bytes_per_sec, "B/s");
}

void report_add_latency_metric(test_report_t *report, const char *metric_name, double milliseconds) {
    report_add_metric(report, metric_name, METRIC_LATENCY_MS, milliseconds, "ms");
}

void report_add_frame_rate_metric(test_report_t *report, const char *metric_name, double fps) {
    report_add_metric(report, metric_name, METRIC_FRAME_RATE, fps, "fps");
}

// Report generation
bool report_generate(test_report_t *report) {
    if (!report) {
        return false;
    }
    
    // Update end time
    report->end_time = time(NULL);
    
    // Generate report based on format
    bool result = false;
    
    // Close and reopen file if needed
    if (report->report_file) {
        fclose(report->report_file);
        report->report_file = NULL;
    }
    
    const char *mode = "w";
    report->report_file = fopen(report->config.report_file, mode);
    if (!report->report_file) {
        return false;
    }
    
    switch (report->config.format) {
        case REPORT_FORMAT_TEXT:
            // Simple text format
            fprintf(report->report_file, "===== %s =====\n", report->title);
            fprintf(report->report_file, "%s\n\n", report->description);
            
            // Summary
            fprintf(report->report_file, "--- Summary ---\n");
            fprintf(report->report_file, "Total Tests: %u\n", report->total_tests);
            fprintf(report->report_file, "Passed Tests: %u\n", report->passed_tests);
            fprintf(report->report_file, "Failed Tests: %u\n", report->failed_tests);
            fprintf(report->report_file, "Skipped Tests: %u\n", report->skipped_tests);
            fprintf(report->report_file, "Error Tests: %u\n\n", report->error_tests);
            
            // Test results
            fprintf(report->report_file, "--- Test Results ---\n");
            test_result_entry_t *entry = report->test_results;
            while (entry) {
                fprintf(report->report_file, "[%s] %s: %s (%u ms) - %s\n",
                        report_subsystem_to_string(entry->subsystem),
                        entry->test_name,
                        test_result_to_string(entry->result),
                        entry->duration_ms,
                        entry->message);
                entry = entry->next;
            }
            
            // Performance metrics
            if (report->config.include_performance_metrics && report->perf_metrics) {
                fprintf(report->report_file, "\n--- Performance Metrics ---\n");
                perf_metric_entry_t *metric = report->perf_metrics;
                while (metric) {
                    fprintf(report->report_file, "%s = %.2f %s\n",
                            metric->metric_name, metric->value, metric->units);
                    metric = metric->next;
                }
            }
            
            result = true;
            break;
            
        case REPORT_FORMAT_JSON:
            // TODO: Implement JSON format
            break;
            
        case REPORT_FORMAT_HTML:
            result = generate_html_report(report);
            break;
            
        case REPORT_FORMAT_XML:
            // TODO: Implement XML format
            break;
            
        case REPORT_FORMAT_CSV:
            // TODO: Implement CSV format
            break;
            
        default:
            break;
    }
    
    return result;
}

// Print summary to specified output
void report_print_summary(test_report_t *report, FILE *output) {
    if (!report || !output) {
        return;
    }
    
    fprintf(output, "===== Test Summary =====\n");
    fprintf(output, "Total Tests: %u\n", report->total_tests);
    fprintf(output, "Passed Tests: %u (%.1f%%)\n", 
            report->passed_tests, 
            report->total_tests > 0 ? (report->passed_tests * 100.0) / report->total_tests : 0.0);
    fprintf(output, "Failed Tests: %u (%.1f%%)\n", 
            report->failed_tests, 
            report->total_tests > 0 ? (report->failed_tests * 100.0) / report->total_tests : 0.0);
    fprintf(output, "Skipped Tests: %u\n", report->skipped_tests);
    fprintf(output, "Error Tests: %u\n", report->error_tests);
    
    if (report->failed_tests > 0 || report->error_tests > 0) {
        fprintf(output, "\n--- Failed Tests ---\n");
        test_result_entry_t *entry = report->test_results;
        while (entry) {
            if (entry->result == TEST_RESULT_FAIL || entry->result == TEST_RESULT_ERROR) {
                fprintf(output, "[%s] %s: %s - %s\n",
                        report_subsystem_to_string(entry->subsystem),
                        entry->test_name,
                        test_result_to_string(entry->result),
                        entry->message);
            }
            entry = entry->next;
        }
    }
}

// Generate summary report
bool report_generate_summary(test_report_t *report) {
    if (!report) {
        return false;
    }
    
    // Create summary file
    char summary_file[260];
    snprintf(summary_file, sizeof(summary_file), "%s.summary", report->config.report_file);
    
    FILE *f = fopen(summary_file, "w");
    if (!f) {
        return false;
    }
    
    report_print_summary(report, f);
    
    fclose(f);
    return true;
}
