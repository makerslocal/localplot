#-------------------------------------------------
#
# Project created by QtCreator 2015-12-17T21:05:53
#
#-------------------------------------------------

QT       += core gui serialport designer svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = localplot
TEMPLATE = app

SOURCES += main.cpp\
		mainwindow.cpp \
	settings.cpp \
	hpglgraphicsview.cpp \
	hpgllistmodel.cpp \
	RectangleBinPack/ShelfBinPack.cpp \
	RectangleBinPack/GuillotineBinPack.cpp \
    RectangleBinPack/MaxRectsBinPack.cpp \
    RectangleBinPack/Rect.cpp \
    ext/plot.cpp \
    ext/binpack.cpp \
    ext/eta.cpp \
    ext/loadfile.cpp \
    dialog/dialogabout.cpp \
    dialog/dialogprogress.cpp \
    dialog/dialogsettings.cpp

HEADERS  += mainwindow.h \
	settings.h \
	hpglgraphicsview.h \
	hpgllistmodel.h \
	RectangleBinPack/Rect.h \
	RectangleBinPack/ShelfBinPack.h \
	RectangleBinPack/GuillotineBinPack.h \
    RectangleBinPack/MaxRectsBinPack.h \
    ext/plot.h \
    ext/binpack.h \
    ext/eta.h \
    ext/loadfile.h \
    dialog/dialogabout.h \
    dialog/dialogprogress.h \
    dialog/dialogsettings.h

FORMS    += mainwindow.ui \
    dialog/dialogabout.ui \
    dialog/dialogprogress.ui \
    dialog/dialogsettings.ui

RESOURCES += \
    icons.qrc \
    images.qrc
