QT       += core gui
QT       += concurrent sql
#QT -= networks qml quick
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QMAKE_PROJECT_DEPTH = 0
#QMAKE_CXXFLAGS += /fsanitize=address /INCLUDE:__asan_init

CONFIG += c++17
CONFIG += clean
CONFIG += console
#QMAKE_LFLAGS += /LTCG
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
        LIBS += -L$$PWD/lib/cryptolibs/win64/ -llibcryptoalone19_341
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
    src/DCT.cpp \
    src/Encryption.cpp \
    src/aboutbox.cpp \
    src/bulk_decode.cpp \
    src/bulk_encode.cpp \
    src/databaseviewer.cpp \
    src/dctreader.cpp \
    src/dialog.cpp \
    src/logicmain.cpp \
    src/main.cpp \
    src/mainwindow.cpp\
    src/Linear_Image.cpp\
    src/utils_a.cpp

HEADERS += \
    include/GlobalSettings.h \
    include/aboutbox.h \
    include/bulk_decode.h \
    include/bulk_encode.h \
    include/databaseviewer.h \
    include/dctreader.h \
    include/dialog.h \
    include/logicmain.h \
    include/mainwindow.h \
    include/utils_a.h

FORMS += \
    forms/aboutbox.ui \
    forms/bulk_decode.ui \
    forms/bulk_encode.ui \
    forms/databaseviewer.ui \
    forms/dctreader.ui \
    forms/dialog.ui \
    forms/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resources/about.qrc
