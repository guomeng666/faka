#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pingthread.h"
#include "qdebug.h"
#include <QSettings>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDateTime>
#include <QStringList>
#include <QSqlError>
#include <QTime>
#include <QVariant>

MainWindow::MainWindow(QWidget *parent) :\
    QMainWindow(parent),\
    ui(new Ui::MainWindow),initCameraOk(false),initUserReaderOK(false),\
    init1356ReaderOK(false),m_readCnt(0),mDBhaveCarInfo(false),mDBhaveUserInfo(false),ledIsConnected(false)
{
    ui->setupUi(this);
    this->setWindowTitle("发卡登记软件");
    ini = new QSettings("config.ini", QSettings::IniFormat);

    initDeviceParameter();
    initHardWare();//初始化硬件
    startPingThread();
    m_formLogin.setWindowTitle("硬件信息");
    m_formLogin.setWindowFlags(Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    if(m_formLogin.exec() == 0)
        exit (0);

    initFrom();
    initTimer();
    startHardWare();
    tts.say("软件启动成功");
}

void MainWindow::startPingThread()
{
    pingThread = new PingThread;
    connect(pingThread,SIGNAL(networkGood(QString,int)),this,SLOT(onPingGoodStatus(QString,int)));
    connect(pingThread,SIGNAL(networkBad(QString,int)),this,SLOT(onPingBadStatus(QString,int)));
    connect(pingThread,SIGNAL(networkDisconnect(QString,int)),this,SLOT(onPingDisconStatus(QString,int)));
    QStringList list;
    list << ledIp << cameraIP << UhfIp << DBIp;
    //list<<"192.168.188.130";
    pingThread->setPingList(list);
    pingThread->start();
}

void MainWindow::onPingGoodStatus(QString str,int n)
{
   // qDebug()<<str<<"网络状态极好:"<<n;
    Q_UNUSED(n)
    if(str == cameraIP)
    {
        if(m_vehicleCamera.isOpen())
        {
            return;
        }
        qDebug()<<"网络恢复重新连接摄像头";
        m_vehicleCamera.connectVehicleCamera(cameraIP,cameraPort);
    }else if(str == UhfIp)
    {
        if(m_UHFOutReader.isOpen())
        {
            return;
        }
        qDebug()<<"网络恢复重新连接900M";
        m_UHFOutReader.connectReader(UhfIp,UhfPort);
    }else if(str == DBIp)
    {
        if(DB.isOpen())
            return;
        initDB();
    }else if(str == ledIp)
    {
        if(ledIsConnected == true)
            return;
        int initCnt = 0;
        while(m_ledControl.connectLedControl(ledIp.toLocal8Bit().data(),ledPort))
        {
             m_formLogin.setHardWareStateText(DialogLogin::HardWareType::LED,false);
             if(initCnt++ == 3)
                 return;
        }
        qDebug()<<"LED重新连接成功";
        ledIsConnected = true;
        m_ledControl.setText("空闲中",GREEN,14);
        m_formLogin.setHardWareStateText(DialogLogin::HardWareType::LED,true);
    }
}

void MainWindow::onPingBadStatus(QString str,int n)
{
    qDebug()<<str<<"网络状态较差:"<<n;
}

void MainWindow::onPingDisconStatus(QString str,int n)
{
    qDebug()<<str;
    if(str == cameraIP)
    {
        qDebug()<<"摄像头网络断开,Ping率:"<<n;
        while(m_vehicleCamera.isOpen())
        {
            m_formLogin.setHardWareStateText(DialogLogin::HardWareType::CAMERA,false);
            timerCamera.stop();
            m_vehicleCamera.close();
        }
    }else if(str == UhfIp)
    {
        qDebug()<<"室外900M天线网络断开,Ping率:"<<n;
        while(m_UHFOutReader.isOpen())
        {
            m_formLogin.setHardWareStateText(DialogLogin::HardWareType::READER900OUT,false);
            m_UHFOutReader.close();
        }
    }else if(str == DBIp)
    {
        qDebug()<<"数据库网络断开,Ping率:"<<n;
        if(DB.isOpen())
            DB.close();
        m_formLogin.setHardWareStateText(DialogLogin::HardWareType::DATABASE,false);
    }else if(str == ledIp)
    {
        qDebug()<<"LED网络断开,Ping率:"<<n;
        if(ledIsConnected == true)
        {
            m_ledControl.disconnectLedControl();
            ledIsConnected = false;
            m_formLogin.setHardWareStateText(DialogLogin::HardWareType::LED,false);
        }
    }
}

void MainWindow::initTimer()
{
    connect(&timerLed, SIGNAL(timeout()),this,SLOT(onLedTimerOut()));
    connect(&timerActive,SIGNAL(timeout()),this,SLOT(onActiveTimerOut()));
    connect(&timerRead1356,SIGNAL(timeout()),this,SLOT(onWrite1356()));
    connect(&timerSerial,SIGNAL(timeout()),this,SLOT(onSerialTimeout()));
    connect(&timerReadCheck,SIGNAL(timeout()),this,SLOT(onReadCheck()));
    connect(&timerCounter,SIGNAL(timeout()),this,SLOT(onTimerCounter()));
}

void MainWindow::startHardWare()
{
    if(initCameraOk == true)
    {
        ui->labCaptrueID->setScaledContents(true);
        ui->labCaptureCar->setScaledContents(true);

        //以下方法可以设置背景图片或者背景文字
        ui->ffmpegWidget->setBgText("视频监控");

        ui->ffmpegWidget->setUrl(resIp);

        ui->ffmpegWidget->open();
    }
    if(initUserReaderOK == true)
    {
        connect(&m_userIdReader,SIGNAL(read_finished(stu_card_info)),this,SLOT(onReadUserID(stu_card_info)));
    }
    if(init1356ReaderOK == true)
    {
        connect(&m_reader1356,SIGNAL(readyRead()),this,SLOT(onRead1356UID()));
        timerRead1356.start(1000);
    }
    timerCounter.start(1000);
}

void MainWindow::initFrom()
{
    labLogin = new QLabel;
    labLogin->setMinimumWidth(200);
    QFont font = labLogin->font();
    font.setPointSize(14);
    labLogin->setFont(font);
    ui->statusBar->addWidget(labLogin);

    labTimer = new QLabel;
    labTimer->setMinimumWidth(200);
    labTimer->setAlignment(Qt::AlignRight);
    font = labTimer->font();
    font.setPointSize(14);
    labTimer->setFont(font);
    ui->statusBar->addWidget(labTimer,1);

    ui->statusBar->setMinimumHeight(40);

    ui->lcdnumber->setDecMode();
    ui->btnSave->setEnabled(false);

    ui->editTagID->setFocusPolicy(Qt::NoFocus);
    ui->editTagPower->setFocusPolicy(Qt::NoFocus);
    ui->editSettlementCardID->setFocusPolicy(Qt::NoFocus);
    ui->editTagStatus->setFocusPolicy(Qt::NoFocus);

    QRegExp regExp{"[a-zA-Z0-9]+$"};
    ui->editPersonalUserID->setValidator(new QRegExpValidator(regExp, this));
}

MainWindow::~MainWindow()
{
    qDebug()<<"析构事件";
    m_userIdReader.disconnect();
    m_userIdReader.quit();
    m_userIdReader.wait();
    m_userIdReader.deleteLater();
    delete ui;
}

void MainWindow::initDeviceParameter()
{
    //读取LED配置参数
    ini->beginGroup("LED");
    ledIp = ini->value("IP").toString();
    ledPort = ini->value("PORT").toInt();
    qDebug()<<"LED"<<ledIp<<ledPort;
    ini->endGroup();

    //读取摄像头配置参数
    ini->beginGroup("CAMERACAPTURE");
    cameraIP = ini->value("IP").toString();
    cameraPort = ini->value("PORT").toInt();
    qDebug()<<"CAMERACAPTURE"<<cameraIP<<cameraPort;
    ini->endGroup();
    ini->beginGroup("CAMERAVIDEO");
    resIp = ini->value("IP").toString();
    qDebug()<<"CAMERAVIDEO"<<resIp;
    ini->endGroup();

    //读取外部900M天线配置参数
    ini->beginGroup("UHFOUT");
    UhfIp = ini->value("IP").toString();
    UhfPort = ini->value("PORT").toInt();
    qDebug()<<"UHFOUT"<<UhfIp<<UhfPort;
    ini->endGroup();

    //读取桌面900M读写器配置参数
    ini->beginGroup("UHFDESKTOP");
    deskReaderCom = ini->value("COM").toString();
    qDebug()<<"UHFDESKTOP"<<deskReaderCom;
    ini->endGroup();

    //读取桌面13.56读写器配置参数
    ini->beginGroup("1356");
    desk1356Com = ini->value("COM").toString();
    qDebug()<<"1356"<<desk1356Com;
    ini->endGroup();

    //读取数据库配置参数
    ini->beginGroup("DB");
    DBIp = ini->value("IP").toString();
    qDebug()<<"DB:"<<DBIp;
    ini->endGroup();

    ini->beginGroup("WEBSERVICE");
    serviceIP = ini->value("IP").toString();
    qDebug()<<"WEBSERVICE:"<<serviceIP;
    ini->endGroup();
}

//初始化外部设备
void MainWindow::initHardWare()
{
    initCamera();
    init900OutReader();
    init900DeskReader();
    init1356DeskReader();
    initUserReader();
    initDB();
}

void MainWindow::initLed()
{
    int initCnt = 0;
    while(m_ledControl.connectLedControl(ledIp.toLocal8Bit().data(),ledPort))
    {
         m_formLogin.setHardWareStateText(DialogLogin::HardWareType::LED,false);
         if(initCnt++ == 3)
             return;
    }
    qDebug()<<"LED连接成功";
    ledIsConnected = true;
    m_ledControl.setText("空闲中",GREEN,14);
    m_formLogin.setHardWareStateText(DialogLogin::HardWareType::LED,true);
}

void MainWindow::initCamera()
{
    connect(&m_vehicleCamera, SIGNAL(connected()), this, SLOT(onCameraConnected()));
    connect(&m_vehicleCamera, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onCameraError(QAbstractSocket::SocketError)));
    connect(&m_vehicleCamera,SIGNAL(disconnected()),this,SLOT(onCameraDiscon()));
    connect(&timerCamera,SIGNAL(timeout()),this,SLOT(sendCameraHeart()));
    connect(&m_vehicleCamera, SIGNAL(readyCarData(QByteArray)), this, SLOT(onCameraReadyCarData(QByteArray)));
    m_vehicleCamera.connectVehicleCamera(cameraIP,cameraPort);
}

