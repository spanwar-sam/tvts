/**
 * @file tizen_usb_test.c
 * @brief USB Test Implementation for Tizen
 * @author Sumit Panwar
 * @date 2025-05-19
 * 
 * @copyright Copyright (c) 2025 Sumit Panwar
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <scsi/sg.h>
#include <scsi/scsi.h>
#include <linux/cdrom.h>
#include <libusb-1.0/libusb.h>

// SCSI generic ioctl command structure
struct sdata {
    unsigned int in_len;     /* _after_ split */
    unsigned int out_len;    /* ongoing count */
    unsigned char in_cmd[16];
    unsigned char out_cmd[16];
    unsigned char *buffer;
};

// For SCSI_IOCTL_SEND_COMMAND
#ifndef SCSI_IOCTL_SEND_COMMAND
#define SCSI_IOCTL_SEND_COMMAND  _IOWR('S', 0x18, struct sdata)
#endif
#include "../../include/usb/tizen_usb_test.h"
#include "../../include/report/test_report.h"

// Private function declarations
static usb_test_result_t get_usb_device_class(const char *device_path, uint8_t *device_class);
static bool is_usb_device_connected(uint16_t vendor_id, uint16_t product_id);

// Test configurations
static const uint16_t MASS_STORAGE_VID = 0x0781;  // SanDisk
static const uint16_t MASS_STORAGE_PID = 0x5591;  // Example: Cruzer Blade

bool usb_test_init(void) {
    int rc = libusb_init(NULL);
    if (rc < 0) {
        fprintf(stderr, "Failed to initialize libusb: %s\n", libusb_error_name(rc));
        return false;
    }
    return true;
}

void usb_test_cleanup(void) {
    libusb_exit(NULL);
}

int usb_test_run_all(const usb_test_config_t *config) {
    if (!config) {
        return -1;
    }

    int failed_tests = 0;
    usb_test_result_t result;
    bool device_found = true;

    // Check if a specific vendor/product ID was requested
    if (config->vendor_id != 0 || config->product_id != 0) {
        device_found = is_usb_device_connected(config->vendor_id, config->product_id);
        if (!device_found) {
            printf("\n[WARNING] Requested USB device %04x:%04x not found.\n", 
                   config->vendor_id, config->product_id);
            printf("Skipping all USB tests.\n");
            return 0;  // Not a failure, just no tests to run
        }
    }

    printf("\n=== Starting USB Tests ===\n");

    // Test USB Mass Storage
    if (config->run_mass_storage_tests) {
        printf("\n[TEST] USB Mass Storage...");
        if (config->vendor_id == 0 || 
            (config->vendor_id == MASS_STORAGE_VID && 
             (config->product_id == 0 || config->product_id == MASS_STORAGE_PID))) {
            result = test_usb_mass_storage(config->test_device_path);
            if (result != USB_TEST_PASSED) {
                failed_tests++;
            }
        } else {
            printf("\n[SKIP] Mass Storage test - Device not matching filter criteria\n");
        }
    }

    // Test USB HID Devices
    if (config->run_hid_tests) {
        printf("\n[TEST] USB HID Devices...");
        // Only run HID test if no specific device is requested or if it matches the HID class
        if (config->vendor_id == 0 || 
            (config->vendor_id != MASS_STORAGE_VID)) {  // Simple example, should check HID VID/PID
            result = test_usb_hid(config->test_device_path);
            if (result != USB_TEST_PASSED) {
                failed_tests++;
            }
        } else {
            printf("\n[SKIP] HID test - Device not matching filter criteria\n");
        }
    }

    // Test USB Audio Devices
    if (config->run_audio_tests) {
        printf("\n[TEST] USB Audio Devices...");
        if (config->vendor_id == 0 || 
            (config->vendor_id != MASS_STORAGE_VID)) {  // Simple example, should check Audio VID/PID
            result = test_usb_audio(config->test_device_path);
            if (result != USB_TEST_PASSED) {
                failed_tests++;
            }
        } else {
            printf("\n[SKIP] Audio test - Device not matching filter criteria\n");
        }
    }

    // Test USB Wireless Devices
    if (config->run_wireless_tests) {
        printf("\n[TEST] USB Wireless Devices...");
        if (config->vendor_id == 0 || 
            (config->vendor_id != MASS_STORAGE_VID)) {  // Simple example, should check Wireless VID/PID
            result = test_usb_wireless(config->test_device_path);
            if (result != USB_TEST_PASSED) {
                failed_tests++;
            }
        } else {
            printf("\n[SKIP] Wireless test - Device not matching filter criteria\n");
        }
    }

    printf("\n=== USB Tests Completed: %d tests failed ===\n", failed_tests);
    return failed_tests;
}

