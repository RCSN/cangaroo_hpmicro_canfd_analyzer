lessThan(QT_MAJOR_VERSION, 5): error("requires Qt 5")

QT += core gui
QT += widgets
QT += xml
QT += charts
QT += serialport

TARGET = cangaroo
TEMPLATE = app
CONFIG += warn_on
CONFIG += link_pkgconfig

DESTDIR = ../bin
MOC_DIR = ../build/moc
RCC_DIR = ../build/rcc
UI_DIR = ../build/ui
unix:OBJECTS_DIR = ../build/o/unix
win32:OBJECTS_DIR = ../build/o/win32
macx:OBJECTS_DIR = ../build/o/mac


SOURCES += main.cpp\
    mainwindow.cpp \

HEADERS  += mainwindow.h \

FORMS    += mainwindow.ui

RESOURCES = cangaroo.qrc

include($$PWD/core/core.pri)
include($$PWD/driver/driver.pri)
include($$PWD/parser/dbc/dbc.pri)
include($$PWD/window/TraceWindow/TraceWindow.pri)
include($$PWD/window/SetupDialog/SetupDialog.pri)
include($$PWD/window/LogWindow/LogWindow.pri)
include($$PWD/window/GraphWindow/GraphWindow.pri)
include($$PWD/window/CanStatusWindow/CanStatusWindow.pri)
include($$PWD/window/RawTxWindow/RawTxWindow.pri)


unix:PKGCONFIG += libnl-3.0
unix:PKGCONFIG += libnl-route-3.0
unix:include($$PWD/driver/SocketCanDriver/SocketCanDriver.pri)

include($$PWD/driver/CANBlastDriver/CANBlastDriver.pri)
include($$PWD/driver/SLCANDriver/SLCANDriver.pri)

win32:include($$PWD/driver/CandleApiDriver/CandleApiDriver.pri)

unix {
    target.path = $${PREFIX}/bin
    INSTALLS += target

    DESKTOP.files = $$PWD/../cangaroo.desktop
    DESKTOP.path = $${PREFIX}/share/applications
    INSTALLS += DESKTOP

    ICONS.files = $$PWD/assets/cangaroo.svg \
                $$PWD/assets/mdibg.svg
    ICONS.path = $${PREFIX}/share/pixmaps
    INSTALLS += ICONS

    SCRIPT_PATH = $$PWD/scripts/setup_vcan.sh
    script.files = $$PWD/scripts/cangaroo-setup-vcan
    script.path = $${PREFIX}/bin
    script.extra = mv $${SCRIPT_PATH} $${script.files} && chmod +x $${script.files} && mv $${script.files} $(INSTALL_ROOT)$${script.path}
    INSTALLS += script

    isEmpty(PREFIX):PREFIX=/usr
}
