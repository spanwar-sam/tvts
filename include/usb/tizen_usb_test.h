/**
 * @file tizen_usb_test.h
 * @brief USB Test Framework for Tizen
 * @author Sumit Panwar
 * @date 2025-05-19
 * 
 * @copyright Copyright (c) 2025 Sumit Panwar
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef TIZEN_USB_TEST_H
#define TIZEN_USB_TEST_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief USB Device Classes
 */
typedef enum {
    USB_CLASS_MASS_STORAGE = 0x08,
    USB_CLASS_HID = 0x03,         // Keyboards, mice, etc.
    USB_CLASS_AUDIO = 0x01,       // Headsets, speakers
    USB_CLASS_WIRELESS = 0xE0,     // Bluetooth, WiFi adapters
    USB_CLASS_VENDOR_SPEC = 0xFF   // Vendor-specific devices
} usb_device_class_t;

/**
 * @brief USB Test Results
 */
typedef enum {
    USB_TEST_PASSED,
    USB_TEST_FAILED,
    USB_TEST_SKIPPED,
    USB_TEST_ERROR
} usb_test_result_t;

/**
 * @brief USB Test Configuration
 */
typedef struct {
    // Test categories to run
    bool run_mass_storage_tests;
    bool run_hid_tests;
    bool run_audio_tests;
    bool run_wireless_tests;
    
    // Device identification
    const char *test_device_path;  // Path to USB device (e.g., /dev/bus/usb/...)
    uint16_t vendor_id;           // USB vendor ID (0 for any)
    uint16_t product_id;          // USB product ID (0 for any)
} usb_test_config_t;

/**
 * @brief Initialize USB test framework
 * @return true if initialization succeeded, false otherwise
 */
bool usb_test_init(void);

/**
 * @brief Clean up USB test framework
 */
void usb_test_cleanup(void);

/**
 * @brief Run all USB tests
 * @param config Test configuration
 * @return Number of failed tests
 */
int usb_test_run_all(const usb_test_config_t *config);

/**
 * @brief Test USB mass storage functionality
 * @param device_path Path to USB mass storage device
 * @return Test result
 */
usb_test_result_t test_usb_mass_storage(const char *device_path);

/**
 * @brief Test HID devices (keyboards, mice, etc.)
 * @param device_path Path to HID device
 * @return Test result
 */
usb_test_result_t test_usb_hid(const char *device_path);

/**
 * @brief Test USB audio devices (headsets, speakers)
 * @param device_path Path to USB audio device
 * @return Test result
 */
usb_test_result_t test_usb_audio(const char *device_path);

/**
 * @brief Test USB wireless devices (Bluetooth, WiFi)
 * @param device_path Path to USB wireless device
 * @return Test result
 */
usb_test_result_t test_usb_wireless(const char *device_path);

#ifdef __cplusplus
}
#endif

#endif /* TIZEN_USB_TEST_H */
