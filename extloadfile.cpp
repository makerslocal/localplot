#include "extloadfile.h"

ExtLoadFile::ExtLoadFile(hpglListModel *model)
{
    QSettings settings;

    hpglModel = model;
    filePath = settings.value("mainwindow/filePath", SETDEF_MAINWINDOW_FILEPATH).toString();
}

ExtLoadFile::~ExtLoadFile()
{
    //
}

void ExtLoadFile::process()
{
    QFile inputFile;
    QString buffer;
    file_uid newFile;
    QPersistentModelIndex index;

    if (filePath.isEmpty())
    {
        qDebug() << "File path is empty.";
        return;
    }

    if (filePath.endsWith(".svg", Qt::CaseInsensitive))
    {
        qDebug() << "File is SVG: " << filePath;
        QProcess * svgToHpgl = new QProcess(this);
        QString program = "python2.7";
        QStringList arguments;
        arguments << "/usr/share/inkscape/extensions/hpgl_output.py"
                  << "--precut=FALSE"
                  << "--force=0"
                  << "--speed=0"
                  << filePath;
        svgToHpgl->setProcessChannelMode(QProcess::MergedChannels);
        svgToHpgl->start(program, arguments);
        svgToHpgl->waitForStarted();
        svgToHpgl->waitForReadyRead();
        QByteArray data = svgToHpgl->readLine();
        buffer = QString::fromUtf8(data.data(), data.length());
        qDebug() << "Output: " << buffer;
        svgToHpgl->waitForFinished();
    }
    else if (filePath.endsWith(".hpgl", Qt::CaseInsensitive))
    {
        qDebug() << "File is HPGL: " << filePath;
        inputFile.setFileName(filePath);
        if (!inputFile.open(QIODevice::ReadOnly))
        {
            // failed to open
            qDebug() << "Failed to open file.";
            return;
        }
        QTextStream fstream(&inputFile);
        buffer = fstream.readAll();
        inputFile.close();
    }
    else
    {
        qDebug() << "File type not recognized D:";
        emit statusUpdate("File type not recognized.", Qt::darkRed);
        emit finished(QModelIndex());
        return;
    }

    if (buffer.isEmpty())
    {
        qDebug() << "Input text buffer is empty.";
        emit statusUpdate("Input buffer empty.", Qt::darkRed);
        emit finished(QModelIndex());
        return;
    }

    QString filename = filePath.split("/").last();
    newFile.filename = filename;
    newFile.path = filePath;

    index = createHpglFile(newFile);
    if (!index.isValid())
    {
        qDebug() << "Failed to create new model row.";
        inputFile.close();
        return;
    }

    parseHPGL(index, &buffer);
    emit statusUpdate("Finished parsing file.");
    emit finished(index);
}

QPersistentModelIndex ExtLoadFile::createHpglFile(file_uid _file)
{
    int numRows = hpglModel->rowCount();

    if (!hpglModel->insertRows(numRows, 1))
    {
        qDebug() << "Insert row failed.";
        return(QModelIndex());
    }

    QPersistentModelIndex index = QPersistentModelIndex(hpglModel->index(numRows));

    if (!hpglModel->setFileUid(index, _file))
    {
        qDebug() << "setFileUid failed.";
        return(QModelIndex());
    }

    return(index);
}

// Modifies input string, warning!
bool ExtLoadFile::parseHPGL(const QPersistentModelIndex index, QString * hpgl_text)
{
    QPointF tail(0, 0);

    hpgl_text->remove('\n');
    int numCmds = hpgl_text->count(';');
    for (int i = 0; i < numCmds; i++)
    {
        QPolygonF newItem;
        QString cmdText;
        QString opcode;
        int pen;

        cmdText = hpgl_text->section(';', i, i);

        qDebug() << "====\n= Processing command: ";

        // Get opcode, first two characters
        opcode = cmdText.mid(0, 2);

        qDebug() << "= " << opcode;

        // Parse opcode
        if (opcode == "PU")
        {
            // Pen up - we assume a single line (two points)
            int commaCount, newX, newY;
            cmdText.remove(0,2);
            commaCount = cmdText.count(',');
//            qDebug() << "= Comma count: " << commaCount;
            int i = commaCount - 1;
            newX = cmdText.section(',', i, i).toInt();
            i++;
            newY = cmdText.section(',', i, i).toInt();
            tail.setX(newX);
            tail.setY(newY);
        }
        else if (opcode == "PD")
        {
            // Pen down
            cmdText.remove(0,2);
            int commaCount = cmdText.count(',');
//            qDebug() << "= Comma count: " << commaCount;
            newItem << tail; // Begin from last PU location
            for (int i = 0; i < commaCount; i++)
            {
                //qDebug() << "processing coord: " << text.at(i) << endl;
                int newX = cmdText.section(',', i, i).toInt();
                i++;
                int newY = cmdText.section(',', i, i).toInt();
//                qDebug() << "= Found x: " << newX << " y: " << newY;
                newItem << QPointF(newX, newY);
//                if (i < (commaCount-2) && ((i+1) % 500) == 0)
//                {
//                    qDebug() << "Breaking line";
//                    cmdList.push_back(newCmd);
//                    newCmd.coordList.clear();
//                    newCmd.coordList.push_back(QPoint(newX, newY));
//                }
            }
            emit newPolygon(index, newItem);
        }
        else if (opcode == "IN")
        {
            // Begin plotting (automatically handled)
        }
        else if (opcode == "SP")
        {
            // Set pen
            pen = cmdText.mid(2,1).toInt();
            qDebug() << "[" << QString::number(pen) << "]";
        }
        else if (opcode == "FS" || opcode == "VS")
        {
            // Real hpgl that we don't care about
        }
        else
        {
            // Default case, something's wrong
            return false;
        }
        int progressInt = ((double)i/(numCmds-1))*100;
        emit progress(progressInt);
    }
    return true;
}

void ExtLoadFile::statusUpdate(QString _consoleStatus)
{
    emit statusUpdate(_consoleStatus, Qt::black);
}































