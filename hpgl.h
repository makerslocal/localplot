/**
 * HPGL_OBJ - header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef HPGLOBJ_H
#define HPGLOBJ_H

#include <QtCore>

#include "settings.h"

namespace std {
class hpgl;
}

class hpgl
{

public:
    hpgl();
    hpgl(QString text);
    ~hpgl();
    QString print();
    int printLen();
    int totalMM();

    int cmdCount();
    void gen_line_lists();
    void gen_height(QList<QLine> lineList);
    void gen_width(QList<QLine> lineList);
    double time(int command_index);


//    QTransform cmdTransformScale;
//    QTransform cmdTransformRotate;
//    QTransform cmdTransformTranslate;
    QList<QLine> lineListUp;
    QList<QLine> lineListDown;

private:
    void clear_cmds();
    int load_file(QString _filepath);
    void parseHPGL(QString hpgl_text);

    QSettings settings;
    QMutex cmdListMutex;

    int width;
    int height;
};

#endif // HPGLOBJ_H
