void MainWindow::init900OutReader()
{
    connect(&m_UHFOutReader, SIGNAL(connected()), this, SLOT(onUHFOutConnected()));
    connect(&m_UHFOutReader,SIGNAL(disconnected()),this,SLOT(onUHFoutDiscon()));
    connect(&m_UHFOutReader, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onUHFOutError(QAbstractSocket::SocketError)));
    connect(&m_UHFOutReader,SIGNAL(recvTagInfo(TagInfo)), this, SLOT(onRecvOutTid(TagInfo)));
    m_UHFOutReader.connectReader(UhfIp,UhfPort);
}

void MainWindow::init900DeskReader()
{
    bool res = m_UHFDeskReader.connectReader(deskReaderCom,QSerialPort::Baud57600);
    if(res == true){
        m_formLogin.setHardWareStateText(DialogLogin::HardWareType::READER900DESK,true);
        init1356ReaderOK = true;
    }
    else{
         m_formLogin.setHardWareStateText(DialogLogin::HardWareType::READER900DESK,false);
    }
}

void MainWindow::init1356DeskReader()
{
    bool res = m_reader1356.connectReader(desk1356Com,QSerialPort::Baud19200);
    if(res == true)
    {
        m_formLogin.setHardWareStateText(DialogLogin::HardWareType::READER1356,true);
    }
    else
    {
         m_formLogin.setHardWareStateText(DialogLogin::HardWareType::READER1356,false);
    }
}

