/**
 * ETC - Source
 * Christopher Bero <bigbero@gmail.com>
 */
#include "etc.h"


QString timeStamp()
{
    return(QTime::currentTime().toString("[HH:mm ss.zzz]\n- "));
}

/**
 * @brief get_nextInt
 * Returns integer from section of string.
 *
 * @param input
 * @param index
 * @return
 */
int get_nextInt(QString input, int * index)
{
    QChar tmp = input[*index];
    QString buffer = "";

    while (tmp != ',' && tmp != ';')
    {
        buffer.append(tmp);
        tmp = input[++*index];
    }
    return(atoi(buffer.toStdString().c_str()));
}

double speedTranslate(int setting_speed)
{
//    return((0.5*setting_speed) + 30);
    return((0.3*setting_speed) + 70);
//    return((0.52*setting_speed) + 24.8);
}

bool operator==(const file_uid& lhs, const file_uid& rhs)
{
    return(lhs.path == rhs.path && lhs.uid == rhs.uid);
}
