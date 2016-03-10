#include "hpgl_obj.h"

/**
 * Object class
 */

hpgl_obj::hpgl_obj()
{
    cmdList.clear();
}

hpgl_obj::hpgl_obj(QString text)
{
    // Initialize
    hpgl_obj();

    text.remove('\n');
    int numCmds = text.count(';');
    qDebug() << "Object text: " << text;
    for (int i = 0; i < numCmds; i++)
    {
        QString tmp;
        tmp = text.section(';', i, i);
        cmdList.push_back(hpgl_cmd(tmp));
//        qDebug() << "Just added: " << cmdList.back().print();
    }
}

hpgl_obj::~hpgl_obj()
{
    //
}

QList<QLine> hpgl_obj::line_list_up()
{
    QList<QLine> lineList;
    lineList.clear();
    QPoint lastPoint = QPoint(0, 0);
    for (int i = 0; i < cmdList.length(); i++)
    {
        if (cmdList[i].opcode() == "PU")
        {
            if (cmdList[i].line_list().isEmpty())
            {
                hpgl_cmd tmp = cmdList.at(i);
                int end_x = tmp.get_verts().last().get_x();
                int end_y = tmp.get_verts().last().get_y();
                QLine implicitLine = QLine(lastPoint.x(), lastPoint.y(),end_x , end_y);
                lineList.push_back(implicitLine);
            }
            else if (lastPoint != cmdList[i].line_list().last().p1())
            {
                lineList.push_back(QLine(lastPoint, cmdList[i].line_list().last().p1()));
            }
            lineList += cmdList[i].line_list();
        }
        if (!lineList.isEmpty())
        {
            lastPoint = lineList.last().p2();
        }
    }
    return lineList;
}

QList<QLine> hpgl_obj::line_list_down()
{
    QList<QLine> lineList;
    lineList.clear();
    for (int i = 0; i < cmdList.length(); i++)
    {
        if (cmdList[i].opcode() == "PD")
        {
            lineList += cmdList[i].line_list();
        }
    }
    return lineList;
}

int hpgl_obj::printLen()
{
    int retval = 0;
    hpgl_cmd cmd;
    for (int i = 0; i < cmdList.length(); i++)
    {
        cmd = cmdList.at(i);
        retval += cmd.printLen();
    }
    return(retval);
}

QString hpgl_obj::print()
{
    QString retval = "";
    hpgl_cmd cmd;
    for (int i = 0; i < cmdList.length(); i++)
    {
        cmd = cmdList.at(i);
        retval += cmd.print();
    }
    return(retval);
}