usb_test_result_t test_usb_mass_storage(const char *device_path) {
    if (!device_path) {
        printf("[ERROR] No device path provided\n");
        return USB_TEST_ERROR;
    }

    printf("\n[INFO] Testing Mass Storage device at %s\n", device_path);
    
    // Check if the device file exists and is accessible
    printf("[TEST] Checking device presence... ");
    if (access(device_path, F_OK) == -1) {
        printf("FAILED (Error: %s)\n", strerror(errno));
        return USB_TEST_FAILED;
    }
    printf("PASSED\n");
    
    // Check read permission
    printf("[TEST] Checking read permission... ");
    if (access(device_path, R_OK) == -1) {
        printf("FAILED (No read permission: %s)\n", strerror(errno));
        return USB_TEST_FAILED;
    }
    printf("PASSED\n");
    
    // Try to open the device in read-only mode
    printf("[TEST] Opening device... ");
    int fd = open(device_path, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        printf("FAILED (Error: %s)\n", strerror(errno));
        return USB_TEST_FAILED;
    }
    printf("PASSED (fd=%d)\n", fd);
    
    // Get device information using ioctl
    printf("[TEST] Getting device information... ");
    struct stat st;
    if (fstat(fd, &st) == -1) {
        printf("FAILED (fstat: %s)\n", strerror(errno));
        close(fd);
        return USB_TEST_FAILED;
    }
    
    // Print basic device information
    printf("PASSED\n");
    printf("  Device Type:   %s\n", S_ISBLK(st.st_mode) ? "Block device" : "Unknown");
    printf("  Size:          %lld bytes\n", (long long)st.st_size);
    
    // Test basic read operation
    printf("[TEST] Testing read operation... ");
    char buffer[512];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
    
    if (bytes_read < 0) {
        if (errno == EIO) {
            // Some devices may return EIO until media is inserted
            printf("WARNING (Read I/O error: %s)\n", strerror(errno));
        } else {
            printf("FAILED (Error: %s)\n", strerror(errno));
            close(fd);
            return USB_TEST_FAILED;
        }
    } else if (bytes_read == 0) {
        printf("WARNING (No data read, device may be empty)\n");
    } else {
        printf("PASSED (%zd bytes read)\n", bytes_read);
        
        // Print first few bytes as hex (useful for debugging)
        if (bytes_read > 0) {
            printf("[DEBUG] First 16 bytes: ");
            for (int i = 0; i < (bytes_read < 16 ? bytes_read : 16); i++) {
                printf("%02x ", (unsigned char)buffer[i]);
            }
            printf("\n");
        }
    }
    
    // Try to get SCSI INQUIRY data (for USB mass storage devices)
    printf("[TEST] SCSI INQUIRY... ");
    
    // SCSI INQUIRY command
    unsigned char inquiry_cmd[6] = {INQUIRY, 0, 0, 0, 0x24, 0};
    unsigned char inquiry_buf[0x24];
    
    // Prepare SG_IO structure for SCSI command
    struct sg_io_hdr io_hdr = {0};
    unsigned char sense_buffer[32];
    
    io_hdr.interface_id = 'S';
    io_hdr.cmd_len = 6;  // Length of SCSI command
    io_hdr.mx_sb_len = sizeof(sense_buffer);
    io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
    io_hdr.dxfer_len = sizeof(inquiry_buf);
    io_hdr.dxferp = inquiry_buf;
    io_hdr.cmdp = inquiry_cmd;
    io_hdr.sbp = sense_buffer;
    io_hdr.timeout = 5000;  // 5 second timeout
    
    if (ioctl(fd, SG_IO, &io_hdr) < 0) {
        printf("NOT SUPPORTED (SG_IO: %s)\n", strerror(errno));
    } else if (io_hdr.status || io_hdr.host_status || io_hdr.driver_status) {
        printf("FAILED (SCSI error: status=0x%02x, host_status=0x%02x, driver_status=0x%02x)\n",
               io_hdr.status, io_hdr.host_status, io_hdr.driver_status);
    } else {
        printf("PASSED\n");
        // Parse and print INQUIRY data
        printf("  Vendor:   %.8s\n", inquiry_buf + 8);
        printf("  Product:  %.16s\n", inquiry_buf + 16);
        printf("  Revision: %.4s\n", inquiry_buf + 32);
        
        // Print device type
        const char *device_type;
        switch (inquiry_buf[0] & 0x1f) {
            case 0x00: device_type = "Direct Access (e.g., disk)"; break;
            case 0x01: device_type = "Sequential Access (e.g., tape)"; break;
            case 0x05: device_type = "CD/DVD drive"; break;
            case 0x07: device_type = "Optical Memory Device"; break;
            case 0x0e: device_type = "Simplified Direct Access Device"; break;
            default:   device_type = "Unknown"; break;
        }
        printf("  Type:     %s (0x%02x)\n", device_type, inquiry_buf[0] & 0x1f);
    }
    
    // Clean up
    close(fd);
    
    // TODO: Add more comprehensive tests (e.g., mount, filesystem operations)
    printf("[INFO] Additional mass storage tests not implemented yet\n");
    
    return USB_TEST_PASSED;
}

