#ifndef HPGLOBJ_H
#define HPGLOBJ_H

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
    int printLen();
    QList<QLine> line_list_up();
    QList<QLine> line_list_down();

public slots:
    //

protected:
    QList<hpgl_cmd> cmdList;
};

#endif // HPGLOBJ_H
