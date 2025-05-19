CROSS_COMPILE ?= 
CC ?= gcc
CFLAGS = -Wall -Werror -O2 -I./include
LDFLAGS = 

# Source directories
DRM_SRC = $(wildcard src/drm/*.c)
AUDIO_SRC = $(wildcard src/audio/*.c)
VIDEO_SRC = $(wildcard src/video/*.c)
REPORT_SRC = $(wildcard src/report/*.c)
MAIN_SRC = src/test_main.c

# Object files
DRM_OBJ = $(DRM_SRC:.c=.o)
AUDIO_OBJ = $(AUDIO_SRC:.c=.o)
VIDEO_OBJ = $(VIDEO_SRC:.c=.o)
REPORT_OBJ = $(REPORT_SRC:.c=.o)
MAIN_OBJ = $(MAIN_SRC:.c=.o)

# Header files
HEADERS = $(wildcard include/*.h) $(wildcard include/audio/*.h) $(wildcard include/video/*.h) $(wildcard include/report/*.h)

# Subsystem flags
DRM_CFLAGS = -D_ENABLE_DRM
DRM_LDFLAGS = -ldrm -lxf86drm -lxf86drm_mode

AUDIO_CFLAGS = -D_ENABLE_AUDIO
AUDIO_LDFLAGS = -lasound

VIDEO_CFLAGS = -D_ENABLE_VIDEO
VIDEO_LDFLAGS = -lv4l2

# Target-specific variables
ifeq ($(TARGET),linux)
    TARGET_ARCH = x86_64
    TARGET_CFLAGS = -D_LINUX
    TARGET_LDFLAGS = 
endif

ifeq ($(TARGET),tizen8)
    TARGET_ARCH = armv7l
    TARGET_CFLAGS = -D_TIZEN_8 -D_ARM
    TARGET_LDFLAGS = 
endif

ifeq ($(TARGET),tizen9)
    TARGET_ARCH = armv7l
    TARGET_CFLAGS = -D_TIZEN_9 -D_ARM
    TARGET_LDFLAGS = 
endif

# Subsystem selection
SUBSYSTEMS ?= all

# The report module is always included
REPORT_CFLAGS = -D_ENABLE_REPORT
REPORT_LDFLAGS = 

ifeq ($(SUBSYSTEMS),drm)
    ENABLED_CFLAGS = $(DRM_CFLAGS) $(REPORT_CFLAGS)
    ENABLED_LDFLAGS = $(DRM_LDFLAGS) $(REPORT_LDFLAGS)
    OBJECTS = $(DRM_OBJ) $(REPORT_OBJ) $(MAIN_OBJ)
else ifeq ($(SUBSYSTEMS),audio)
    ENABLED_CFLAGS = $(AUDIO_CFLAGS) $(REPORT_CFLAGS)
    ENABLED_LDFLAGS = $(AUDIO_LDFLAGS) $(REPORT_LDFLAGS)
    OBJECTS = $(AUDIO_OBJ) $(REPORT_OBJ) $(MAIN_OBJ)
else ifeq ($(SUBSYSTEMS),video)
    ENABLED_CFLAGS = $(VIDEO_CFLAGS) $(REPORT_CFLAGS)
    ENABLED_LDFLAGS = $(VIDEO_LDFLAGS) $(REPORT_LDFLAGS)
    OBJECTS = $(VIDEO_OBJ) $(REPORT_OBJ) $(MAIN_OBJ)
else
    ENABLED_CFLAGS = $(DRM_CFLAGS) $(AUDIO_CFLAGS) $(VIDEO_CFLAGS) $(REPORT_CFLAGS)
    ENABLED_LDFLAGS = $(DRM_LDFLAGS) $(AUDIO_LDFLAGS) $(VIDEO_LDFLAGS) $(REPORT_LDFLAGS)
    OBJECTS = $(DRM_OBJ) $(AUDIO_OBJ) $(VIDEO_OBJ) $(REPORT_OBJ) $(MAIN_OBJ)
endif

# Default target is linux
TARGET ?= linux

# Combine flags
CFLAGS += $(TARGET_CFLAGS) $(ENABLED_CFLAGS)
LDFLAGS += $(TARGET_LDFLAGS) $(ENABLED_LDFLAGS)

all: test_suite

test_suite: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(DRM_OBJ) $(AUDIO_OBJ) $(VIDEO_OBJ) $(REPORT_OBJ) $(MAIN_OBJ) test_suite

dist: clean
	mkdir -p tizen-vendor-test-suite-1.0.0
	cp -r include src Makefile tizen_vendor_test_suite.spec ChangeLog README.GBS tizen-vendor-test-suite-1.0.0/
	tar -czf tizen-vendor-test-suite-1.0.0.tar.gz tizen-vendor-test-suite-1.0.0/
	rm -rf tizen-vendor-test-suite-1.0.0

# Subsystem-specific targets
drm:
	$(MAKE) SUBSYSTEMS=drm

audio:
	$(MAKE) SUBSYSTEMS=audio

video:
	$(MAKE) SUBSYSTEMS=video

# Cross-compilation targets
linux:
	$(MAKE) TARGET=linux

tizen8:
	$(MAKE) TARGET=tizen8

tizen9:
	$(MAKE) TARGET=tizen9

# Combined targets
linux-drm: 
	$(MAKE) TARGET=linux SUBSYSTEMS=drm

linux-audio: 
	$(MAKE) TARGET=linux SUBSYSTEMS=audio

linux-video: 
	$(MAKE) TARGET=linux SUBSYSTEMS=video

tizen8-drm: 
	$(MAKE) TARGET=tizen8 SUBSYSTEMS=drm

tizen8-audio: 
	$(MAKE) TARGET=tizen8 SUBSYSTEMS=audio

tizen8-video: 
	$(MAKE) TARGET=tizen8 SUBSYSTEMS=video

tizen9-drm: 
	$(MAKE) TARGET=tizen9 SUBSYSTEMS=drm

tizen9-audio: 
	$(MAKE) TARGET=tizen9 SUBSYSTEMS=audio

tizen9-video: 
	$(MAKE) TARGET=tizen9 SUBSYSTEMS=video

.PHONY: all clean dist drm audio video linux tizen8 tizen9 linux-drm linux-audio linux-video tizen8-drm tizen8-audio tizen8-video tizen9-drm tizen9-audio tizen9-video

cross-compile:
	# For ARM64
	CC=aarch64-linux-gnu-gcc
	CFLAGS=-Wall -Werror -O2
	LDFLAGS=-L$(LIBDRM_PATH) -L$(LIBDRM_MODE_PATH) -ldrm -lxf86drm -lxf86drm_mode
	INCLUDES=-I$(INCLUDE_PATH)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC_DIR)/tizen_drm_test.c $(SRC_DIR)/test_main.c -o test_suite_arm64 $(LDFLAGS)

run-tests:
	./test_suite

.PHONY: clean cross-compile run-tests