void MainWindow::initUserReader()
{
    //初始化身份证读写器
    if(!m_userIdReader.init_cvr())
    {
        qDebug()<<"身份证读卡器初始化失败";
        m_formLogin.setHardWareStateText(DialogLogin::HardWareType::USERCARD,false);
    }else
    {
        qDebug()<<"身份证读卡器初始化成功";
        m_formLogin.setHardWareStateText(DialogLogin::HardWareType::USERCARD,true);
        initUserReaderOK = true;
    }
}

void MainWindow::initDB()
{
    DB = QSqlDatabase::addDatabase("QOCI");
    DB.setPort(1521);
    DB.setHostName(DBIp);
    DB.setDatabaseName("FYJT");
    DB.setUserName("FY");
    DB.setPassword("FYJT");
    if (!DB.open())
    {
        qDebug()<<"数据库初始化失败";
        m_formLogin.setHardWareStateText(DialogLogin::HardWareType::DATABASE,false);
        return;
    }
    qDebug()<<"数据库初始化成功";
    m_formLogin.setHardWareStateText(DialogLogin::HardWareType::DATABASE,true);

    //WEBSERVICE初始化
    webAPI.setWebServiceAddress(serviceIP);
}

//切换界面
void MainWindow::on_cboAcquisitionType_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "散收粮")
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->btnSendCard->setEnabled(true);
        ui->cboPackType->removeItem(3);
        ui->editContractID->clear();
        ui->editContractSource->clear();
        ui->editContractSupplier->clear();
        ui->editContractTransport->clear();
    }
    else if(arg1 == "合同粮")
    {
         ui->stackedWidget->setCurrentIndex(1);
         ui->btnSendCard->setEnabled(false);
         ui->cboPackType->addItem("集装箱");
         ui->editPersonalUserAddr->clear();
         ui->editPersonalUserID->clear();
         ui->editPersonalUserName->clear();
    }
}

//摄像头连接成功槽函数
void MainWindow::onCameraConnected()
{
    qDebug()<<"摄像头连接成功";
    m_formLogin.setHardWareStateText(DialogLogin::HardWareType::CAMERA,true);
    m_vehicleCamera.setCameraWorkMode(true,"bin",true,2);
    timerCamera.start(20000);
    initCameraOk = true;
}

//摄像头断开连接槽函数
void MainWindow::onCameraDiscon()
{
    qDebug()<<"摄像头断开连接";
    timerCamera.stop();
    m_vehicleCamera.abort();
    m_vehicleCamera.connectVehicleCamera(cameraIP,cameraPort);
}

//摄像头连接失败槽函数
void MainWindow::onCameraError(QAbstractSocket::SocketError error)
{
    qDebug()<<"重新连接摄像头";
    timerCamera.stop();
    m_formLogin.setHardWareStateText(DialogLogin::HardWareType::CAMERA,false);
    if(error == QAbstractSocket::RemoteHostClosedError)
    {

    }else if(error == QAbstractSocket::ConnectionRefusedError || error == QAbstractSocket::NetworkError)
    {
        m_vehicleCamera.abort();
        m_vehicleCamera.connectVehicleCamera(cameraIP,cameraPort);
    }
}

//摄像头抓拍信号
void MainWindow::onCameraReadyCarData(QByteArray data)
{
    if(data[0] == (char)'I' && data[1] == (char)'R' &&data[2] == (char)0x01)
    {
        qint32 carInfoLen = (0xff & data[7]) <<24 | (0xff&data[6]) << 16 | (0xff&data[5]) << 8 | (0xff&data[4]);
        QByteArray carData;
        TH_PlateResult *plateData;
        carData = data.mid(8,carInfoLen);
        plateData = (TH_PlateResult *)carData.data();
        qDebug()<<"捕获车牌号码:"<<QString::fromLocal8Bit((const char *)plateData->license)<<"车牌颜色:"<<QString::fromLocal8Bit((const char *)plateData->color);
        if(ui->editCarID->text().isEmpty())
            ui->editCarID->setText(QString::fromLocal8Bit((const char *)plateData->license));
        else
        {
            if(ui->editCarID->text() != QString::fromLocal8Bit((const char *)plateData->license))
            {
                clearInterFace(ui->stackedWidget->currentIndex());
            }
        }
        if(ledIsConnected == true)
            m_ledControl.setText(QString::fromLocal8Bit((const char *)plateData->license),GREEN,14);
        timerLed.start(60000);

        tts.say(QString::fromLocal8Bit((const char *)plateData->license));

        queryCarFromDB(QString::fromLocal8Bit((const char *)plateData->license)); //查询当前抓拍的车牌是否在数据库中存在

        data = data.mid(carInfoLen+8);
        if(data[0] == (char)'I' && data[1] == (char)'R' &&data[2] == (char)0x02) //大图片
        {
            qint32 bigJPGLen = (0xff & data[7]) <<24 | (0xff&data[6]) << 16 | (0xff&data[5]) << 8 | (0xff&data[4]);
            bigJpg.loadFromData(data.mid(8));
            bigJpg = bigJpg.scaled(ui->labCaptureCar->size());
            ui->labCaptureCar->setPixmap(bigJpg);
            data = data.mid(bigJPGLen+8);
            if(data[0] == (char)'I' && data[1] == (char)'R' &&data[2] == (char)0x03)//小图片
            {
                //mCurSmallJpg = data.mid(8).toHex(); //保存当前图片,一会可能要使用WEBAPI传走
                smallJpg.loadFromData(data.mid(8));
                smallJpg = smallJpg.scaled(ui->labCaptrueID->size());
                ui->labCaptrueID->setPixmap(smallJpg);
            }
        }
    }
}

