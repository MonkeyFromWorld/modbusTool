#include (./login/login.pri )
include (./navegation/navegation.pri )
include (./modbusClient/modbusClient.pri)
include (./modbusServer/modbusServer.pri)

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET      = Tools
TEMPLATE    = app
MOC_DIR     = temp/moc
RCC_DIR     = temp/rcc
UI_DIR      = temp/ui
OBJECTS_DIR = temp/obj

CONFIG      += c++11

SOURCES += \
    main.cpp \
    widget.cpp

HEADERS += \
    common.h \
    widget.h

FORMS       +=
RESOURCES   +=

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

