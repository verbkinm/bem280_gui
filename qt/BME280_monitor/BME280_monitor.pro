QT += widgets serialport
requires(qtConfig(combobox))

TARGET = BME280_monitor
TEMPLATE = app

SOURCES += \
    datatransmit.cpp \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp

HEADERS += \
    datatransmit.h \
    mainwindow.h \
    settingsdialog.h

FORMS += \
    datatransmit.ui \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    terminal.qrc

#target.path = $$[QT_INSTALL_EXAMPLES]/serialport/terminal
INSTALLS += target
