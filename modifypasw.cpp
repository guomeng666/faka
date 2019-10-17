#include "modifypasw.h"
#include "ui_modifypasw.h"
#include <QMessageBox>
#include <QDebug>

ModifyPASW::ModifyPASW(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModifyPASW)
{
    ui->setupUi(this);
}

void ModifyPASW::setWebApiAddress(QString addr)
{
    mWebApi.setWebServiceAddress(addr);
}

void ModifyPASW::setUserInfo(int clientID, int orgID, int userID, int roleID)
{
    //用户登录的4个ID
     mClientID = clientID; //公司ID
     mOrgID = orgID;    //部门ID
     mUserID = userID;   //用户ID
     mRoleID = roleID;   //用户权限
}

ModifyPASW::~ModifyPASW()
{
    delete ui;
}

void ModifyPASW::on_pushButton_clicked()
{
    if(ui->editUser->text().isEmpty())
    {
        QMessageBox::warning(NULL,"输入提示","用户名不能为空");
        return;
    }
    if(ui->editOldPWSD->text().isEmpty())
    {
        QMessageBox::warning(NULL,"输入提示","旧密码不能为空");
        return;
    }
    if(ui->editNewPWSD->text().isEmpty() || ui->editPWSDAgain->text().isEmpty())
    {
        QMessageBox::warning(NULL,"输入提示","新密码不能为空");
        return;
    }
    if(ui->editNewPWSD->text() != ui->editPWSDAgain->text())
    {
        QMessageBox::warning(NULL,"输入提示","2次输入的新密码不一样");
        return;
    }
    QStringList list;
    list << ui->editUser->text()+","+QString::number(mClientID)+","+QString::number(mOrgID)+","+QString::number(mUserID)+","+QString::number(mRoleID)\
         << ui->editOldPWSD->text() << ui->editNewPWSD->text();
    qDebug()<<list;
    QString res = mWebApi.updatePassword(list);
    if(res.contains("成功"))
    {
        ui->editNewPWSD->clear();
        ui->editOldPWSD->clear();
        ui->editPWSDAgain->clear();
        ui->editUser->clear();
        this->accept();
    }
    else
    {
        QMessageBox::warning(NULL,"更改提示","密码修改失败:"+res);
    }
}

void ModifyPASW::on_pushButton_2_clicked()
{
    this->reject();
}
