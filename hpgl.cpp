/**
 * HPGL_OBJ - Data structure for plotter
 * Christopher Bero <bigbero@gmail.com>
 */
#include "hpgl.h"

/**
 * We process the following opcodes:
 * IN - begin/reset
 * SP - Select Pen
 * PU - Move to coord with pen up
 * PD - Move to coord with pen down
 */

hpgl::hpgl()
{
    //
}

hpgl::hpgl(QString hpgl_text)
{
    // Initialize
    hpgl();

    parseHPGL(hpgl_text);
}

hpgl::~hpgl()
{
    //
}

void hpgl::gen_line_lists()
{
    lineListUp.clear();
    lineListDown.clear();
    QPoint lastPoint = QPoint(0, 0);
    QList<QPoint> pointList;
    QList<QLine> tempLineList;
    pointList.clear();
    tempLineList.clear();

    for (int i = 0; i < cmdList.length(); i++)
    {
        if (cmdList[i].opcode != "PU" && cmdList[i].opcode != "PD")
        {
            continue; // only point based commands, otherwise skip
        }
        pointList = cmdList[i].coordList;
        tempLineList.clear();
        // First line is always extension of previous last point
        tempLineList.push_back(QLine(lastPoint, pointList[0]));

        for (int c = 0; c < (pointList.length()-1); c++)
        {
            QLine tempLine(pointList[c], pointList[c+1]);
            tempLineList.push_back(tempLine);
        }

        // Remove zero-length lines
//        for (int c = 0; c < tempLineList.length(); c++)
//        {
//            if (tempLineList[c].p1() == tempLineList[c].p2())
//            {
//                tempLineList.removeAt(c);
//            }
//        }

        // Assign to corresponding list
        if (cmdList[i].opcode == "PD")
        {
            lineListDown += tempLineList;
        }
        else if (cmdList[i].opcode == "PU")
        {
            lineListUp += tempLineList;
        }
        lastPoint = pointList.last();
    }

    QTransform center, uncenter;
    gen_width(lineListDown);
    gen_height(lineListDown);
    center.translate(-width/2, -height/2);
    for (int c = 0; c < lineListUp.length(); c++)
    {
        if (lineListUp[c].p1() == QPoint(0, 0))
        {
            QPoint newPt = lineListUp[c].p2();
            newPt = center.map(newPt);
            newPt = cmdTransformScale.map(newPt);
            newPt = cmdTransformRotate.map(newPt);
            newPt = cmdTransformTranslate.map(newPt);
            lineListUp[c].setP2(newPt);
            continue;
        }
        if (lineListUp[c].p2() == QPoint(0, 0))
        {
            QPoint newPt = lineListUp[c].p1();
            newPt = center.map(newPt);
            newPt = cmdTransformScale.map(newPt);
            newPt = cmdTransformRotate.map(newPt);
            newPt = cmdTransformTranslate.map(newPt);
            lineListUp[c].setP1(newPt);
            continue;
        }
        lineListUp[c] = center.map(lineListUp[c]);
        lineListUp[c] = cmdTransformScale.map(lineListUp[c]);
        lineListUp[c] = cmdTransformRotate.map(lineListUp[c]);
        lineListUp[c] = cmdTransformTranslate.map(lineListUp[c]);
    }
    for (int c = 0; c < lineListDown.length(); c++)
    {
        lineListDown[c] = center.map(lineListDown[c]);
        lineListDown[c] = cmdTransformScale.map(lineListDown[c]);
        lineListDown[c] = cmdTransformRotate.map(lineListDown[c]);
        lineListDown[c] = cmdTransformTranslate.map(lineListDown[c]);
    }
    gen_width(lineListDown);
    gen_height(lineListDown);
    uncenter.translate(width/2, height/2);
    for (int c = 0; c < lineListUp.length(); c++)
    {
        if (lineListUp[c].p1() == QPoint(0, 0))
        {
            QPoint newPt = lineListUp[c].p2();
            newPt = uncenter.map(newPt);
            lineListUp[c].setP2(newPt);
            continue;
        }
        if (lineListUp[c].p2() == QPoint(0, 0))
        {
            QPoint newPt = lineListUp[c].p1();
            newPt = uncenter.map(newPt);
            lineListUp[c].setP1(newPt);
            continue;
        }
        lineListUp[c] = uncenter.map(lineListUp[c]);
    }
    for (int c = 0; c < lineListDown.length(); c++)
    {
        lineListDown[c] = uncenter.map(lineListDown[c]);
    }
}

void hpgl::gen_width(QList<QLine> lineList)
{
    int maxX = 0;
    int minX = INT_MAX;
    int x1, x2;

    for (int i = 0; i < lineList.length(); i++)
    {
        x1 = lineList[i].x1();
        x2 = lineList[i].x2();
        if (x1 > maxX)
        {
            maxX = x1;
        }
        if (x1 < minX)
        {
            minX = x1;
        }
        if (x2 > maxX)
        {
            maxX = x2;
        }
        if (x2 < minX)
        {
            minX = x2;
        }
    }
    width = maxX - minX;
    //return(maxX - minX);
}

