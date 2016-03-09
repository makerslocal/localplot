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
