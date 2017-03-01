QT += core network
include(../default.pri)
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LauncherLib
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$PWD/src

SOURCES     += $$PWD/src/*.cpp \
    src/apioperator.cpp
SOURCES     += $$PWD/src/*.c
HEADERS     += $$PWD/src/*.h \
    src/apioperator.h
