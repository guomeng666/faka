HEADERS     += $$PWD/ffmpeghead.h

HEADERS     += $$PWD/ffmpeg.h
HEADERS     += $$PWD/videoffmpeg.h
HEADERS     += $$PWD/usbcamera.h

SOURCES     += $$PWD/ffmpeg.cpp
SOURCES     += $$PWD/videoffmpeg.cpp
SOURCES     += $$PWD/usbcamera.cpp

contains(DEFINES, qt32) {
INCLUDEPATH += $$PWD/include
} else {
INCLUDEPATH += $$PWD/include64
}

win32 {
contains(DEFINES, qt32) {
LIBS += -L$$PWD/winlib/ -lavcodec -lavfilter -lavformat -lswscale -lavutil
} else {
LIBS += -L$$PWD/winlib64/ -lavcodec -lavfilter -lavformat -lswscale -lavutil
}
}

unix {
contains(DEFINES, gcc47) {LIBS += -L$$PWD/armlib/ -lavcodec -lavfilter -lavformat -lswscale -lavutil -lswresample}
contains(DEFINES, gcc45) {LIBS += -L$$PWD/armlib4.5/ -lavcodec -lavfilter -lavformat -lswscale -lavutil -lpostproc -lswresample}
contains(DEFINES, rk3399) {LIBS += -L$$PWD/rk3399lib/ -lavcodec -lavfilter -lavformat -lswscale -lavutil -lswresample} 
else {LIBS += -L$$PWD/linuxlib/ -lavfilter -lavformat -lavdevice -lavcodec -lswscale -lavutil -lswresample -lpthread -lm -lrt -ldl -lz}
}
