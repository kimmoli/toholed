#
# toholed spec
# (C) kimmoli 2015
#

Name: harbour-ambience-toholed

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

Summary:  The OtherHalf OLED
Version:  0.4.15
Release:  devel
Group:    Qt/Qt
License:  LICENSE
URL:      https://github.com/kimmoli/toholed
Source0:  %{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(libiphb)
BuildRequires:  pkgconfig(contextkit-statefs)
BuildRequires:  pkgconfig(sailfishapp) >= 0.0.10
BuildRequires:  pkgconfig(Qt5Xml)
BuildRequires:  pkgconfig(Qt5XmlPatterns)
BuildRequires:  desktop-file-utils

Requires:   ambienced

Obsoletes:  harbour-toholed
Obsoletes:  harbour-toholed-settings-ui

%description
The OtherHalf OLED daemon and Settings application

%prep
%setup -q -n %{name}-%{version}

%build

%qtc_qmake5 SPECVERSION=%{version}

%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%qmake5_install

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}/harbour-toholed
%attr(755,root,root) %{_bindir}/harbour-toholed-settings-ui
%config /etc/systemd/system/harbour-toholed.service
%config /etc/udev/rules.d/95-harbour-toholed.rules
%config /etc/dbus-1/system.d/harbour-toholed.conf
%{_datadir}/ambience/%{name}/%{name}.ambience
%{_datadir}/ambience/%{name}/images/%{name}.jpg
%{_datadir}/harbour-toholed-settings-ui
%{_datadir}/applications/harbour-toholed-settings-ui.desktop
%{_datadir}/icons/hicolor/86x86/apps/harbour-toholed-settings-ui.png

%post
#reload udev rules
udevadm control --reload
# if toholed is connected, start daemon now
if [ -e /sys/devices/platform/toh-core.0/vendor ]; then
 if grep -q 19276 /sys/devices/platform/toh-core.0/vendor ; then
  if grep -q 2 /sys/devices/platform/toh-core.0/product ; then
   systemctl start harbour-toholed.service
  fi
 fi
fi
%_ambience_post

%pre
# In case of update, stop and disable first
if [ "$1" = "2" ]; then
  systemctl stop harbour-toholed.service
  systemctl disable harbour-toholed.service
  udevadm control --reload
fi

%preun
# in case of complete removal, stop and disable
if [ "$1" = "0" ]; then
  systemctl stop harbour-toholed.service
  systemctl disable harbour-toholed.service
  udevadm control --reload
fi