//外部900天线连接成功信号
void MainWindow::onUHFOutConnected()
{
    qDebug()<<"室外900M连接成功";
    m_formLogin.setHardWareStateText(DialogLogin::HardWareType::READER900OUT,true);
}

void MainWindow::onUHFoutDiscon()
{
    qDebug()<<"室外900M连接断开";
    m_UHFOutReader.abort();
    m_UHFOutReader.connectReader(UhfIp,UhfPort);
}

//外部天线连接失败天线
void MainWindow::onUHFOutError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    qDebug()<<"重新连接室外天线";
    m_formLogin.setHardWareStateText(DialogLogin::HardWareType::READER900OUT,false);
    if(error == QAbstractSocket::RemoteHostClosedError)
    {

    }else if(error == QAbstractSocket::ConnectionRefusedError || error == QAbstractSocket::NetworkError)
    {
        m_UHFOutReader.abort();
        m_UHFOutReader.connectReader(UhfIp,UhfPort);
    }
}

void MainWindow::onRecvOutTid(TagInfo info)
{
    qDebug()<<"外部天线:"<<info.tagID.toHex();
    if(info.tagID == m_curTagID)
    {
        if(info.workStat == 2)//发卡成功
        {
            m_readCnt = 0;
            timerReadCheck.stop();
            ui->btnSave->setEnabled(true);
            ui->lcdnumber->display(0);
            if(ledIsConnected)
                m_ledControl.setText("发签成功",GREEN,14);
            tts.say("电子标签校验成功");
            ui->editTagID->setStyleSheet("background-color: rgb(ff, ff, ff)");
            timerLed.start(30000);
            QMessageBox::information(nullptr,"标签状态提示","标签号为"+info.tagID.toHex()+"的标签门前校验成功","确定");
        }
    }
}

//身份证信息槽函数
void MainWindow::onReadUserID(stu_card_info info)
{
    QSqlQuery query;
    QSqlRecord record;
    qDebug()<<info.ID<<info.name<<info.address;
    if(ui->stackedWidget->currentIndex() == 0)
    {
        ui->editPersonalUserName->setText(info.name);
        ui->editPersonalUserAddr->setText(info.address);
        ui->editPersonalUserID->setText(info.ID);
    }
    //查看此身份证信息是否在数据库中存在
    query.prepare("select C_GRAINUSER_ID,NAME,IDCARD,ADDRESS1 from C_GRAINUSER where IDCARD = :CARDID and ISACTIVE = 'Y'");
    query.bindValue(":CARDID",info.ID);
    query.exec();
    query.first();
    if(!query.isValid())
    {
        //身份证信息不存在保存时调用WEBAPI存储身份信息
        mDBhaveUserInfo = false;
        qDebug()<<"此身份证信息在数据库中不存在";
    }else
    {
        record = query.record();
        mDBhaveUserInfo = true;
        mCurC_GRAINUSER_ID = record.value(0).toInt();
        qDebug()<<"身份证信息存在,当前C_GRAINUSER_ID:"<<mCurC_GRAINUSER_ID;
    }
}

void MainWindow::on_btnActive_clicked()
{
    unsigned char write[2] = {0x97, 0xf4};
    bool res = m_UHFDeskReader.writeTID(write , 2);
    if(res == true)
    {
        qDebug()<<"电子标签状态写入成功";
        timerActive.start(1000);
        ui->btnActive->setEnabled(false);
    }
    else
        qDebug()<<"电子标签状态写入失败";
}

//发卡激活查询标志
void MainWindow::onActiveTimerOut()
{
    static int timeOutCnt = 0;
    TagInfo info;
    bool ok;
    info = m_UHFDeskReader.readTagStatus(ok);
    if(ok == true)
    {
        qDebug()<<"电子标签号码:"<<info.tagID.toHex();
        ui->editTagID->setText(info.tagID.toHex());
        qDebug()<<"电子标签通信状态:"<<info.communicationStat;
        qDebug()<<"电子标签报警状态:"<<info.warningStat;
        if(info.warningStat == true)
        {
            ui->editTagStatus->setStyleSheet("background-color: rgb(170, 0, 0)");
            ui->editTagStatus->setText("报警");
        }
        else
        {
            ui->editTagStatus->setStyleSheet("background-color: rgb(0, 170, 0)");
            ui->editTagStatus->setText("正常");
        }
        qDebug()<<"电子标签工作状态:"<<info.workStat;
        if(info.workStat != 3)
        {
            if(timeOutCnt++ == 8)
            {
                timerActive.stop();
                ui->btnActive->setEnabled(true);
                timeOutCnt = 0;
                QMessageBox::warning(nullptr,"标签状态提示","标签号为"+info.tagID.toHex()+"的标签激活失败","确定");
            }
        }
        else
        {
            timeOutCnt = 0;
            timerActive.stop();
            ui->editTagID->setStyleSheet("background-color: rgb(0, 170, 0)");
            ui->btnActive->setEnabled(true);
            m_curTagID = info.tagID;
            QMessageBox::information(nullptr,"标签状态提示","标签号为"+info.tagID.toHex()+"的标签激活成功","确定");
            ui->lcdnumber->display(120);
            timerReadCheck.start(200);//开始读取外部天线
            qDebug()<<"电子标签当前电量:"  << ((info.power / 1024.0) * 2 * 2.048)/3.6 *100;
            int power = static_cast<int>(((info.power / 1024.0) * 2 * 2.048)/3.6 *100);
            if(power > 56)
            {
                ui->editTagPower->setText("正常");
                ui->editTagPower->setStyleSheet("background-color: rgb(0, 170, 0)");
            }
            else
            {
                ui->editTagPower->setText("欠压");
                ui->editTagPower->setStyleSheet("background-color: rgb(170, 0, 0)");
            }
        }
    }/*else //读电子标签时,用户把电子标签拿走了
    {
          timerActive.stop();
          ui->btnActive->setEnabled(true);
    }*/
}

