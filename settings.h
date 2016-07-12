#ifndef SETTINGS_H
#define SETTINGS_H

#include <QCoreApplication>

void init_localplot_settings();

/*
 * Current Settings:
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
 * - parity (int)
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
 */

#endif // SETTINGS_H
