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
        QString cmdText;
        QString opCode;
        int pen;
        QList<QPoint> coordList;
        cmdText = text.section(';', i, i);

        qDebug() << "====\n" << "= Processing command: ";

        // Get opcode, first two characters
        opCode = cmdText.mid(0, 2);

        // Parse opcode
        if (opCode == "IN")
        {
            // Just opcode
            qDebug() << "= IN";
        }
        else if (opCode == "SP")
        {
            pen = cmdText.mid(2,1).toInt();
            qDebug() << "= SP[" << QString::number(pen) << "]";
        }
        else if (opCode == "PU" || opCode == "PD")
        {
            qDebug() << "= " << opCode;
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
                coordList.push_back(QPoint(newX, newY));
            }
        }

        cmdList.push_back(hpgl_cmd(opCode, coordList));
    }
}

//hpgl_cmd::hpgl_cmd(QString text)
//{
//    // Make sure the pointers are null(?)
//    hpgl_cmd();

//    qDebug() << "====\n" << "= Processing command: ";

//    // Get opcode, first two characters
//    _opcode = text.mid(0, 2);

//    // Parse opcode
//    if (_opcode == "IN")
//    {
//        // Just opcode
//        qDebug() << "= IN";
//    }
//    else if (_opcode == "SP")
//    {
//        pen = text.mid(2,1).toInt();
//        qDebug() << "= SP[" << QString::number(pen) << "]";
//    }
//    else if (_opcode == "PU" || _opcode == "PD")
//    {
//        qDebug() << "= " << _opcode;
//        // need coords
//        text.remove(0,2);
//        int commaCount = text.count(',');
//        qDebug() << "= Comma count: " << commaCount;
//        for (int i = 0; i < commaCount; i++)
//        {
//            //qDebug() << "processing coord: " << text.at(i) << endl;
//            int newX = text.section(',', i, i).toInt();
//            i++;
//            int newY = text.section(',', i, i).toInt();
//            //i++;
//            qDebug() << "= Found x: " << newX << " y: " << newY;
//            coordList.push_back(QPoint(newX, newY));
//        }
//    }
//}

hpgl_obj::~hpgl_obj()
{
    //
}

void hpgl_obj::gen_line_lists()
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
        if (cmdList[i].opcode() != "PU" && cmdList[i].opcode() != "PD")
        {
            continue; // only point based commands, otherwise skip
        }
        pointList = cmdList[i].point_list();
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
        if (cmdList[i].opcode() == "PD")
        {
            lineListDown += tempLineList;
        }
        else if (cmdList[i].opcode() == "PU")
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

void hpgl_obj::gen_width(QList<QLine> lineList)
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

void hpgl_obj::gen_height(QList<QLine> lineList)
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

//int hpgl_obj::printLen()
//{
//    int retval = 0;
//    hpgl_cmd cmd;
//    for (int i = 0; i < cmdList.length(); i++)
//    {
//        cmd = cmdList.at(i);
//        retval += cmd.printLen();
//    }
//    return(retval);
//}

QString hpgl_obj::print()
{
    QString retval = "";
    hpgl_cmd cmd;
    for (int i = 0; i < cmdList.length(); i++)
    {
        cmd = cmdList.at(i);

        retval += cmd.opcode();
        if (cmd.opcode() == "SP")
        {
            retval += QString::number(cmd.get_pen());
        }
        else
        {
            for (int i = 0; i < cmd.point_list().length(); i++)
            {
                QPoint point = cmd.point_list().at(i);

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
                if (i < (cmd.point_list().length()-1))
                {
                    retval += ",";
                }
            }
        }
        retval += ";";
    }
    return(retval);
}

//QString hpgl_cmd::print(QTransform transform /*= QTransform()*/)
//{
//    QString retval = "";
//    retval += _opcode;
//    if (_opcode == "SP")
//    {
//        retval += QString::number(pen);
//    }
//    else
//    {
//        for (int i = 0; i < coordList.length(); i++)
//        {
//            QPoint point = coordList[i];
//            point = transform.map(point);
//            retval += QString::number(point.x());
//            retval += ",";
//            retval += QString::number(point.y());
//            if (i < (coordList.length()-1))
//            {
//                retval += ",";
//            }
//        }
//    }
//    retval += ";";
//    return(retval);
//}












