QT       += core gui
QT       += concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#QMAKE_LFLAGS += /NODEFAULTLIB:jpeg.lib /NODEFAULTLIB:libjpeg.lib /NODEFAULTLIB:jpeg62.lib
QMAKE_PROJECT_DEPTH = 0
QMAKE_CXXFLAGS += /utf-8

CONFIG += c++17
#The next two lines are for MSVC static link
QMAKE_CXXFLAGS_RELEASE += -MT
QMAKE_CFLAGS_RELEASE += -MT

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32 {
INCLUDEPATH += $$PWD\include
LIBS += -L$$PWD/lib/ -llibcryptostandalone -lcrypt32 -luser32 -ladvapi32 -lkernel32
}
unix:!macx{
LIBS += -L$$PWD/ -lcrypto -ldl

}
SOURCES += \
    DCT.cpp \
    Encryption.cpp \
    aboutbox.cpp \
    dctreader.cpp \
    dialog.cpp \
    main.cpp \
    mainwindow.cpp\
    Linear_Image.cpp\


HEADERS += \
    GlobalSettings.h \
    aboutbox.h \
    dctreader.h \
    dialog.h \
    mainwindow.h

FORMS += \
    aboutbox.ui \
    dctreader.ui \
    dialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resources/about.qrc
