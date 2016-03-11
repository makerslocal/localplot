#ifndef HPGLCMD_H
#define HPGLCMD_H

#include <QList>
#include <QLine>
#include <QTransform>
#include <QPoint>
#include <QDebug>
#include <QString>

#include <string>

namespace std {
class hpgl_cmd;
}

class hpgl_cmd
{
public:
    hpgl_cmd();
    hpgl_cmd(QString text);
    hpgl_cmd(QChar newOpcode[], QList<QPoint> newVerts);
    ~hpgl_cmd();
    void set_opcode(QString newOpcode);
    void set_opcode(QChar newOpcode[]);
    void set_verts(QList<QPoint> newVerts);
    QList<QPoint> get_verts();
    QString print();
    QString print(QTransform transform);
    QString opcode();
    int printLen();
    QList<QLine> line_list();

public slots:
    //

protected:
    //QChar opcode[2];
    QString _opcode;
    QList<QPoint> coordList;
    int pen;
};

#endif // HPGLCMD_H
