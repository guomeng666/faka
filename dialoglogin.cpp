#include "dialoglogin.h"
#include "ui_dialoglogin.h"

DialogLogin::DialogLogin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogLogin)
{
    ui->setupUi(this);
}

void DialogLogin::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    exit(0);
}

DialogLogin::~DialogLogin()
{
    delete ui;
}

void DialogLogin::setHardWareStateText(HardWareType type, bool state)
{
    switch(type)
    {
        case LED:
        if(state == true)
        {
            ui->labLed->setStyleSheet("color:green;");
            ui->labLed->setText("正常");
        }else
        {
            ui->labLed->setStyleSheet("color:red;");
            ui->labLed->setText("异常");
        }
        break;
        case CAMERA:
        if(state == true)
        {
            ui->labCamera->setStyleSheet("color:green;");
            ui->labCamera->setText("正常");
        }else
        {
            ui->labCamera->setStyleSheet("color:red;");
            ui->labCamera->setText("异常");
        }
        break;
        case USERCARD:
        if(state == true)
        {
            ui->labUserCard->setStyleSheet("color:green;");
            ui->labUserCard->setText("正常");
        }else
        {
            ui->labUserCard->setStyleSheet("color:red;");
            ui->labUserCard->setText("异常");
        }
        break;
        case READER1356:
        if(state == true)
        {
            ui->lab1356->setStyleSheet("color:green;");
            ui->lab1356->setText("正常");
        }else
        {
            ui->lab1356->setStyleSheet("color:red;");
            ui->lab1356->setText("异常");
        }
        break;
        case READER900DESK:
        if(state == true)
        {
            ui->lab900Desk->setStyleSheet("color:green;");
            ui->lab900Desk->setText("正常");
        }else
        {
            ui->lab900Desk->setStyleSheet("color:red;");
            ui->lab900Desk->setText("异常");
        }
        break;
        case READER900OUT:
        if(state == true)
        {
            ui->lab900Out->setStyleSheet("color:green;");
            ui->lab900Out->setText("正常");
        }else
        {
            ui->lab900Out->setStyleSheet("color:red;");
            ui->lab900Out->setText("异常");
        }
        break;
        case DATABASE:
        if(state == true)
        {
            ui->labDB->setStyleSheet("color:green;");
            ui->labDB->setText("正常");
        }else
        {
            ui->labDB->setStyleSheet("color:red;");
            ui->labDB->setText("异常");
        }
        break;
    default:
        break;
    }
}
