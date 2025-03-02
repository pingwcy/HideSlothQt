QT       += core gui
QT       += concurrent sql
#QT -= networks qml quick
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QMAKE_PROJECT_DEPTH = 0
#QMAKE_CXXFLAGS += /fsanitize=address /INCLUDE:__asan_init

CONFIG += c++17
CONFIG += clean

#The next two lines are for MSVC static link

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += $$PWD/include

win32 {
QMAKE_CXXFLAGS += /utf-8
QMAKE_CXXFLAGS_RELEASE += -MT
QMAKE_CFLAGS_RELEASE += -MT
greaterThan(QT_MAJOR_VERSION, 5) {
    # Qt 6.x 或更高版本
    contains(QMAKE_CXXFLAGS, -MD) {
        # 动态链接 (MD)
        LIBS += -L$$PWD/lib/cryptolibs/win64/ -llibcryptosym
    } else {
        # 静态链接 (MT)
        LIBS += -L$$PWD/lib/winsys/ -lcrypt32 -luser32 -ladvapi32 -lkernel32
        LIBS += -L$$PWD/lib/cryptolibs/win64/ -llibcryptostandalone22
    }
} else {
    # Qt 5.x 版本
    # 判断是否使用了 -MD（动态链接运行时库）
    contains(QMAKE_CXXFLAGS, -MD) {
        # 动态链接 (MD)
        LIBS += -L$$PWD/lib/cryptolibs/win64/ -llibcryptosym
    } else {
        # 静态链接 (MT)
        LIBS += -L$$PWD/lib/winsys/ -lcrypt32 -luser32 -ladvapi32 -lkernel32
        LIBS += -L$$PWD/lib/cryptolibs/win64/ -llibcryptostandalone19
    }
}
}
unix:!macx{
LIBS += -L$$PWD/lib/cryptolibs/linux64/ -lcryptolinux
LIBS += -L$$PWD/ -ldl
#QMAKE_CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
#QMAKE_LFLAGS += -fsanitize=address

}
SOURCES += \
    DCT.cpp \
    Encryption.cpp \
    aboutbox.cpp \
    bulk_decode.cpp \
    bulk_encode.cpp \
    databaseviewer.cpp \
    dctreader.cpp \
    dialog.cpp \
    main.cpp \
    mainwindow.cpp\
    Linear_Image.cpp\
    utils_a.cpp


HEADERS += \
    GlobalSettings.h \
    aboutbox.h \
    bulk_decode.h \
    bulk_encode.h \
    databaseviewer.h \
    dctreader.h \
    dialog.h \
    mainwindow.h \
    utils_a.h

FORMS += \
    aboutbox.ui \
    bulk_decode.ui \
    bulk_encode.ui \
    databaseviewer.ui \
    dctreader.ui \
    dialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resources/about.qrc
