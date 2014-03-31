Toholed
=======

install
==========

root access needed, `devel-su`

copy binary `toholed` to `/usr/sbin/` and `chmod 755 /usr/sbin/toholed`

copy `toholed.conf` to `/etc/dbus-1/system.d/`

copy `toholed.service` to `/etc/systemd/system/`

start service `systemctl start toholed.service`

enable service to start at boot `systemctl enable toholed.service`

To see what is going on there `journalctl _SYSTEMD_UNIT=toholed.service -f` it is kinda verbal creature


dbus calls
==========

Screencapture on front proximity:

`dbus-send --system --print-reply --dest=com.kimmoli.toholed / com.kimmoli.toholed.setScreenCaptureOnProximity string:"on"`

`dbus-send --system --print-reply --dest=com.kimmoli.toholed / com.kimmoli.toholed.setScreenCaptureOnProximity string:"off"`

Draw circle x y radius:

`dbus-send --system --print-reply --dest=com.kimmoli.toholed / com.kimmoli.toholed.draw string:"circle" int32:25 int32:25 int32:20`


Draw windows mono bitmap:

see example http://pastebin.com/hvxp72VD