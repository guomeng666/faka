#ifndef DIALOGLOGIN_H
#define DIALOGLOGIN_H

#include <QDialog>

namespace Ui {
class DialogLogin;
}

class DialogLogin : public QDialog
{
    Q_OBJECT
public:
    enum HardWareType
    {
        LED = 0,
        CAMERA,
        USERCARD,
        READER1356,
        READER900DESK,
        READER900OUT,
        DATABASE
    };

public:
    explicit DialogLogin(QWidget *parent = 0);

     ~DialogLogin();

private:
    void closeEvent(QCloseEvent *event);

public:
    void setHardWareStateText(HardWareType type, bool state);

private:
    Ui::DialogLogin *ui;
};

#endif // DIALOGLOGIN_H
