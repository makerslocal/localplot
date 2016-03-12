#ifndef HPGLOBJ_H
#define HPGLOBJ_H

#include <QTransform>

#include "hpgl_cmd.h"

namespace std {
class hpgl_obj;
}

class hpgl_obj
{
public:
    hpgl_obj();
    hpgl_obj(QString text);
    ~hpgl_obj();
    QString print();
//    int printLen();
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
