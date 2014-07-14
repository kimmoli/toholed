#
# toholed spec
# (C) kimmoli 2014
#

Name: toholed

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

Summary: The OtherHalf OLED daemon
Version: 0.1
Release: 24
Group: Qt/Qt
License: LICENSE
URL: https://github.com/kimmoli/toholed
Source0: %{name}-%{version}.tar.bz2

BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5DBus)

Requires:   ambienced

%description
The OtherHalf OLED daemon

%prep
%setup -q -n %{name}-%{version}

%build

%qtc_qmake5 SPECVERSION=%{version}

%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%qmake5_install

%files
%defattr(-,root,root,-)
%{_sbindir}/%{name}
%config /etc/systemd/system/%{name}.service
%config /etc/udev/rules.d/95-%{name}.rules
%config /etc/dbus-1/system.d/%{name}.conf
%{_datadir}/ambience/%{name}
%{_datadir}/ambience/%{name}/%{name}.ambience
%{_datadir}/ambience/%{name}/images/*

%post
#reload udev rules
udevadm control --reload
# if toholed is connected, start daemon now
if [ -e /sys/devices/platform/toh-core.0/vendor ]; then
 if grep -q 19276 /sys/devices/platform/toh-core.0/vendor ; then
  if grep -q 2 /sys/devices/platform/toh-core.0/product ; then
   systemctl start %{name}.service
  fi
 fi
fi
%_ambience_post

%pre
# In case of update, stop and disable first
if [ "$1" = "2" ]; then
  systemctl stop %{name}.service
  systemctl disable %{name}.service
  udevadm control --reload
fi

%preun
# in case of complete removal, stop and disable
if [ "$1" = "0" ]; then
  systemctl stop %{name}.service
  systemctl disable %{name}.service
  udevadm control --reload
fi
