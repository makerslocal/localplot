#-------------------------------------------------
#
# Project created by QtCreator 2015-12-17T21:05:53
#
#-------------------------------------------------

QT       += core gui serialport designer network svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = localplot
TEMPLATE = app

INCLUDEPATH += qmqtt-master/src/mqtt/

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
	extbinpack.cpp \
    qmqtt-master/src/mqtt/qmqtt_client_p.cpp \
    qmqtt-master/src/mqtt/qmqtt_client.cpp \
    qmqtt-master/src/mqtt/qmqtt_frame.cpp \
    qmqtt-master/src/mqtt/qmqtt_message_p.cpp \
    qmqtt-master/src/mqtt/qmqtt_message.cpp \
    qmqtt-master/src/mqtt/qmqtt_network.cpp \
    qmqtt-master/src/mqtt/qmqtt_routedmessage.cpp \
    qmqtt-master/src/mqtt/qmqtt_router.cpp \
    qmqtt-master/src/mqtt/qmqtt_routesubscription.cpp \
    qmqtt-master/src/mqtt/qmqtt_socket.cpp \
    qmqtt-master/src/mqtt/qmqtt_ssl_network.cpp \
    qmqtt-master/src/mqtt/qmqtt_ssl_socket.cpp \
    qmqtt-master/src/mqtt/qmqtt_timer.cpp

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
	extbinpack.h \
    qmqtt-master/src/mqtt/qmqtt_client_p.h \
    qmqtt-master/src/mqtt/qmqtt_client.h \
    qmqtt-master/src/mqtt/qmqtt_frame.h \
    qmqtt-master/src/mqtt/qmqtt_global.h \
    qmqtt-master/src/mqtt/qmqtt_message_p.h \
    qmqtt-master/src/mqtt/qmqtt_message.h \
    qmqtt-master/src/mqtt/qmqtt_network_p.h \
    qmqtt-master/src/mqtt/qmqtt_networkinterface.h \
    qmqtt-master/src/mqtt/qmqtt_routedmessage.h \
    qmqtt-master/src/mqtt/qmqtt_router.h \
    qmqtt-master/src/mqtt/qmqtt_routesubscription.h \
    qmqtt-master/src/mqtt/qmqtt_socket_p.h \
    qmqtt-master/src/mqtt/qmqtt_socketinterface.h \
    qmqtt-master/src/mqtt/qmqtt_ssl_network_p.h \
    qmqtt-master/src/mqtt/qmqtt_ssl_socket_p.h \
    qmqtt-master/src/mqtt/qmqtt_timer_p.h \
    qmqtt-master/src/mqtt/qmqtt_timerinterface.h \
    qmqtt-master/src/mqtt/qmqtt.h

FORMS    += mainwindow.ui \
	dialogabout.ui \
	dialogsettings.ui

SUBDIRS += \
	qmqtt-master/src/mqtt/mqtt.pro

RESOURCES += \
    icons.qrc
