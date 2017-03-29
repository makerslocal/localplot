/**
 * Settings - helper code for using QSettings
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef SETTINGS_H
#define SETTINGS_H

#include <QCoreApplication>
#include <QtCore>

void init_localplot_settings();

/**
 * Settings Identifiers
 */
#define ORGANIZATION_NAME   ("Makers Local 256")
#define ORGANIZATION_DOMAIN ("256.makerslocal.org")
#define APPLICATION_NAME    ("localplot")

// Data type of device width setting
// http://stackoverflow.com/a/9150607/1349825
enum deviceWidth_t {
    INCH = 0,
    CM,
    SIZE_OF_ENUM
};
static const char* deviceWidth_names[] = {"inch", "cm"};

// statically check that the size of ColorNames fits the number of Colors
  static_assert(sizeof(deviceWidth_names)/sizeof(char*) == deviceWidth_t::SIZE_OF_ENUM
    , "Settings device width sizes dont match");

/**
 * Settings Defaults
 */
#define SETDEF_PEN_DOWN_SIZE    (1)
#define SETDEF_PEN_DOWN_RED     (100)
#define SETDEF_PEN_DOWN_GREEN   (150)
#define SETDEF_PEN_DOWN_BLUE    (200)

#define SETDEF_PEN_UP_SIZE    (1)
#define SETDEF_PEN_UP_RED     (250)
#define SETDEF_PEN_UP_GREEN   (150)
#define SETDEF_PEN_UP_BLUE    (150)

#define SETDEF_DEVICE_INCREMENTAL   (true)
#define SETDEF_DEVICE_SPEED_CUT     (80)
#define SETDEF_DEVICE_SPEED_TRAVEL  (150)
#define SETDEF_DEVICE_WIDTH         (36)
#define SETDEF_DEVICE_WDITH_TYPE    (deviceWidth_t::INCH)
#define SETDEF_DEVICE_CUTOUTBOXES   (false)
#define SETDEF_DEVICE_CUTOUTBOXES_PADDING (0.25)

#define SETDEF_MAINWINDOW_FILEPATH  ("")
#define SETDEF_MAINWINDOW_GRID (true)
#define SETDEF_MAINWINDOW_GRID_SIZE (1)
#define SETDEF_DIALLOGSETTINGS_INDEX (1)
#define SETDEF_MAINWINDOW_COMMAND_FINISHED ("echo -ne '\\a'")

#define SETDEF_SERIAL_PORT      ("")
#define SETDEF_SERIAL_BAUD      (9600)
#define SETDEF_SERIAL_BYTESIZE  (8)
#define SETDEF_SERIAL_PARITY    ("none")
#define SETDEF_SERIAL_STOPBITS  (1)
#define SETDEF_SERIAL_XONOFF    (false)
#define SETDEF_SERIAL_RTSCTS    (false)

/**
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
 * - width (int)
 * - - type (enum)
 * - cutoutboxes (bool)
 * - - padding (double)
 *
 * mainwindow
 * - filePath (string)
 * - windowState (bytearray)
 * - geometry (bytearray)
 * - splitter
 * - - state (bytearray)
 * - - geometry (bytearray)
 * - grid (bool)
 * - - size (int)
 * - command
 * - - finished (string)
 *
 * dialogsettings
 * - index (int)
 * - geometry (bytearray)
 *
 * dialogabout
 * - geometry (bytearray)
 */

#endif // SETTINGS_H