//读取外部900M天线,有定时器控制
void MainWindow::onReadCheck()
{
    if(m_readCnt++ == 600) //开始计时
    {
        m_readCnt = 0;
        timerReadCheck.stop(); //停止读写外部900MHZ天线
        clearInterFace(ui->stackedWidget->currentIndex());
        QMessageBox::warning(nullptr,"超时提示","电子标签超时","确定");
    }
    if(m_readCnt%5 == 0)
    {
        ui->lcdnumber->display(120 - m_readCnt/5);
        m_UHFOutReader.readTID();
    }
}

char badId[] = {0x46,0x61,0x69,0x6C,0x10,0xFF,0x04,0x46};

//串口定时缓冲区
void MainWindow::onSerialTimeout()
{
    timerSerial.stop();
    if(m_tempSerial.length() >= 11 && (strcmp(m_tempSerial.data(),badId) == 0))
    {
       ui->editSettlementCardID->setText(m_tempSerial.mid(3,8).toHex().toUpper());
    }
    m_tempSerial.clear();
}

//1秒读取一次13.56卡信息
void MainWindow::onRead1356UID()
{
    QByteArray uid;
    uid = m_reader1356.readAll();
    timerSerial.start(50);
    m_tempSerial.append(uid);
}

//LED屏幕超时槽函数
void MainWindow::onLedTimerOut()
{
    timerLed.stop();
    if(ledIsConnected == true)
        m_ledControl.setText("空闲中",GREEN,14);
}

//定时读取1356卡信息
void MainWindow::onWrite1356()
{
    m_reader1356.readUid();
}

//窗口退出
void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    qDebug()<<"关闭事件";
    ui->ffmpegWidget->close();
    m_ledControl.disconnectLedControl();
    m_UHFOutReader.abort();
    m_vehicleCamera.abort();
    m_reader1356.close();
    m_UHFOutReader.close();
    exit(0);
}

//摄像头心跳
void MainWindow::sendCameraHeart()
{
    m_vehicleCamera.sendHeart();
}

//清空界面
void MainWindow::clearInterFace(int index)
{
    ui->editTagID->clear();
    ui->editTagPower->clear();
    ui->editTagStatus->clear();
    ui->editCarID->clear();
    ui->editCarColor->clear();
    ui->editSettlementCardID->clear();
    ui->labCaptureCar->clear();
    ui->labCaptrueID->clear();
    ui->editTagID->setStyleSheet("background-color: rgb(ff,ff,ff)");
    ui->editTagPower->setStyleSheet("background-color: rgb(ff,ff,ff)");
    ui->editTagStatus->setStyleSheet("background-color: rgb(ff,ff,ff)");
    ui->cboCarType->setCurrentIndex(0);
    ui->cboPackType->setCurrentIndex(0);
    if(index == 0)
    {
        ui->editPersonalUserName->clear();
        ui->editPersonalUserID->clear();
        ui->editPersonalUserAddr->clear();
        ui->editSettlementCardID->clear();
    }else if(index == 1)
    {
        ui->editContractID->clear();
        ui->editContractSource->clear();
        ui->editContractSupplier->clear();
        ui->editContractTransport->clear();
    }
    mDBhaveCarInfo = false;
    mDBhaveUserInfo =false;
    mCurCarDBID = 0;
    mCurC_GRAINUSER_ID = 0;
    m_curTagID.clear();
}

QString MainWindow::generateRandomString()
{
    int max = 5;
    QString tmp = QString("0123456789abcdefghigklmnopqrstuvwxyz");
    QString str[5];
    QTime t;
    t= QTime::currentTime();
    qsrand(t.msec()+t.second()*1000);
    for(int j = 0; j < 5; j++)
    {
        for(int i = 0; i < max; i++)
        {
            int ir = qrand()%tmp.length();
            str[j][i] = tmp.at(ir);
        }
    }
    return str[0]+"-"+str[1]+"-"+str[2]+"-"+str[3]+"-"+str[4];
}

QString MainWindow::findCarType(int index)
{
    QString type;
    switch(index)
    {
        case 1:
            type.append('S');
        break;
        case 2:
            type.append('H');
        break;
        case 3:
            type.append('A');
        break;
        case 4:
            type.append('T');
        break;
        default:
        break;
    }
    return type;
}


int MainWindow::findCarType(QString type)
{
    if(type.contains('S'))
    {
        return 1;
    }
    else if(type.contains('H'))
    {
        return 2;
    }
    else if(type.contains('A'))
    {
        return 3;
    }
    else if(type.contains('T'))
    {
        return 4;
    }else
    {
        return 0;
    }
}

QString MainWindow::findPackType(int index)
{
    QString type;
    switch(index)
    {
        case 1:
            type.append('B');
        break;

        case 2:
            type.append('S');
        break;

        case 3:
            type.append('C');
        break;
        default:
        break;
    }
    return type;
}

void MainWindow::on_btnSendCard_clicked()
{
    m_userIdReader.read_card();
}

