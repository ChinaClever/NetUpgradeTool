#-------------------------------------------------
#
# Project created by QtCreator 2019-04-03T14:26:10
#
#-------------------------------------------------

QT       += core gui
QT      += printsupport
QT      += sql
QT      += network
QT      += charts
QT      += serialport
QT      += websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets



TARGET = CleverUpgradeTool_V1.8
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11
# Cryptopp使用方法链接：https://blog.zhengjunxin.com/archives/996/
# https://github.com/weidai11/cryptopp
# 使用了英特尔官方指令集用于优化算法速度，以下是缺少的：
QMAKE_CXXFLAGS += "-mssse3"
QMAKE_CXXFLAGS += "-msse4.1"
QMAKE_CXXFLAGS += "-mavx2"
QMAKE_CXXFLAGS += "-mpclmul"
QMAKE_CXXFLAGS += "-maes"
QMAKE_CXXFLAGS += "-msha"
# 支持OpenMP特性，发挥算法的最大潜能
LIBS += -lgomp -lpthread
QMAKE_CXXFLAGS += "-fopenmp"


#TRANSLATIONS = English.ts
TRANSLATIONS += english.ts chinese.ts
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
include(common/common.pri)

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    udptesting.cpp \
    devselectwid.cpp

HEADERS += \
        mainwindow.h \
    udptesting.h \
    devselectwid.h

FORMS += \
        mainwindow.ui \
    devselectwid.ui

RESOURCES += \
    images/images.qrc

RC_ICONS = logo.ico
