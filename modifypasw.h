#ifndef MODIFYPASW_H
#define MODIFYPASW_H

#include <QDialog>
#include "webserviceapi.h"

namespace Ui {
class ModifyPASW;
}

class ModifyPASW : public QDialog
{
    Q_OBJECT

public:
    explicit ModifyPASW(QWidget *parent = 0);
    void setWebApiAddress(QString addr);
    void setUserInfo(int clientID, int orgID, int userID, int roleID);
    ~ModifyPASW();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::ModifyPASW *ui;
    WebServiceAPI mWebApi;

    QString mUserName;
    QString mOldPSWD;
    QString mNewPSWD;

    //用户登录的4个ID
    int mClientID; //公司ID
    int mOrgID;    //部门ID
    int mUserID;   //用户ID
    int mRoleID;   //用户权限

};

#endif // MODIFYPASW_H
