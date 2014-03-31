Toholed
=======



dbus calls
==========

Screencapture on front proximity:

`dbus-send --system --print-reply --dest=com.kimmoli.toholed / com.kimmoli.toholed.setScreenCaptureOnProximity string:"on"`

`dbus-send --system --print-reply --dest=com.kimmoli.toholed / com.kimmoli.toholed.setScreenCaptureOnProximity string:"off"`

Draw circle x y radius:

`dbus-send --system --print-reply --dest=com.kimmoli.toholed / com.kimmoli.toholed.draw string:"circle" int32:25 int32:25 int32:20`


Draw windows mono bitmap:

see example http://pastebin.com/hvxp72VD