/**
 * ETC - Source
 * Christopher Bero <bigbero@gmail.com>
 */
#include "etc.h"

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
