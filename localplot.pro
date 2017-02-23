#-------------------------------------------------
#
# Project created by QtCreator 2015-12-17T21:05:53
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = localplot
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    dialogabout.cpp \
    dialogsettings.cpp \
    settings.cpp \
    ancilla.cpp \
    etc.cpp

HEADERS  += mainwindow.h \
    dialogabout.h \
    dialogsettings.h \
    settings.h \
    ancilla.h \
    etc.h

FORMS    += mainwindow.ui \
    dialogabout.ui \
    dialogsettings.ui

DISTFILES += \
    hpgl_filec \
    hpgl_fileh
