QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    allocateaudiothread.cpp \
    allocatevideothread.cpp \
    decodethread.cpp \
    main.cpp \
    mainwindow.cpp \
    pcmplayer.cpp

HEADERS += \
    allocateaudiothread.h \
    allocatevideothread.h \
    cache.h \
    datastructure.h \
    decodethread.h \
    mainwindow.h \
    pcmplayer.h

FORMS += \
    mainwindow.ui

#ffmpeg config start
FFMPEG_INC = $$(FFMPEG)\include
FFMPEG_LIB = $$(FFMPEG)\lib
FFMPEG_BIN = $$(FFMPEG)\bin

INCLUDEPATH += \
    $$FFMPEG_INC \

LIBS += \
    -L$$FFMPEG_LIB -lavcodec \
    -L$$FFMPEG_LIB -lavdevice \
    -L$$FFMPEG_LIB -lavfilter \
    -L$$FFMPEG_LIB -lavformat \
    -L$$FFMPEG_LIB -lavutil \
    -L$$FFMPEG_LIB -lpostproc \
    -L$$FFMPEG_LIB -lswresample \
    -L$$FFMPEG_LIB -lswscale \

DEPENDPATH += \
    FFMPEG_BIN \
#ffmpeg config end

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
