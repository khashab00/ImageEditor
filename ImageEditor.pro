QT       += core gui
QT       += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

win32:RC_ICONS += images/pixmaps/logo.ico
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
RESOURCES += qdarkstyle/style.qrc
QT += quickcontrols2
QT_QUICK_CONTROLS_STYLE=universal ./app
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Settings.cpp \
    dialog.cpp \
    main.cpp \
    imageviewer.cpp
HEADERS += \
    Settings.h \
    dialog.h \
    imageviewer.h
FORMS += \
    dialog.ui \
    imageviewer.ui
TRANSLATIONS = languages/ImageEditor_en.ts \
               languages/ImageEditor_de.ts
#CODECFORSRC     = UTF-8

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ressources.qrc
