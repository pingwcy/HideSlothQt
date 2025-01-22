QT       += core gui
QT       += concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Encryption.cpp \
    aboutbox.cpp \
    dialog.cpp \
    main.cpp \
    mainwindow.cpp\
    Linear_Image.cpp\


win32 {
INCLUDEPATH += $$PWD/include
### The first line is static build, the second line is dynamic!
LIBS += -L$$PWD/lib/ -llibcrypto -lcrypt32 -luser32 -ladvapi32 -lkernel32
#LIBS += -L$$PWD/lib/ -llibcryptosym
}
unix:!macx{
LIBS += -L$$PWD/ -lcrypto -ldl

}
HEADERS += \
    GlobalSettings.h \
    aboutbox.h \
    dialog.h \
    mainwindow.h

FORMS += \
    aboutbox.ui \
    dialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RESOURCES += \
    Resources/about.qrc
