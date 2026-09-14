
%if 0%{?mlz}
%define fixedversion %{version}
%else
# use fixedversion for builds on build.opensuse.org - needed for deb builds.
%define fixedversion fixed
%define compression gz
%endif

Summary:       Qwt provides a 2D plotting widget and more
Name:          qwt
Version:       6.3.0
Release:       1%{?dist}
License:       Qwt License
Source:        qwt-%{fixedversion}.tar%{?compression:.%{compression}}
BuildRequires: cmake
%if 0%{?rhel} && 0%{?rhel} <= 9
BuildRequires: qt5-qtbase-devel
BuildRequires: qt5-qtsvg-devel
Requires:      qt5-qtsvg
%else
BuildRequires: qt6-qtbase-devel
BuildRequires: qt6-qtsvg-devel
Requires:      qt6-qtsvg
%endif

%undefine __cmake_in_source_build

%description
%{summary}

%prep
%setup -q -n qwt-%{fixedversion}

%build
%cmake %{?_cmake_skip_rpath}
%cmake_build

%install
%cmake_install

%files
%defattr(-,root,root)
%{_libdir}/*.so*


%package  devel
Summary:  Qwt provides a 2D plotting widget and more
%if 0%{?rhel} && 0%{?rhel} <= 9
Requires: qt5-qtbase-devel
Requires: qt5-qtsvg-devel
%else
Requires: qt6-qtbase-devel
Requires: qt6-qtsvg-devel
%endif

%description devel
%{summary}

%files devel
%defattr(-,root,root)
%{_includedir}/qwt
%{_libdir}/libqwt*