void MainWindow::queryCarFromDB(QString license)
{
    //查询数据库信息
    QSqlQuery query;
    query.prepare("select C_VEHICLEINFO_ID,VALUE,TYPE,COLOR from C_VEHICLEINFO where VALUE = :LICENSE and ISACTIVE = 'Y' order by created desc");
    query.bindValue(":LICENSE",license);
    query.exec();
    query.first();
    if(query.isValid())
    {
        qDebug()<<"数据库存在这个车牌";
        mDBhaveCarInfo = true;
        QSqlRecord record= query.record();
        mCurCarDBID = record.value(0).toInt();
        QString type = record.value(2).toString();
        QString color = record.value(3).toString();
        if(type.isEmpty() || color.isEmpty())
        {
            mDBhaveCarInfo = false;
        }
        if(!type.isEmpty())
        {
            int res = findCarType(type);
            ui->cboCarType->setCurrentIndex(res);
        }
        if(!color.isEmpty())
        {
            ui->editCarColor->setText(color);
        }
    }
    else
    {
         qDebug()<<"error:数据库不存在这个车牌";
         mDBhaveCarInfo = false;
    }
}

bool MainWindow::checkLineEdit1()
{
    if(ui->cboCarType->currentIndex() == 0)
    {
        QMessageBox::warning(NULL,"错误","请选择车辆类型","确定");
        return false;
    }
    if(ui->cboPackType->currentIndex() == 0)
    {
        QMessageBox::warning(NULL,"错误","请选择包装类型","确定");
        return false;
    }
    if(ui->editCarColor->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","车辆颜色不能为空","确定");
        return false;
    }
    if(ui->editCarID->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","车牌号码不能为空","确定");
        return false;
    }
    if( ui->editPersonalUserAddr->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","身份证地址不能为空","确定");
        return false;
    }
    if( ui->editPersonalUserID->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","身份证号码不能为空","确定");
        return false;
    }
    if(ui->editPersonalUserName->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","身份证姓名不能为空","确定");
        return false;
    }
    if(ui->editTagPower->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","电子标签电量不能为空","确定");
        return false;
    }
    if(ui->editSettlementCardID->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","结算卡号不能为空","确定");
        return false;
    }
    if(ui->editTagID->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","电子标签号不能为空","确定");
        return false;
    }
    if(ui->editTagStatus->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","电子标签状态不能为空","确定");
        return false;
    }
    return true;
}

bool MainWindow::checkLineEdit2()
{
    if(ui->editCarColor->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","车辆颜色不能为空","确定");
        return false;
    }
    if(ui->editCarID->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","车牌号码不能为空","确定");
        return false;
    }
    if(ui->editTagPower->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","电子标签电量不能为空","确定");
        return false;
    }
    if(ui->editSettlementCardID->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","结算卡号不能为空","确定");
        return false;
    }
    if(ui->editTagID->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","电子标签号不能为空","确定");
        return false;
    }
    if(ui->editContractID->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","合同号不能为空","确定");
        return false;
    }
    if(ui->editContractSource->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","发站地点不能为空","确定");
        return false;
    }
    if(ui->editContractSupplier->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","供应商不能为空","确定");
        return false;
    }
    if(ui->editContractTransport->text().isEmpty())
    {
        QMessageBox::warning(NULL,"错误","车船好不能为空","确定");
        return false;
    }
    if(ui->cboCarType->currentIndex() == 0)
    {
        QMessageBox::warning(NULL,"错误","请选择车辆类型","确定");
        return false;
    }
    if(ui->cboPackType->currentIndex() == 0)
    {
        QMessageBox::warning(NULL,"错误","请选择包装类型","确定");
        return false;
    }
    return true;
}

bool MainWindow::insertCarInfo()
{
    if(!mDBhaveCarInfo)
    {
        qDebug()<<"调用车表API,存储车辆信息,存储的车牌号是:"<<ui->editCarID->text();
        //调用WEBAPI保存车牌信息,服务器会返回插入车牌信息的主键ID号
        QStringList list;
        QString type = findCarType(ui->cboCarType->currentIndex());

        list<<QString::number(mClientID)<<QString::number(mOrgID)<<QString::number(mRoleID)<<QString::number(mUserID)\
                                        <<ui->editCarID->text()<<type<<ui->editCarColor->text();
        QString result = webAPI.saveVehicleInfo(list);
        list.clear();
        list = result.split(',');
        if((list.at(0).contains("成功") == true) || list.at(0).contains("已存在") == true)
        {
            qDebug()<<"车辆信息添加数据库成功,返回的ID:"<<list.at(1);
            mCurCarDBID = list.at(1).toInt();
        }else
        {
            qDebug()<<"error:保存车辆信息失败";
            QMessageBox::warning(NULL,"错误",list.at(1),"确定");
            return false;
        }
     }
    return true;
}

bool MainWindow::insertUserInfo()
{
    if(!mDBhaveUserInfo)
    {
        //调用WEBAPI保存身份证信息,服务器会返回插入身份证信息的主键ID号
        QStringList list;
        list<<QString::number(mClientID)<<QString::number(mOrgID)<<QString::number(mRoleID)<<QString::number(mUserID)\
            <<ui->editPersonalUserName->text()<<ui->editPersonalUserID->text()<<ui->editPersonalUserAddr->text();
        QString result = webAPI.saveGrainUser(list);
        list.clear();
        list = result.split(',');
        if(list.at(0).contains("成功") == true || list.at(0).contains("已存在") == true)
        {
            qDebug()<<"身份信息添加数据库成功,返回的ID:"<<list.at(1);
            mCurC_GRAINUSER_ID = list.at(1).toInt();
        }else
        {
            qDebug()<<"error:保存身份证信息失败";
            QMessageBox::warning(NULL,"错误",list.at(1),"确定");
            return false;
        }
    }else
    {
        qDebug()<<"身份证信息存在不需要保存";
    }
    return true;
}

