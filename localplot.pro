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
	dialogabout.cpp \
	dialogsettings.cpp \
	settings.cpp \
	hpglgraphicsview.cpp \
	hpgllistmodel.cpp \
	RectangleBinPack/ShelfBinPack.cpp \
	RectangleBinPack/GuillotineBinPack.cpp \
    dialogprogress.cpp \
    RectangleBinPack/MaxRectsBinPack.cpp \
    RectangleBinPack/Rect.cpp \
    ext/plot.cpp \
    ext/binpack.cpp \
    ext/eta.cpp \
    ext/loadfile.cpp

HEADERS  += mainwindow.h \
	dialogabout.h \
	dialogsettings.h \
	settings.h \
	hpglgraphicsview.h \
	hpgllistmodel.h \
	RectangleBinPack/Rect.h \
	RectangleBinPack/ShelfBinPack.h \
	RectangleBinPack/GuillotineBinPack.h \
    dialogprogress.h \
    RectangleBinPack/MaxRectsBinPack.h \
    ext/plot.h \
    ext/binpack.h \
    ext/eta.h \
    ext/loadfile.h

FORMS    += mainwindow.ui \
	dialogabout.ui \
	dialogsettings.ui \
    dialogprogress.ui

RESOURCES += \
    icons.qrc \
    images.qrc
