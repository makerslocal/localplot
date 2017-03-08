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
//    lineListUp.clear();
//    lineListDown.clear();
//    QPoint lastPoint = QPoint(0, 0);
//    QList<QPoint> pointList;
//    QList<QLine> tempLineList;
//    pointList.clear();
//    tempLineList.clear();

//    for (int i = 0; i < cmdList.length(); i++)
//    {
//        if (cmdList[i].opcode != "PU" && cmdList[i].opcode != "PD")
//        {
//            continue; // only point based commands, otherwise skip
//        }
//        pointList = cmdList[i].coordList;
//        tempLineList.clear();
//        // First line is always extension of previous last point
//        tempLineList.push_back(QLine(lastPoint, pointList[0]));

//        for (int c = 0; c < (pointList.length()-1); c++)
//        {
//            QLine tempLine(pointList[c], pointList[c+1]);
//            tempLineList.push_back(tempLine);
//        }

//        // Remove zero-length lines
////        for (int c = 0; c < tempLineList.length(); c++)
////        {
////            if (tempLineList[c].p1() == tempLineList[c].p2())
////            {
////                tempLineList.removeAt(c);
////            }
////        }

//        // Assign to corresponding list
//        if (cmdList[i].opcode == "PD")
//        {
//            lineListDown += tempLineList;
//        }
//        else if (cmdList[i].opcode == "PU")
//        {
//            lineListUp += tempLineList;
//        }
//        lastPoint = pointList.last();
//    }

//    QTransform center, uncenter;
//    gen_width(lineListDown);
//    gen_height(lineListDown);
//    center.translate(-width/2, -height/2);
//    for (int c = 0; c < lineListUp.length(); c++)
//    {
//        if (lineListUp[c].p1() == QPoint(0, 0))
//        {
//            QPoint newPt = lineListUp[c].p2();
//            newPt = center.map(newPt);
//            newPt = cmdTransformScale.map(newPt);
//            newPt = cmdTransformRotate.map(newPt);
//            newPt = cmdTransformTranslate.map(newPt);
//            lineListUp[c].setP2(newPt);
//            continue;
//        }
//        if (lineListUp[c].p2() == QPoint(0, 0))
//        {
//            QPoint newPt = lineListUp[c].p1();
//            newPt = center.map(newPt);
//            newPt = cmdTransformScale.map(newPt);
//            newPt = cmdTransformRotate.map(newPt);
//            newPt = cmdTransformTranslate.map(newPt);
//            lineListUp[c].setP1(newPt);
//            continue;
//        }
//        lineListUp[c] = center.map(lineListUp[c]);
//        lineListUp[c] = cmdTransformScale.map(lineListUp[c]);
//        lineListUp[c] = cmdTransformRotate.map(lineListUp[c]);
//        lineListUp[c] = cmdTransformTranslate.map(lineListUp[c]);
//    }
//    for (int c = 0; c < lineListDown.length(); c++)
//    {
//        lineListDown[c] = center.map(lineListDown[c]);
//        lineListDown[c] = cmdTransformScale.map(lineListDown[c]);
//        lineListDown[c] = cmdTransformRotate.map(lineListDown[c]);
//        lineListDown[c] = cmdTransformTranslate.map(lineListDown[c]);
//    }
//    gen_width(lineListDown);
//    gen_height(lineListDown);
//    uncenter.translate(width/2, height/2);
//    for (int c = 0; c < lineListUp.length(); c++)
//    {
//        if (lineListUp[c].p1() == QPoint(0, 0))
//        {
//            QPoint newPt = lineListUp[c].p2();
//            newPt = uncenter.map(newPt);
//            lineListUp[c].setP2(newPt);
//            continue;
//        }
//        if (lineListUp[c].p2() == QPoint(0, 0))
//        {
//            QPoint newPt = lineListUp[c].p1();
//            newPt = uncenter.map(newPt);
//            lineListUp[c].setP1(newPt);
//            continue;
//        }
//        lineListUp[c] = uncenter.map(lineListUp[c]);
//    }
//    for (int c = 0; c < lineListDown.length(); c++)
//    {
//        lineListDown[c] = uncenter.map(lineListDown[c]);
//    }
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

//hpgl_cmd hpgl::cmdGet(int cmd_index)
//{
////    return(cmdList.at(cmd_index));
//}

int hpgl::cmdCount()
{
//    return(cmdList.length());
}

int hpgl::totalMM()
{
//    QPoint prev;
//    QPoint curr;
//    hpgl_cmd cmd;
//    int mm;
//    prev.setX(0);
//    prev.setY(0);
//    for (int idx = 0; idx < cmdList.length(); idx++)
//    {
//        cmd = cmdList[idx];
//        if (cmd.opcode != "SP")
//        {
//            for (int i = 0; i < cmd.coordList.length(); i++)
//            {
//                int x, y;
//                curr = cmd.coordList.at(i);
//                x = curr.x() - prev.x();
//                y = curr.y() - prev.y();
//                mm += sqrt(x*x + y*y);
//            }
//        }

//    }
//    mm = mm * 0.025; // convert graphics units to mm
//    return(mm);
}

int hpgl::printLen()
{
    return(print().length());
}





void hpgl::clear_cmds()
{
//    for (int i = 0; i < cmdList.count(); i++)
//    {
//        delete cmdList[i];
//    }
//    cmdList.clear();
}





















