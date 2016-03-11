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

hpgl_cmd::hpgl_cmd(QString text)
{
    // Make sure the pointers are null(?)
    hpgl_cmd();

    qDebug() << "Processing command: ";

    // Get opcode, first two characters
    _opcode = text.mid(0, 2);

    // Parse opcode
    if (_opcode == "IN")
    {
        // Just opcode
        qDebug() << "IN";
    }
    else if (_opcode == "SP")
    {
        pen = text.mid(2,1).toInt();
        qDebug() << "SP[" << QString::number(pen) << "]";
    }
    else if (_opcode == "PU" || _opcode == "PD")
    {
        qDebug() << _opcode;
        // need coords
        text.remove(0,2);
        int commaCount = text.count(',');
        qDebug() << "Comma count: " << commaCount;
        for (int i = 0; i < commaCount; i++)
        {
            //qDebug() << "processing coord: " << text.at(i) << endl;
            int newX = text.section(',', i, i).toInt();
            i++;
            int newY = text.section(',', i, i).toInt();
            //i++;
            qDebug() << "Found x: " << newX << " y: " << newY;
            coordList.push_back(QPoint(newX, newY));
        }
    }
}

hpgl_cmd::~hpgl_cmd()
{
    //
}

QString hpgl_cmd::opcode()
{
    return(_opcode);
}

QList<QPoint> hpgl_cmd::get_verts()
{
    return coordList;
}

QList<QLine> hpgl_cmd::line_list()
{
    QList<QLine> lineList;
    lineList.clear();
    int x1, y1, x2, y2;
    for (int i = 0; i < (coordList.length()-1); i++)
    {
        x1 = coordList[i].x();
        y1 = coordList[i].y();
        x2 = coordList[i+1].x();
        y2 = coordList[i+1].y();
        lineList.push_back(QLine(x1, y1, x2, y2));
    }
    return lineList;
}

int hpgl_cmd::printLen()
{
    QString check = print();
    return check.length();
}

QString hpgl_cmd::print()
{
    QString retval = "";
    retval += _opcode;
    if (_opcode == "SP")
    {
        retval += QString::number(pen);
    }
    else
    {
        for (int i = 0; i < coordList.length(); i++)
        {
            QPoint tmp = coordList[i];
            retval += QString::number(tmp.x());
            retval += ",";
            retval += QString::number(tmp.y());
            if (i < (coordList.length()-1))
            {
                retval += ",";
            }
        }
    }
    retval += ";";
    return(retval);
}
