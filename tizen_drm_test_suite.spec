Name:           tizen-drm-test-suite
Summary:        DRM Test Suite for Tizen
Version:        1.0.0
Release:        1
License:        Apache-2.0
Group:          Development/Tools
URL:            https://github.com/tizen/tizen-drm-test-suite
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  libdrm-devel
BuildRequires:  libdrm_mode-devel
BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  pkgconfig

%description
DRM Test Suite for Tizen provides comprehensive testing for DRM functionality including buffer sharing, format conversion, and plane configuration.

%package devel
Summary:        Development files for %{name}
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}

%description devel
Development files for %{name}, including headers and libraries.

%prep
%setup -q

%build
make

%install
mkdir -p %{buildroot}%{_bindir}
install -m 755 test_suite %{buildroot}%{_bindir}/

mkdir -p %{buildroot}%{_includedir}/tizen_drm_test
install -m 644 include/tizen_drm_test.h %{buildroot}%{_includedir}/tizen_drm_test/

%files
%{_bindir}/test_suite

%files devel
%{_includedir}/tizen_drm_test/tizen_drm_test.h

%changelog
* Thu May 17 2024 - sumit
- Initial release of Tizen DRM Test Suite
