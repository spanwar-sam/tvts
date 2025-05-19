#!/bin/bash

# Configuration
TARGET="linux"
DEBUG="false"

# Colors for output
RED="\033[0;31m"
GREEN="\033[0;32m"
BLUE="\033[0;34m"
NC="\033[0m" # No Color

# Target-specific configurations
TARGET_CONFIGS=(
    "linux:x86_64:"
    "tizen8:armv7l:mobile"
    "tizen9:armv7l:mobile"
)

function print_info() {
    echo -e "${BLUE}[*] $1${NC}"
}

function print_success() {
    echo -e "${GREEN}[+] $1${NC}"
}

function print_error() {
    echo -e "${RED}[-] $1${NC}"
    exit 1
}

function usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  --target <target>    Target to build for (linux|tizen8|tizen9)"
    echo "  --debug              Build with debug symbols"
    echo "  --help               Show this help message"
    exit 0
}

function parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --target)
                TARGET="$2"
                shift 2
                ;;
            --debug)
                DEBUG="true"
                shift
                ;;
            --help)
                usage
                ;;
            *)
                print_error "Unknown option: $1"
                ;;
        esac
    done
}

function get_target_config() {
    local target=$1
    for config in "${TARGET_CONFIGS[@]}"; do
        IFS=':' read -r t arch profile <<< "$config"
        if [[ $t == $target ]]; then
            echo "$arch $profile"
            return 0
        fi
    done
    print_error "Unsupported target: $target"
}

function build_package() {
    local target=$1
    local arch=$(get_target_config $target | awk '{print $1}')
    local profile=$(get_target_config $target | awk '{print $2}')
    
    print_info "Building package for $target ($arch)"
    
    # Create source tarball
    make dist || print_error "Failed to create source tarball"
    
    # Generate target-specific spec file
    if [[ $target =~ ^tizen ]]; then
        sed "s/%{tizen_version}/$(echo $target | sed 's/tizen//')/g" tizen_drm_test_suite.spec.in > tizen_drm_test_suite.spec
    fi
    
    # Copy to appropriate workspace
    if [[ $target == "linux" ]]; then
        cp tizen-drm-test-suite-1.0.0.tar.gz /tmp/
    else
        cp tizen_drm_test_suite.spec ~/tizen-workspace/tizen-drm-test-suite/ || \
            print_error "Failed to copy spec file"
        cp tizen-drm-test-suite-1.0.0.tar.gz ~/tizen-workspace/tizen-drm-test-suite/ || \
            print_error "Failed to copy source tarball"
        
        # Build using GBS
        if [ "$DEBUG" = "true" ]; then
            print_info "Building with debug symbols"
            gbs build --include-all --arch $arch --profile $profile --debug || \
                print_error "Build failed"
        else
            gbs build --include-all --arch $arch --profile $profile || \
                print_error "Build failed"
        fi
    fi
    
    print_success "Package build completed successfully"
}

function install_package() {
    local target=$1
    local arch=$(get_target_config $target | awk '{print $1}')
    
    if [[ $target == "linux" ]]; then
        print_info "Installing package on Linux"
        sudo rpm -Uvh /tmp/package.rpm || print_error "Installation failed"
    else
        print_info "Installing package on Tizen target"
        sdb devices | grep device || print_error "No target device found"
        sdb shell "setprop debug.drm.enable 1"
        sdb push /path/to/package.rpm /tmp/ || print_error "Failed to copy package"
        sdb shell "rpm -Uvh /tmp/package.rpm" || print_error "Failed to install package"
    fi
    
    print_success "Package installed successfully"
}

function run_tests() {
    local target=$1
    
    print_info "Running tests on $target"
    
    if [[ $target == "linux" ]]; then
        ./test_suite || print_error "Tests failed"
    else
        sdb shell "/usr/bin/test_suite" || print_error "Tests failed"
    fi
    
    print_success "Tests completed successfully"
}

function main() {
    parse_args "$@"
    
    # Check required commands
    if [[ $TARGET != "linux" ]]; then
        check_command gbs
        check_command sdb
    fi
    check_command make
    
    # Build package
    build_package $TARGET
    
    # Install package
    install_package $TARGET
    
    # Run tests
    run_tests $TARGET
}

main "$@"
