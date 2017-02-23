/**
 * HPGL_FILE - source
 * Christopher Bero <bigbero@gmail.com>
 */
#include "hpgl_file.h"

hpgl_file::hpgl_file(QString _filename)
{
    // load file
}

hpgl_file::~hpgl_file()
{
    // Clean up
}





QString hpgl_file::print(int command_index)
{
    return(cmdList[command_index]->print());
}

int hpgl_file::cmdCount()
{
    return(cmdList.count());
}































