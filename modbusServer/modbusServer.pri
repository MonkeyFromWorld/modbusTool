QT += serialbus widgets core
qtConfig(modbus-serialport): QT += serialport
 
HEADERS += \
    $$PWD/modbusServer.h \
    $$PWD/serverSettingsdialog.h


SOURCES += \
    $$PWD/modbusServer.cpp \
    $$PWD/serverSettingsdialog.cpp

FORMS += \
    $$PWD/modbusServer.ui \
    $$PWD/serverSettingsdialog.ui

RESOURCES += \
    $$PWD/modbusServer.qrc


