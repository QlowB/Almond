#-------------------------------------------------
#
# Project created by QtCreator 2019-05-03T18:16:23
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Almond
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

SOURCES += \
        Almond.cpp \
        Bitmap.cpp \
        Generators.cpp \
        MandelWidget.cpp \
        QueueManager.cpp \
        SectionManager.cpp \
        VideoStream.cpp \
        benchmarkdialog.cpp \
        main.cpp

HEADERS += \
        Almond.h \
        Bitmap.h \
        Color.h \
        Fixed.h \
        Generators.h \
        GenericMandelbrot.h \
        MandelWidget.h \
        QueueManager.h \
        SectionManager.h \
        VideoStream.h \
        benchmarkdialog.h

FORMS += \
        Almond.ui \
        benchmarks.ui \
        exportimagedialog.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -lopengl32
win32:QMAKE_CXXFLAGS+= -openmp
else:unix:QMAKE_CXXFLAGS+= -fopenmp
win32:QMAKE_LFLAGS +=  -openmp
else:unix:QMAKE_LFLAGS+= -fopenmp
LIBS += -fopenmp

QMAKE_CXXFLAGS += -mavx

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libs/ffmpeg-4.1.1-win32-dev/lib/ -lavcodec
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libs/ffmpeg-4.1.1-win32-dev/lib/ -lavcodec
else:unix: LIBS += -L$$PWD/../libs/ffmpeg-4.1.1-win32-dev/lib/ -lavcodec

INCLUDEPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include
DEPENDPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../../../../Program Files (x86)/AMD APP SDK/3.0/lib/x86/' -lOpenCL
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../../../../Program Files (x86)/AMD APP SDK/3.0/lib/x86/' -lOpenCL
else:unix: LIBS += -L$$PWD/'../../../../../Program Files (x86)/AMD APP SDK/3.0/lib/x86/' -lOpenCL

INCLUDEPATH += $$PWD/'../../../../../Program Files (x86)/AMD APP SDK/3.0/include'
DEPENDPATH += $$PWD/'../../../../../Program Files (x86)/AMD APP SDK/3.0/include'

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libs/ffmpeg-4.1.1-win32-dev/lib/ -lavformat
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libs/ffmpeg-4.1.1-win32-dev/lib/ -lavformat
else:unix: LIBS += -L$$PWD/../libs/ffmpeg-4.1.1-win32-dev/lib/ -lavformat

INCLUDEPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include
DEPENDPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include

unix|win32: LIBS += -L$$PWD/../libs/ffmpeg-4.1.1-win32-dev/lib/ -lavdevice

INCLUDEPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include
DEPENDPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include

unix|win32: LIBS += -L$$PWD/../libs/ffmpeg-4.1.1-win32-dev/lib/ -lavfilter

INCLUDEPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include
DEPENDPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include

unix|win32: LIBS += -L$$PWD/../libs/ffmpeg-4.1.1-win32-dev/lib/ -lavutil

INCLUDEPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include
DEPENDPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include

unix|win32: LIBS += -L$$PWD/../libs/ffmpeg-4.1.1-win32-dev/lib/ -lswscale

INCLUDEPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include
DEPENDPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include

RESOURCES += \
    Almond.qrc
