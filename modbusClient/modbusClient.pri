QT += serialbus widgets core
qtConfig(modbus-serialport): QT += serialport

HEADERS += \
    $$PWD/clientSettingsdialog.h \
    $$PWD/modbusclient.h


SOURCES += \
    $$PWD/clientSettingsdialog.cpp \
    $$PWD/modbusclient.cpp

FORMS += \
    $$PWD/clientSettingsdialog.ui \
    $$PWD/modbusClient.ui

RESOURCES += \
    $$PWD/modbusClient.qrc


