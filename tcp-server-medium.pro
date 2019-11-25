QT += core network
QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = tcp-server-medium-bbb
TEMPLATE = app

linux-beagleboard-g++:{
    target.path = /opt/matilda/bin
    INSTALLS += target
}


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


VERSION = 0.0.2

#DEFINES += APPLCTN_NAME=\\\"quick-collect\\\" it is only for GUI
DEFINES += "MYAPPNAME=\"\\\"tcp-medium-server-bbb\\\"\""
DEFINES += "MYAPPOWNER=\"\\\"KTS-Intek Ltd\\\"\""
DEFINES += "MYAPPOWNERSITE=\"\\\"http://kts-intek.com\\\"\""

DEFINES += DEV_TYPE_UC=1
DEFINES += DISABLE_RUN_PROCESS=1


include(../../Matilda-units/matilda-base/type-converter/type-converter.pri)

include(../../Matilda-units/ipc/sharedmemory/sharedmemory.pri)

include(../../Matilda-units/matilda-bbb/matilda-bbb-settings/matilda-bbb-settings.pri)

include(../../Matilda-units/ipc/localsockets/localsockets.pri)

include(../../Matilda-units/matilda-base/MatildaIO/MatildaIO.pri)


SOURCES += \
        main.cpp \
        network-src/tcpmediumsocket.cpp \
        network-src/thelordofservices.cpp \
        network-src/themediumtcpserver.cpp \
        network-types/tcpmediumtypeconverter.cpp \
        tcpservermediumlocalsocket.cpp \
        thelordofservicescover.cpp


HEADERS += \
    network-src/tcpmediumsocket.h \
    network-src/thelordofservices.h \
    network-src/themediumtcpserver.h \
    network-types/tcpmediumtypeconverter.h \
    network-types/tcpservermediumtypes.h \
    tcpservermediumlocalsocket.h \
    thelordofservicescover.h
