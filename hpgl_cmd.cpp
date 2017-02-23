/**
 * HPGL_CMD - source
 * Christopher Bero <bigbero@gmail.com>
 */
#include "hpgl_cmd.h"

/**
 * @brief hpgl_cmd::hpgl_cmd
 * Default constructor
 */
hpgl_cmd::hpgl_cmd()
{
    pen = 0;
    opcode = "";
    coordList.clear();
}

/**
 * @brief hpgl_cmd::~hpgl_cmd
 * Destructor
 */
hpgl_cmd::~hpgl_cmd()
{
    // Nothing to clean up
}

/**
 * @brief hpgl_cmd::set_opcode
 * @param _opcode - The opcode for this command
 * @return - Returns 0 for success
 */
int hpgl_cmd::set_opcode(QString _opcode)
{
    if (_opcode != "IN" && _opcode != "SP" && _opcode != "PD" && _opcode != "PU")
    {
        qDebug() << "ERROR: cannot set an unsupported opcode in hpgl_cmd!";
        opcode = "IN"; // IN should be harmless if the program continues executing.
        return 1;
    }
    opcode = _opcode;
    return 0;
}

/**
 * @brief hpgl_cmd::set_pen
 * @param _pen - The pen number to set
 * @return - Returns 0 for success
 */
int hpgl_cmd::set_pen(int _pen)
{
    if (_pen < 0)
    {
        qDebug() << "ERROR: cannot set a pen that is less than 0!";
        pen = 0; // Sane default
        return 1;
    }
    pen = _pen;
    return 0;
}

void hpgl_cmd::add_coord(QPoint _coord)
{
    coordList.push_back(_coord);
}

/**
 * @brief hpgl_cmd::get_opcode
 * @return - Returns the commands opcode
 */
QString hpgl_cmd::get_opcode()
{
    return(opcode);
}

/**
 * @brief hpgl_cmd::get_pen
 * @return - Returns the commands pen
 */
int hpgl_cmd::get_pen()
{
    return(pen);
}

/**
 * @brief hpgl_cmd::get_coordList
 * @return - Returns the coordnate list
 */
QList<QPoint> hpgl_cmd::get_coordList()
{
    return(coordList);
}

/**
 * @brief hpgl_cmd::lenHyp
 * @return - the length (in mm) of the hypotenuse of the command's line segments
 */
double hpgl_cmd::lenHyp()
{
    QMutexLocker locker(&cmdListMutex);

    QPoint prev;
    QPoint curr;
    hpgl_cmd cmd;
    double mm = 0;
    prev.setX(0);
    prev.setY(0);
    cmd = cmdList[cmd_index];
    if (cmd.opcode != "SP")
    {
        for (int i = 0; i < cmd.coordList.length(); i++)
        {
            int x, y;
            curr = cmd.coordList.at(i);
            x = abs(curr.x() - prev.x());
            y = abs(curr.y() - prev.y());
            mm += sqrt(x*x + y*y);
            prev.setX(curr.x());
            prev.setY(curr.y());
        }
    }
    mm = mm * 0.025; // convert graphics units to mm
    return(mm);
}

/**
 * @brief hpgl_cmd::lenX
 * @return - The "width" of a hpgl_cmd
 */
double hpgl_cmd::lenX()
{
    int prev = 0;
    int curr = 0;
    double mm = 0;

    if (opcode != "SP")
    {
        for (int i = 0; i < coordList.length(); i++)
        {
            curr = coordList.at(i).x();
            mm += abs(curr - prev);
            prev = curr;
        }
    }
    mm = mm * 0.025; // convert graphics units to mm
    return(mm);
}

/**
 * @brief hpgl_cmd::lenY
 * @return - The "height" of an hpgl_cmd
 */
double hpgl_cmd::lenY()
{
    int prev = 0;
    int curr = 0;
    double mm = 0;

    if (opcode != "SP")
    {
        for (int i = 0; i < coordList.length(); i++)
        {
            curr = coordList.at(i).y();
            mm += abs(curr - prev);
            prev = curr;
        }
    }
    mm = mm * 0.025; // convert graphics units to mm
    return(mm);
}

/**
 * @brief hpgl_cmd::print
 * @return - Builds and returns the HPGL style text for this hpgl_cmd
 */
QString hpgl_cmd::print()
{
    QString retval = "";

    retval += opcode;
    if (opcode == "SP")
    {
        retval += QString::number(pen);
    }
    else
    {
        for (int i = 0; i < coordList.length(); i++)
        {
            QPoint point = coordList.at(i);

            // I don't remember if these are useful
//            QTransform center, uncenter;
//            center.translate(-width/2, -height/2);
//            uncenter.translate(width/2, height/2);

            // transformations are disabled until further notice
//            point = cmdTransformScale.map(point);
//            point = cmdTransformRotate.map(point);
//            point = cmdTransformTranslate.map(point);

            if (point.x() < 0 || point.y() < 0)
            {
                retval = "OOB"; // Out of Bounds
                return retval;
            }

            retval += QString::number(point.x());
            retval += ",";
            retval += QString::number(point.y());
            if (i < (coordList.length()-1))
            {
                retval += ",";
            }
        }
    }
    retval += ";";
    return(retval);
}

// Returns the estimated time to execute command in seconds
double hpgl_cmd::time()
{
    double retval = 0;
    QSettings settings;

    retval = lenHyp();
//    retval = fmax(obj.cmdLenX(cmd_index), obj.cmdLenY(cmd_index));
//    retval = (obj.cmdLenX(cmd_index) + obj.cmdLenY(cmd_index));
    if (retval <= 0)
    {
        return(retval);
    }
    if (opcode == "PD")
    {
//        qDebug() << "Is null: " << settings.isNull();
        retval = retval / speedTranslate(settings.value("device/speed/cut", SETDEF_DEVICE_SPEED_CUT).toInt());
//        qDebug() << "- PD, speedTranslate: " << speedTranslate(CUTSPEED);
    }
    else if (opcode == "PU")
    {
        retval = retval / speedTranslate(settings.value("device/speed/travel", SETDEF_DEVICE_SPEED_TRAVEL).toInt());
//        qDebug() << "- PU, speedTranslate: " << speedTranslate(TRAVELSPEED);
    }

    return(retval);
}

double hpgl_cmd::speedTranslate(int setting_speed)
{
//    return((0.5*setting_speed) + 30);
    return((0.3*setting_speed) + 70);
//    return((0.52*setting_speed) + 24.8);
}





































