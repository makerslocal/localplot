#ifndef SETTINGS_H
#define SETTINGS_H

#include <QCoreApplication>
#include <QDebug>

void init_localplot_settings();

/**
 * Settings Identifiers
 */
#define ORGANIZATION_NAME   ("Makers Local 256")
#define ORGANIZATION_DOMAIN ("256.makerslocal.org")
#define APPLICATION_NAME    ("localplot")

/**
 * Settings Defaults
 */
#define SETDEF_PEN_DOWN_SIZE    2
#define SETDEF_PEN_DOWN_RED     100
#define SETDEF_PEN_DOWN_GREEN   150
#define SETDEF_PEN_DOWN_BLUE    200

#define SETDEF_PEN_UP_SIZE    1
#define SETDEF_PEN_UP_RED     250
#define SETDEF_PEN_UP_GREEN   150
#define SETDEF_PEN_UP_BLUE    150

#define SETDEF_DEVICE_INCREMENTAL true
#define SETDEF_DEVICE_SPEED_CUT 80
#define SETDEF_DEVICE_SPEED_TRAVEL 150

#define SETDEF_MAINWINDOW_FILEPATH ""

#define SETDEF_SERIAL_PORT ""
#define SETDEF_SERIAL_BAUD 9600
#define SETDEF_SERIAL_BYTESIZE 8
#define SETDEF_SERIAL_PARITY "none"
#define SETDEF_SERIAL_STOPBITS 1
#define SETDEF_SERIAL_XONOFF false
#define SETDEF_SERIAL_RTSCTS false

/**
 * Macros
 */
#define CUTSPEED settings->value("device/speed/cut", SETDEF_DEVICE_SPEED_CUT).toInt()
#define TRAVELSPEED settings->value("device/speed/travel", SETDEF_DEVICE_SPEED_TRAVEL).toInt()

/*
 * Current Settings Paths:
 *
 * pen
 * - down
 * - - size, red, green, blue (int)
 * - up
 * - - size, red, green, blue (int)
 *
 * serial
 * - port (string)
 * - parity (string)
 * - baud (int)
 * - bytesize (int)
 * - stopbits (int)
 * - xonxoff (bool)
 * - rtscts (bool)
 *
 * device
 * - incremental (bool)
 * - speed
 * - - cut (int)
 * - - travel (int)
 *
 * mainwindow
 * - filePath (string)
 */

#endif // SETTINGS_H
