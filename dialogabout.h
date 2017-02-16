/**
 * DialogAbout - About window header
 * Christopher Bero <bigbero@gmail.com>
 */
#ifndef DIALOGABOUT_H
#define DIALOGABOUT_H

#include <QDialog>
#include <QDebug>

namespace Ui {
class DialogAbout;
}

class DialogAbout : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAbout(QWidget *parent = 0);
    ~DialogAbout();

signals:
    void please_close();

private:
    Ui::DialogAbout *ui;
};

#endif // DIALOGABOUT_H