QString MainWindow::savePersonalInfo()
{
    QString type = findCarType(ui->cboCarType->currentIndex());
    qDebug()<<type;
    QString packType = findPackType(ui->cboPackType->currentIndex());
    qDebug()<<packType;
    //调用WEBAPI保存发卡信息
    QStringList list;
    list<<QString::number(mClientID)<<QString::number(mOrgID)<<QString::number(mRoleID)<<QString::number(mUserID)\
        <<ui->editPersonalUserAddr->text()<<""<<"0"<<"0"<<""<<QString::number(mCurC_GRAINUSER_ID)<<""<<""\
        <<QString::number(mCurCarDBID)<<""<<ui->editCarColor->text()\
        <<ui->editTagPower->text()<<"CO"<<"IP"<<""<<ui->editPersonalUserID->text()<<""<<"N"<<"N"\
        <<ui->editCarID->text()<<packType<<"N"<<"N"<<""<<""<<ui->editSettlementCardID->text()<<""\
        <<ui->editTagID->text()<<type<<""<<"";
    QString result = webAPI.saveGPOrders(list);
    return result;
}

QString MainWindow::saveContractInfo()
{
    QString type = findCarType(ui->cboCarType->currentIndex());
    qDebug()<<type;
    QString packType = findPackType(ui->cboPackType->currentIndex());
    qDebug()<<packType;
    //调用WEBAPI保存发卡信息
    QStringList list;
    list<<QString::number(mClientID)<<QString::number(mOrgID)<<QString::number(mRoleID)<<QString::number(mUserID)\
        <<""<<QString::number(c_contract_id)<<"0"<<"0"<<""<<""<<QString::number(c_location_id)<<QString::number(c_targetvehicle_id)\
        <<QString::number(mCurCarDBID)<<QString::number(c_vendor_id)<<ui->editCarColor->text()<<ui->editTagPower->text()<<"CO"<<"IP"\
        <<""<<""<<""<<"N"<<"Y"<<ui->editCarID->text()<<packType<<"N"<<"N"<<""<<""<<ui->editSettlementCardID->text()<<""\
        <<ui->editTagID->text()<<type<<""<<"";
    QString result = webAPI.saveGPOrders(list);
    return result;
}

//提交数据
void MainWindow::on_btnSave_clicked()
{
    QString res;
    if(ui->stackedWidget->currentIndex() == 0) //散收粮
    {
        qDebug("检查散收粮界面字段是否填写齐全");
        if(!checkLineEdit1())
            return;
        if(!DB.isOpen())
        {
            QMessageBox::warning(NULL,"保存提示","与数据库断开连接","确定");
            return;
        }
         queryCarFromDB(ui->editCarID->text());
         if(!insertCarInfo())
         {
             QMessageBox::warning(NULL,"保存提示","在向数据库添加新的车牌信息时出现错误","确定");
             return;
         }
         if(!insertUserInfo())
         {
             QMessageBox::warning(NULL,"保存提示","在向数据库添加新的身份信息时出现错误","确定");
             return;
         }
         res = savePersonalInfo();
    }else
    {
        qDebug("检查合同粮界面 N字段是否填写齐全");
        if(!checkLineEdit2())
            return;
        if(!DB.isOpen())
        {
            QMessageBox::warning(NULL,"保存提示","与数据库断开连接","确定");
            return;
        }
        queryCarFromDB(ui->editCarID->text());
        if(!insertCarInfo())
        {
            QMessageBox::warning(NULL,"保存提示","在向数据库添加新的车牌信息时出现错误","确定");
            return;
        }
        res = saveContractInfo();
    }
    if(res.contains("成功"))
    {
        if(ledIsConnected == true)
            m_ledControl.setText("请入场",GREEN,14);
        tts.say(ui->editCarID->text()+"请入场");
        qDebug()<<ui->editCarID->text()+"数据保存成功";
        timerLed.start(30000);
        clearInterFace(ui->stackedWidget->currentIndex());
        mUdpSocket.writeDatagram("SYNUPADATE",QHostAddress::Broadcast,2538); //通知其他客户端有发卡记录
        QMessageBox::information(NULL,"保存提示","数据保存成功","确定");
        ui->btnSave->setEnabled(false);
    }
    else if(res.contains("二次进场"))
    {
        tts.say(ui->editCarID->text()+"不允许二次进场");
        qDebug()<<ui->editCarID->text()+"不允许二次进场";
        QMessageBox::warning(NULL,"保存提示","此车辆不允许当日二次进场","确定");
        clearInterFace(ui->stackedWidget->currentIndex());
        ui->btnSave->setEnabled(false);
    }
    else if(res.contains("计价方式维护错误"))
    {
        qDebug()<<ui->editCarID->text()+"计价方式维护错误";
        QMessageBox::warning(NULL,"保存提示","此车辆计价方式维护错误,请联系业务管理员进行处理","确定");
        clearInterFace(ui->stackedWidget->currentIndex());
        ui->btnSave->setEnabled(false);
    }
    else
    {
        qDebug()<<ui->editCarID->text()+"保存失败,错误原因是:"+res;
        QMessageBox::warning(NULL,"保存失败","错误原因是:"+res,"确定");
    }
}

