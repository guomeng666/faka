#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QSettings>
#include <QSqlDatabase>
#include <QUdpSocket>
#include <QtTextToSpeech/QTextToSpeech>
#include "dialoglogin.h"
#include "ledcontrol.h"
#include "uhfdesktopreader.h"
#include "uhfoutreader.h"
#include "userread.h"
#include "videoplayer.h"
#include "vehiclecamera.h"
#include "vzclientsdk_commondefine.h"
#include "vzclientsdk_lpdefine.h"
#include "reader1356.h"
#include "webserviceapi.h"
#include "logindialog.h"
#include "modifypasw.h"

namespace Ui {
class MainWindow;
}
class PingThread;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    QString getWebServiceIp() { return serviceIP; }
    void setLoginDialog(loginDialog* dialog);
    ~MainWindow();

private:
    void initDeviceParameter();
    void initHardWare();
    void initLed();
    void initCamera();
    void init900OutReader();
    void init900DeskReader();
    void init1356DeskReader();
    void initUserReader();
    void initDB();
    void initFrom();
    void initTimer();

    void startHardWare();
    void startPingThread();

    void clearInterFace(int index);
    void queryCarFromDB(QString license);
    QString generateRandomString();

    QString findCarType(int index);
    int findCarType(QString type);
    QString findPackType(int index);

    bool checkLineEdit1();
    bool checkLineEdit2();

    bool insertCarInfo();
    bool insertUserInfo();
    QString savePersonalInfo();
    QString saveContractInfo();

    void setLoginInfo();

private slots:
    void on_cboAcquisitionType_currentIndexChanged(const QString &arg1);
    void sendCameraHeart();
    void onCameraConnected();
    void onCameraDiscon();
    void onCameraError(QAbstractSocket::SocketError);
    void onCameraReadyCarData(QByteArray data);

    void onUHFOutConnected();
    void onUHFoutDiscon();
    void onUHFOutError(QAbstractSocket::SocketError error);
    void onRecvOutTid(TagInfo);
    void onReadCheck();

    //身份证信息槽函数
    void onReadUserID(stu_card_info info);
    void onRead1356UID();
    void on_btnActive_clicked();

    /*定时器槽函数*/
    void onLedTimerOut();
    void onActiveTimerOut();
    void onWrite1356();
    void onSerialTimeout();
    void onTimerCounter();

    void on_btnSave_clicked();
    void closeEvent(QCloseEvent *event);
    void on_btnSendCard_clicked();

    void onPingGoodStatus(QString str,int n);
    void onPingBadStatus(QString str,int n);
    void onPingDisconStatus(QString str,int n);

    void on_editContractTransport_returnPressed();

    void on_editPersonalUserID_returnPressed();

    void on_editCarID_returnPressed();

    void on_actLogin_triggered();

    void on_actUpdatePasswd_triggered();

    void on_actHDInfo_triggered();

    void on_actAbout_triggered();

private:
    Ui::MainWindow *ui;
    loginDialog *mLoginUserFrom;
    ModifyPASW *mModifyPASW;
    DialogLogin m_formLogin;
    QLabel * labLogin;
    QLabel* labTimer;
    QSettings *ini;

    /*硬件设备*/
    LedControl m_ledControl;
    UHFDeskTopReader m_UHFDeskReader;
    UhfOutReader m_UHFOutReader;
    ReadCard m_userIdReader;
    VehicleCamera m_vehicleCamera;
    Reader1356 m_reader1356;
    QTextToSpeech  tts;

    PingThread *pingThread;

    QByteArray m_tempSerial;

    //数据库
    QSqlDatabase  DB; //数据库
    WebServiceAPI webAPI;

    bool mDBhaveCarInfo;  //数据库中是否有当前车辆信息
    int mCurCarDBID;      //当前业务要处理的车信息
    QString mCurSmallJpg;

    bool mDBhaveUserInfo;//数据库中是否有当前的身份证号
    int mCurC_GRAINUSER_ID;//当前业务要处理的身份证信息

    //初始化标记
    bool initCameraOk;
    bool initUserReaderOK;
    bool init1356ReaderOK;

    //定时器
    QTimer timerCamera;
    QTimer timerLed;
    QTimer timerActive;
    QTimer timerRead1356;
    QTimer timerSerial;
    QTimer timerReadCheck;
    QTimer timerCounter;

    //当前发卡的标签号码
    QByteArray m_curTagID;
    int m_readCnt;

    //用户登录的4个ID
    int mClientID; //公司ID
    int mOrgID;    //部门ID
    int mUserID;   //用户ID
    int mRoleID;   //用户权限

    //合同粮字段
    int c_targetvehicle_id;//车船号ID
    int c_contract_id;     //合同号ID
    int c_vendor_id;       //供应商ID
    int c_location_id;     //发站地点ID

    QPixmap bigJpg;
    QPixmap smallJpg;

    QString serviceIP;
    QUdpSocket mUdpSocket;
    //配置文件信息
    QString DBIp;
    QString resIp; //视频地址
    QString cameraIP; //摄像头IP
    qint32 cameraPort; //摄像头端口
    QString UhfIp;
    int UhfPort;
    QString deskReaderCom;
    QString desk1356Com;

    QString ledIp;
    int ledPort;
    bool ledIsConnected;
};

#endif // MAINWINDOW_H