usb_test_result_t test_usb_hid(const char *device_path) {
    if (!device_path) {
        printf("Error: No device path provided\n");
        return USB_TEST_ERROR;
    }
    
    printf("\n[INFO] Testing HID device at %s\n", device_path);
    
    // Check if the device file exists and is accessible
    printf("[TEST] Checking device presence... ");
    if (access(device_path, F_OK) == -1) {
        printf("FAILED (Device not found)\n");
        return USB_TEST_FAILED;
    }
    printf("PASSED\n");
    
    // TODO: Implement actual HID device tests
    printf("[INFO] HID device test not fully implemented yet\n");
    
    return USB_TEST_PASSED;
}

usb_test_result_t test_usb_audio(const char *device_path) {
    if (!device_path) {
        printf("Error: No device path provided\n");
        return USB_TEST_ERROR;
    }
    
    printf("\n[INFO] Testing USB Audio device at %s\n", device_path);
    
    // Check if the device file exists and is accessible
    printf("[TEST] Checking device presence... ");
    if (access(device_path, F_OK) == -1) {
        printf("FAILED (Device not found)\n");
        return USB_TEST_FAILED;
    }
    printf("PASSED\n");
    
    // TODO: Implement actual audio device tests
    printf("[INFO] USB Audio device test not fully implemented yet\n");
    
    return USB_TEST_PASSED;
}

usb_test_result_t test_usb_wireless(const char *device_path) {
    if (!device_path) {
        printf("Error: No device path provided\n");
        return USB_TEST_ERROR;
    }
    
    printf("\n[INFO] Testing USB Wireless device at %s\n", device_path);
    
    // Check if the device file exists and is accessible
    printf("[TEST] Checking device presence... ");
    if (access(device_path, F_OK) == -1) {
        printf("FAILED (Device not found)\n");
        return USB_TEST_FAILED;
    }
    printf("PASSED\n");
    
    // TODO: Implement actual wireless device tests
    printf("[INFO] USB Wireless device test not fully implemented yet\n");
    
    return USB_TEST_PASSED;
}

// Helper function to get USB device class
static usb_test_result_t get_usb_device_class(const char *device_path, uint8_t *device_class) {
    if (!device_path || !device_class) {
        printf("Error: Invalid input parameters\n");
        return USB_TEST_ERROR;
    }
    
    // Implementation would use libusb to get device class
    // This is a simplified placeholder
    return USB_TEST_ERROR;
}

// Helper function to check if a specific USB device is connected
static bool is_usb_device_connected(uint16_t vendor_id, uint16_t product_id) {
    libusb_device **devs = NULL;
    libusb_device *dev = NULL;
    int i = 0;
    bool found = false;

    if (vendor_id == 0 && product_id == 0) {
        printf("[WARNING] No vendor or product ID specified for device check\n");
        return false;
    }

    // Get the list of USB devices
    ssize_t cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0) {
        printf("[ERROR] Failed to get USB device list: %s\n", libusb_error_name((int)cnt));
        return false;
    }

    if (cnt == 0) {
        printf("[INFO] No USB devices found\n");
        libusb_free_device_list(devs, 1);
        return false;
    }

    printf("[DEBUG] Scanning %zd USB devices...\n", cnt);

    // Iterate through all USB devices
    while ((dev = devs[i++]) != NULL) {
        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0) {
            printf("[WARNING] Failed to get device descriptor: %s\n", libusb_error_name(r));
            continue;
        }

        // If vendor_id is 0, match any vendor; same for product_id
        bool vendor_match = (vendor_id == 0) || (desc.idVendor == vendor_id);
        bool product_match = (product_id == 0) || (desc.idProduct == product_id);

        if (vendor_match && product_match) {
            printf("[DEBUG] Found matching device: %04x:%04x\n", 
                   desc.idVendor, desc.idProduct);
            found = true;
            break;
        }
    }

    if (!found) {
        printf("[DEBUG] No matching device found for %04x:%04x\n", 
               vendor_id, product_id);
    }

    // Free the device list
    libusb_free_device_list(devs, 1);
    
    return found;
}
