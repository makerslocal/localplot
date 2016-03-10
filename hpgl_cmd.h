#ifndef HPGLCMD_H
#define HPGLCMD_H

#include <QList>
#include <QLine>
#include <QPen>

#include <string>

#include "hpgl_coord.h"

namespace std {
class hpgl_cmd;
}

class hpgl_cmd
{
public:
    hpgl_cmd();
    hpgl_cmd(QString text);
    hpgl_cmd(QChar newOpcode[], QList<hpgl_coord> newVerts);
    ~hpgl_cmd();
    void set_opcode(QString newOpcode);
    void set_opcode(QChar newOpcode[]);
    void set_verts(QList<hpgl_coord> newVerts);
    QList<hpgl_coord> get_verts();
    QString print();
    QString opcode();
    int printLen();
    QList<QLine> line_list();

public slots:
    //

protected:
    //QChar opcode[2];
    QString _opcode;
    QList<hpgl_coord> coordList;
    int pen;
};

#endif // HPGLCMD_H
