#ifndef SETTINGS_H
#define SETTINGS_H

#include <QCoreApplication>

void init_localplot_settings();

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
 * - port
 * - - name (string)
 * - - index (int)
 * - parity (string)
 * - baud (int)
 * - bytesize (int)
 * - stopbits (int)
 * - xonxoff (bool)
 * - rtscts (bool)
 * - dsrdtr (bool)
 *
 * cutter
 * - incremental (bool)
 * - speed (int)
 *
 * mainwindow
 * - filePath (string)
 */

#endif // SETTINGS_H
