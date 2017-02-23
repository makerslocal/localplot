/**
 * HPGL_FILE - header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef HPGLFILE_H
#define HPGLFILE_H

#include <QtCore>

#include "hpgl_cmd.h"

namespace std {
class hpgl_file;
}

class hpgl_file
{
public:
    hpgl_file(QString _filename);
    ~hpgl_file();
    void set_perimeter(bool _perimeter);
    bool get_perimeter();
    hpgl_cmd cmdGet(int cmd_index);
    int cmdCount();
    double speedTranslate(int setting_speed);
    double time(int command_index);
    QString print(int command_index);

private:
    void clear_cmds();
    int load_file(QString _filepath);
    void parseHPGL(QString hpgl_text);

    bool perimeter;
    int width;
    int height;
    QTransform fileTransformScale;
    QTransform fileTransformRotate;
    QTransform fileTransformTranslate;
    QString filename;
    QList<hpgl_cmd*> cmdList;
};

#endif // HPGLFILE_H