void MainWindow::on_editContractTransport_returnPressed()
{
    if(ui->editContractTransport->hasFocus())
    {
        if(!DB.isOpen())
        {
            QMessageBox::warning(NULL,"保存提示","与数据库断开连接","确定");
            return;
        }
        if(ui->editContractTransport->text().isEmpty())
        {
            QMessageBox::warning(NULL,"错误提示","车船号不能为空","确定");
            return;
        }
        QSqlQuery query;
        QString str = "select * from (select b.C_VEHICLEINFO_ID,b.C_CONTRACT_ID,c.DOCUMENTNO,b.C_BPARTNER_ID,d.VALUE,d.NAME,b.C_LOCATION_ID,e.CITY,e.ADDRESS1,e.ADDRESS2,e.ADDRESS3,e.ADDRESS4 from C_VEHICLEINFO a left join C_CONTRACTLINE b on a.C_VEHICLEINFO_ID = b.C_VEHICLEINFO_ID left join C_CONTRACT c on b.C_CONTRACT_ID = c.C_CONTRACT_ID left join C_BPARTNER d on b.C_BPARTNER_ID = d.C_BPARTNER_ID left join C_LOCATION e on b.C_LOCATION_ID = e.C_LOCATION_ID where a.VALUE ='"\
                + ui->editContractTransport->text() + "' and to_char(sysdate,'yyyy-mm-dd') between to_char(c.VALIDFROM,'yyyy-mm-dd') and to_char(c.VALIDTO,'yyyy-mm-dd') and b.ISACTIVE = 'Y' and c.ISACTIVE = 'Y' order by b.created desc) where rownum = 1";
        query.exec(str);
        query.first();
        if(query.isValid())
        {
            QSqlRecord record= query.record();
            qDebug()<<"车船号ID:"<<record.value(0).toString();
            c_targetvehicle_id = record.value(0).toInt();

            qDebug()<<"合同号ID:"<<record.value(1).toString();
            c_contract_id = record.value(1).toInt();

            qDebug()<<"显示合同号:"<<record.value(2).toString();
            ui->editContractID->setText(record.value(2).toString());

            qDebug()<<"供应商ID:"<<record.value(3).toString();
            c_vendor_id = record.value(3).toInt();

            qDebug()<<"显示供应商:"<<record.value(4).toString();
            qDebug()<<"显示供应商:"<<record.value(5).toString();
            ui->editContractSupplier->setText(record.value(4).toString() + '-' + record.value(5).toString());

            qDebug()<<"发站地点ID"<<record.value(6).toString();
            c_location_id = record.value(6).toInt();

            qDebug()<<"发站1:"<<record.value(7).toString();
            qDebug()<<"发站2:"<<record.value(8).toString();
            qDebug()<<"发站3:"<<record.value(9).toString();
            qDebug()<<"发站4:"<<record.value(10).toString();
            qDebug()<<"发站5:"<<record.value(11).toString();
            QString Source;
            for(int i = 0; i < 5; i++)
            {
                if(!record.value(7+i).toString().isEmpty())
                {
                    Source.append(record.value(7+i).toString() + ",");
                }
            }
            Source.chop(1);
           ui->editContractSource->setText(Source);
        }else
        {
            QMessageBox::warning(NULL,"错误提示","合同信息未录入或合同已到期","确定");
            return;
        }
    }
}

void MainWindow::on_editPersonalUserID_returnPressed()
{
    if(!DB.isOpen())
    {
        QMessageBox::warning(NULL,"保存提示","与数据库断开连接","确定");
        return;
    }
    if(ui->editPersonalUserID->hasFocus())
    {
        QSqlQuery query;
        QString str = "select C_GRAINUSER_ID,name,address1 from C_GRAINUSER where idcard = '" + ui->editPersonalUserID->text()+ "'";
        query.exec(str);
        query.first();
        if(query.isValid())
        {
            QSqlRecord record= query.record();
            ui->editPersonalUserAddr->setText(record.value(2).toString());
            ui->editPersonalUserName->setText(record.value(1).toString());
            mCurC_GRAINUSER_ID = record.value(0).toInt();
            mDBhaveUserInfo = true;
        }else
        {
            QMessageBox::warning(NULL,"查找错误","没有查找此身份证的信息");
        }
    }
}

void MainWindow::on_editCarID_returnPressed()
{
    if(ui->editCarID->hasFocus())
    {
        if(!DB.isOpen())
        {
            QMessageBox::warning(NULL,"保存提示","与数据库断开连接","确定");
            return;
        }
         queryCarFromDB(ui->editCarID->text()); //查询当前抓拍的车牌是否在数据库中存在
    }
}

void MainWindow::setLoginInfo()
{
    mClientID = mLoginUserFrom->mClientID;//公司ID
    mOrgID =    mLoginUserFrom->mOrgID; //部门ID
    mUserID =   mLoginUserFrom->mUserID; //用户ID
    mRoleID =   mLoginUserFrom->mRoleID; //用户权限
    qDebug()<<mClientID<<mOrgID<<mUserID<<mRoleID;
    labLogin->setText("登录人:" + mLoginUserFrom->mName);
}

void MainWindow::on_actLogin_triggered()
{
    if(mLoginUserFrom->exec() == QDialog::Accepted)
    {
        setLoginInfo();
    }
}

void MainWindow::setLoginDialog(loginDialog *dialog)
{
    mLoginUserFrom = dialog;
    setLoginInfo();
}

//修改密码
void MainWindow::on_actUpdatePasswd_triggered()
{
    if(mModifyPASW == nullptr)
    {
        mModifyPASW  = new ModifyPASW;
        mModifyPASW->setWebApiAddress(serviceIP);
        mModifyPASW->setUserInfo(mClientID,mOrgID,mUserID,mRoleID);
    }
    if(mModifyPASW->exec() == QDialog::Accepted)
    {
        QMessageBox::information(NULL,"更改提示","密码修改成功","确定");
    }
}

void MainWindow::onTimerCounter()
{
    labTimer->setText(QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss"));
}

void MainWindow::on_actHDInfo_triggered()
{
    m_formLogin.exec();
}

void MainWindow::on_actAbout_triggered()
{
    QMessageBox::about(NULL,"关于","软件版本V1.0");
}

