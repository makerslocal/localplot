#include "hpgl_cmd.h"

/**
 * We process the following opcodes:
 * IN - begin/reset
 * SP - Select Pen
 * PU - Move to coord with pen up
 * PD - Move to coord with pen down
 */

hpgl_cmd::hpgl_cmd()
{
    _opcode = "NA";
    coordList.clear();
    pen = 0;
}

hpgl_cmd::hpgl_cmd(QString newopcode, QList<QPoint> newList)
{
    // Make sure the pointers are null(?)
    hpgl_cmd();

    _opcode = newopcode;
    coordList = newList;
}

hpgl_cmd::~hpgl_cmd()
{
    //
}

QString hpgl_cmd::opcode()
{
    return(_opcode);
}

int hpgl_cmd::get_pen()
{
    return(pen);
}


QList<QPoint> hpgl_cmd::point_list()
{
    return coordList;
}
