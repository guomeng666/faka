#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "webserviceapi.h"

namespace Ui {
class loginDialog;
}

class loginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit loginDialog(QWidget *parent = 0);
    ~loginDialog();

    void setWebApiAddress(QString addr);
private slots:
    void on_btEnter_clicked();
    void on_btCancel_clicked();

private:
    //用于鼠标拖动窗口的鼠标事件操作
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    Ui::loginDialog *ui;
    bool    m_moving = false;//表示窗口是否在鼠标操作下移动
    QPoint  m_lastPos;  //上一次的鼠标位置
    WebServiceAPI mWebApi;

public:
    int mClientID; //公司ID
    int mOrgID;    //部门ID
    int mUserID;   //用户ID
    int mRoleID;   //用户权限
    QString mName;
};

#endif // LOGINDIALOG_H
