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
	extplot.cpp \
	exteta.cpp \
	extloadfile.cpp \
	RectangleBinPack/ShelfBinPack.cpp \
	RectangleBinPack/GuillotineBinPack.cpp \
	extbinpack.cpp

HEADERS  += mainwindow.h \
	dialogabout.h \
	dialogsettings.h \
	settings.h \
	hpglgraphicsview.h \
	hpgllistmodel.h \
	extplot.h \
	exteta.h \
	extloadfile.h \
	RectangleBinPack/Rect.h \
	RectangleBinPack/ShelfBinPack.h \
	RectangleBinPack/GuillotineBinPack.h \
	extbinpack.h

FORMS    += mainwindow.ui \
	dialogabout.ui \
	dialogsettings.ui

RESOURCES += \
    icons.qrc
