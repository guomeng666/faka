#include "logindialog.h"
#include "ui_logindialog.h"
#include "soapStub.h"
#include  <QMouseEvent>
#include <QMessageBox>
#include <QDebug>

loginDialog::loginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::loginDialog)
{
    ui->setupUi(this);

    ui->editPSWD->setEchoMode(QLineEdit::Password); //密码输入编辑框设置为密码输入模式
    //this->setAttribute(Qt::WA_DeleteOnClose);//设置为关闭时删除
    //this->setWindowFlags(Qt::SplashScreen); //设置为SplashScreen, 窗口无边框,不在任务栏显示
   this->setWindowFlags(Qt::FramelessWindowHint);//无边框，但是在任务显示对话框标题
}

loginDialog::~loginDialog()
{
    delete ui;
}

void loginDialog::setWebApiAddress(QString addr)
{
    mWebApi.setWebServiceAddress(addr);
}

void loginDialog::mousePressEvent(QMouseEvent *event)
{ //鼠标按键被按下
    if (event->button() == Qt::LeftButton)
    {
        m_moving = true;
        //记录下鼠标相对于窗口的位置
        //event->globalPos()鼠标按下时，鼠标相对于整个屏幕位置
        //pos() this->pos()鼠标按下时，窗口相对于整个屏幕位置
        m_lastPos = event->globalPos() - pos();
    }
    return QDialog::mousePressEvent(event);  //
}

void loginDialog::mouseMoveEvent(QMouseEvent *event)
{//鼠标按下左键移动
    //(event->buttons() && Qt::LeftButton)按下是左键
    //鼠标移动事件需要移动窗口，窗口移动到哪里呢？就是要获取鼠标移动中，窗口在整个屏幕的坐标，然后move到这个坐标，怎么获取坐标？
    //通过事件event->globalPos()知道鼠标坐标，鼠标坐标减去鼠标相对于窗口位置，就是窗口在整个屏幕的坐标
    if (m_moving && (event->buttons() && Qt::LeftButton)
        && (event->globalPos()-m_lastPos).manhattanLength() > QApplication::startDragDistance())
    {
        move(event->globalPos()-m_lastPos);
        m_lastPos = event->globalPos() - pos();
    }
    return QDialog::mouseMoveEvent(event);
}

void loginDialog::mouseReleaseEvent(QMouseEvent *event)
{//鼠标按键释放
    m_moving=false; //停止移动
}

void loginDialog::on_btEnter_clicked()
{
    web__user resParameter;

    QStringList list;
    list<<ui->editName->text() << ui->editPSWD->text();
    QString resStr = mWebApi.webLogin(list,resParameter);
    if(resStr.contains("成功"))
    {
        mClientID = resParameter.AD_USCOREClient_USCOREID;
        mOrgID =    resParameter.AD_USCOREOrg_USCOREID;
        mUserID =   resParameter.AD_USCOREUSER_USCOREID;
        mRoleID =   resParameter.AD_USCOREROLE_USCOREID;
        mName   = QString::fromUtf8((const char*)(resParameter.NAME->data()));
        if(mRoleID == 1000008 || mRoleID == 1000020 || mRoleID == 1000012)
        {
            qDebug()<<mName+"登录成功";
            this->accept();
        }
        else
        {
            qDebug()<<mName+"没有权限,登录失败";
            QMessageBox::warning(NULL,"登录失败","没有权限登录此软件","确定");
        }
    }else
    {
         qDebug()<<"登录失败"+resStr;
        QMessageBox::warning(NULL,"登录失败",resStr,"确定");
    }
}

void loginDialog::on_btCancel_clicked()
{
    this->reject();
}
