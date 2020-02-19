#-------------------------------------------------
#
# Project created by QtCreator 2019-05-03T18:16:23
#
#-------------------------------------------------

QT       += core gui opengl xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Almond
TEMPLATE = app
RC_ICONS = almond.ico
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
@CONFIG += debug_and_release@

SOURCES += \
        Almond.cpp \
        BackgroundTask.cpp \
        Bitmap.cpp \
        Color.cpp \
        CubicSpline.cpp \
        Gradient.cpp \
        GradientWidget.cpp \
        MandelVideoGenerator.cpp \
        MandelWidget.cpp \
        VideoStream.cpp \
        choosegenerators.cpp \
        exportdialogs.cpp \
        gradientchoosedialog.cpp \
        main.cpp

HEADERS += \
        Almond.h \
        BackgroundTask.h \
        Bitmap.h \
        Color.h \
        CubicSpline.h \
        Gradient.h \
        GradientWidget.h \
        MandelVideoGenerator.h \
        MandelWidget.h \
        VideoStream.h \
        choosegenerators.h \
        exportdialogs.h \
        gradientchoosedialog.h

FORMS += \
        Almond.ui \
        choosegenerators.ui \
        exportimagedialog.ui \
        exportvideodialog.ui \
        gradientchooser.ui



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:LIBS += -lopengl32
else:LIBS += -lOpenGL

win32:QMAKE_CXXFLAGS += -openmp
else:unix:QMAKE_CXXFLAGS +=
win32:QMAKE_LFLAGS +=  -openmp
else:unix:QMAKE_LFLAGS+= -fopenmp -flto
LIBS += -fopenmp
unix:LIBS += -lm -latomic -lquadmath

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libs/ffmpeg-20200216-8578433-win64-dev/lib/ -lavcodec
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libs/ffmpeg-20200216-8578433-win64-dev/lib/ -lavcodec
else:unix: LIBS += -L$$PWD/../libs/ffmpeg-4.1.1-win32-dev/lib/ -lavcodec

win32:FFMPEGPATH = $$PWD/../libs/ffmpeg-20200216-8578433-win64-dev/lib/
win32:INCLUDEPATH += $$PWD/../libs/ffmpeg-20200216-8578433-win64-dev/include
win32:DEPENDPATH += $$PWD/../libs/ffmpeg-20200216-8578433-win64-dev/include

win32:INCLUDEPATH += ../libs/boost_1_72_0
DEFINES += WITH_BOOST=1

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../../../../Program Files (x86)/AMD APP SDK/3.0/lib/x86/' -lOpenCL
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../../../../Program Files (x86)/AMD APP SDK/3.0/lib/x86/' -lOpenCL
#else:unix: LIBS += -lOpenCL


win32:INCLUDEPATH += $$PWD/'../../../../../Program Files (x86)/AMD APP SDK/3.0/include'
win32:DEPENDPATH += $$PWD/'../../../../../Program Files (x86)/AMD APP SDK/3.0/include'

win32:CONFIG(release, debug|release): LIBS += -L$FFMPEGPATH -lavformat
else:win32:CONFIG(debug, debug|release): LIBS += -L$FFMPEGPATH -lavformat
else:unix: LIBS += -L$FFMPEGPATH -lavformat

#INCLUDEPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include
#DEPENDPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include

unix|win32: LIBS += -L$FFMPEGPATH -lavdevice

#INCLUDEPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include
#DEPENDPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include

unix|win32: LIBS += -L$FFMPEGPATH -lavfilter

#INCLUDEPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include
#DEPENDPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include

unix|win32: LIBS += -L$FFMPEGPATH -lavutil

#INCLUDEPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include
#DEPENDPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include

unix|win32: LIBS += -L$FFMPEGPATH -lswscale

#INCLUDEPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include
#DEPENDPATH += $$PWD/../libs/ffmpeg-4.1.1-win32-dev/include

RESOURCES += Almond.qrc

unix|win32: LIBS += -L$$PWD/libmandel/ -lmandel -lqd

INCLUDEPATH += $$PWD/libmandel/include $$PWD/libmandel/qd-2.3.22/include
DEPENDPATH += $$PWD/libmandel/include $$PWD/libmandel/qd-2.3.22/include

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/libmandel/mandel.lib  $$PWD/libmandel/qd.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/libmandel/libmandel.a $$PWD/libmandel/libqd.a


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../../../../Program Files (x86)/OCL_SDK_Light/lib/x86_64/' -lopencl
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../../../../Program Files (x86)/OCL_SDK_Light/lib/x86_64/' -lopencl
else:unix: LIBS += -lOpenCL

win32:INCLUDEPATH += $$PWD/'../../../../../Program Files (x86)/OCL_SDK_Light/include'
win32:DEPENDPATH += $$PWD/'../../../../../Program Files (x86)/OCL_SDK_Light/include'

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/'../../../../../Program Files (x86)/OCL_SDK_Light/lib/x86_64/libopencl.a'
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/'../../../../../Program Files (x86)/OCL_SDK_Light/lib/x86_64/libopencl.a'
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/'../../../../../Program Files (x86)/OCL_SDK_Light/lib/x86_64/opencl.lib'
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/'../../../../../Program Files (x86)/OCL_SDK_Light/lib/x86_64/opencl.lib'
