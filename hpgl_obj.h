#ifndef HPGLOBJ_H
#define HPGLOBJ_H

#include <math.h>

#include <QTransform>
#include <QList>
#include <QLine>
#include <QTransform>
#include <QPoint>
#include <QDebug>
#include <QString>

struct hpgl_cmd {
    QString opcode;
    QList<QPoint> coordList;
    int pen;
};

namespace std {
class hpgl_obj;
}

class hpgl_obj
{
public:
    hpgl_obj();
    hpgl_obj(QString text);
    ~hpgl_obj();
    void parseHPGL(QString hpgl_text);
    QString print();
    int printLen();
    int totalMM();
    double cmdLenHyp(int cmd_index);
    double cmdLenX(int cmd_index);
    double cmdLenY(int cmd_index);
    hpgl_cmd cmdGet(int cmd_index);
    QString cmdPrint(int cmd_index);
    int cmdCount();
    hpgl_cmd initCmd();
    void gen_line_lists();
    void gen_height(QList<QLine> lineList);
    void gen_width(QList<QLine> lineList);

    QTransform cmdTransformScale;
    QTransform cmdTransformRotate;
    QTransform cmdTransformTranslate;
    QList<QLine> lineListUp;
    QList<QLine> lineListDown;

public slots:
    //

protected:
    QList<hpgl_cmd> cmdList;
    int width;
    int height;
};

#endif // HPGLOBJ_H
















