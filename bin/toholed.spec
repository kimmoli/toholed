%define        __spec_install_post %{nil}
%define          debug_package %{nil}
%define        __os_install_post %{_dbpath}/brp-compress

Summary: The Other Half OLED
Name: toholed
Version: 0.1
Release: 18
License: MIT
Group: Development/Tools
SOURCE0 : %{name}-%{version}.tar.gz
URL: https://bitbucket.org/tohs/toholed_daemon

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
%{summary}

%prep
%setup -q

%build
# Empty section.

%install
rm -rf %{buildroot}
mkdir -p  %{buildroot}

# in builddir
cp -a * %{buildroot}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_sbindir}/*
/etc/systemd/system/%{name}.service
/etc/dbus-1/system.d/%{name}.conf

%post
systemctl enable %{name}.service
systemctl start %{name}.service

%preun
systemctl stop %{name}.service
systemctl disable %{name}.service

%changelog
* Sat Apr 05 2014  Kimmo Lindholm <kimmo.lindholm@gmail.com> 0.1-18
- Brightness settings, ALS Interrupt hysteresis
* Mon Mar 31 2014  Kimmo Lindholm <kimmo.lindholm@gmail.com> 0.1-9
- First RPM

