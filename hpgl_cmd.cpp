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
    opcode = "NA";
    verts.clear();
    pen = 0;
}

hpgl_cmd::hpgl_cmd(QString text)
{
    // Make sure the pointers are null(?)
    hpgl_cmd();

    qDebug() << "Processing command: ";

    // Get opcode, first two characters
    opcode = text.mid(0, 2);

    // Parse opcode
    if (opcode == "IN")
    {
        // Just opcode
        qDebug() << "IN";
    }
    else if (opcode == "SP")
    {
        pen = text.mid(2,1).toInt();
        qDebug() << "SP[" << QString::number(pen) << "]";
    }
    else if (opcode == "PU" || opcode == "PD")
    {
        qDebug() << opcode;
        // need coords
        text.remove(0,2);
        int commaCount = text.count(',');
        for (int i = 0; i < commaCount; i++)
        {
            //qDebug() << "processing coord: " << text.at(i) << endl;
            int newX = text.section(',', i, i).toInt();
            i++;
            int newY = text.section(',', i, i).toInt();
            //i++;
            qDebug() << "Found x: " << newX << " y: " << newY;
            verts.push_back(hpgl_coord(newX, newY));
        }
    }
}

hpgl_cmd::~hpgl_cmd()
{
    //
}

QString hpgl_cmd::print()
{
    QString retval = "";
    retval += opcode;
    for (int i = 0; i < verts.length(); i++)
    {
        hpgl_coord tmp = verts[i];
        retval += tmp.print();
        if (i < (verts.length()-1))
        {
            retval += ",";
        }
    }
    retval += ";";
    return(retval);
}