void hpgl::gen_height(QList<QLine> lineList)
{
    int maxY = 0;
    int minY = INT_MAX;

    for (int i = 0; i < lineList.length(); i++)
    {
        int y1, y2;
        y1 = lineList[i].y1();
        y2 = lineList[i].y2();
        if (y1 > maxY)
        {
            maxY = y1;
        }
        if (y1 < minY)
        {
            minY = y1;
        }
        if (y2 > maxY)
        {
            maxY = y2;
        }
        if (y2 < minY)
        {
            minY = y2;
        }
    }
    height = maxY - minY;
    //return(maxY - minY);
}

hpgl_cmd hpgl::cmdGet(int cmd_index)
{
    return(cmdList.at(cmd_index));
}

int hpgl::cmdCount()
{
    return(cmdList.length());
}

int hpgl::totalMM()
{
    QPoint prev;
    QPoint curr;
    hpgl_cmd cmd;
    int mm;
    prev.setX(0);
    prev.setY(0);
    for (int idx = 0; idx < cmdList.length(); idx++)
    {
        cmd = cmdList[idx];
        if (cmd.opcode != "SP")
        {
            for (int i = 0; i < cmd.coordList.length(); i++)
            {
                int x, y;
                curr = cmd.coordList.at(i);
                x = curr.x() - prev.x();
                y = curr.y() - prev.y();
                mm += sqrt(x*x + y*y);
            }
        }

    }
    mm = mm * 0.025; // convert graphics units to mm
    return(mm);
}

int hpgl::printLen()
{
    return(print().length());
}

QString hpgl::print()
{
    QString retval = "";
    hpgl_cmd cmd;
    for (int idx = 0; idx < cmdList.length(); idx++)
    {
        cmd = cmdList[idx];

        retval += cmd.opcode;
        if (cmd.opcode == "SP")
        {
            retval += QString::number(cmd.pen);
        }
        else
        {
            for (int i = 0; i < cmd.coordList.length(); i++)
            {
                QPoint point = cmd.coordList.at(i);

                QTransform center, uncenter;
                center.translate(-width/2, -height/2);
                uncenter.translate(width/2, height/2);

                point = cmdTransformScale.map(point);
                point = cmdTransformRotate.map(point);
                point = cmdTransformTranslate.map(point);

                if (point.x() < 0 || point.y() < 0)
                {
                    retval = "OOB"; // Out of Bounds
                    return retval;
                }

                retval += QString::number(point.x());
                retval += ",";
                retval += QString::number(point.y());
                if (i < (cmd.coordList.length()-1))
                {
                    retval += ",";
                }
            }
        }
        retval += ";";
    }
    return(retval);
}

int hpgl::load_file(QString _filepath)
{
    QFile inputFile;
    QString buffer;

    if (_filepath.isEmpty())
    {
        return;
    }
    inputFile.setFileName(_filepath);
    inputFile.open(QIODevice::ReadOnly);
    clear_cmds();

    QTextStream fstream(&inputFile);
    buffer = fstream.readAll();

    parseHPGL(buffer);
    inputFile.close();
}

void hpgl::parseHPGL(QString hpgl_text)
{
    hpgl_text.remove('\n');
    int numCmds = hpgl_text.count(';');
    for (int i = 0; i < numCmds; i++)
    {
        QPointer<hpgl_cmd> newCmd;
        newCmd = new hpgl_cmd();
        QString cmdText;
        cmdText = hpgl_text.section(';', i, i);

        qDebug() << "====\n" << "= Processing command: ";

        // Get opcode, first two characters
        newCmd->set_opcode(cmdText.mid(0, 2));

        // Parse opcode
        if (newCmd->get_opcode() == "IN")
        {
            // Just opcode
            qDebug() << "= IN";
        }
        else if (newCmd->get_opcode() == "SP")
        {
            newCmd->set_pen(cmdText.mid(2,1).toInt());
            qDebug() << "= SP[" << QString::number(newCmd.pen) << "]";
        }
        else if (newCmd->get_opcode() == "PU" || newCmd->get_opcode() == "PD")
        {
            qDebug() << "= " << newCmd->get_opcode();
            // need coords
            cmdText.remove(0,2);
            int commaCount = cmdText.count(',');
            qDebug() << "= Comma count: " << commaCount;
            for (int i = 0; i < commaCount; i++)
            {
                //qDebug() << "processing coord: " << text.at(i) << endl;
                int newX = cmdText.section(',', i, i).toInt();
                i++;
                int newY = cmdText.section(',', i, i).toInt();
                //i++;
                qDebug() << "= Found x: " << newX << " y: " << newY;
                newCmd->add_coord(QPoint(newX, newY));
//                if (i < (commaCount-2) && ((i+1) % 500) == 0)
//                {
//                    qDebug() << "Breaking line";
//                    cmdList.push_back(newCmd);
//                    newCmd.coordList.clear();
////                    newCmd.coordList.push_back(QPoint(newX, newY));
//                }
            }
        }
        cmdList.push_back(newCmd);
    }
}

void hpgl::clear_cmds()
{
    for (int i = 0; i < cmdList.count(); i++)
    {
        delete cmdList[i];
    }
    cmdList.clear();
}





















