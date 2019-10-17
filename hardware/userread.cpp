#include <QFile>
#include "userread.h"

ReadCard::ReadCard()
{
}

ReadCard::~ReadCard()
{
    if(func_cvr_CloseComm)
        func_cvr_CloseComm();
}

//加载dll中的函数指针
bool ReadCard::load_function()
{
    func_cvr_InitComm       = {nullptr};
    func_cvr_Authenticate   = {nullptr};
    func_cvr_Read_FPContent = {nullptr};
    func_cvr_GetPeopleName  = {nullptr};
    func_cvr_GetPeopleIDCode= {nullptr};
    func_cvr_CloseComm      = {nullptr};
    func_cvr_GetPeopleAddressU = {nullptr};
    qRegisterMetaType<stu_card_info>("stu_card_info");
    QLibrary lib("Termb.dll");

    if(lib.load())                      //加载身份证读取库中的函数
    {
        func_cvr_InitComm = (CVR_InitComm)lib.resolve("CVR_InitComm");
        if(!func_cvr_InitComm)
            qDebug()<<"CVR_InitComm 解析失败"<<lib.errorString();

        func_cvr_GetPeopleName = (CVR_GetPeopleName)lib.resolve("GetPeopleName");
        if(!func_cvr_GetPeopleName)
            qDebug()<<"CVR_GetPeopleName 解析失败"<<lib.errorString();

        func_cvr_GetPeopleIDCode = (CVR_GetPeopleIDCode)lib.resolve("GetPeopleIDCode");
        if(!func_cvr_GetPeopleIDCode)
            qDebug()<<"CVR_GetPeopleIDCode 解析失败"<<lib.errorString();

        func_cvr_GetPeopleAddressU = (CVR_GetPeopleAddressU)lib.resolve("GetPeopleAddress");
        if(!func_cvr_GetPeopleAddressU)
            qDebug()<<"GetPeopleAddress 解析失败"<<lib.errorString();

        func_cvr_Authenticate = (CVR_Authenticate)lib.resolve("CVR_Authenticate");
        if(!func_cvr_Authenticate)
            qDebug()<<"CVR_Authenticate 解析失败"<<lib.errorString();

        func_cvr_Read_FPContent = (CVR_Read_FPContent)lib.resolve("CVR_Read_FPContent");
        if(!func_cvr_Read_FPContent)
            qDebug()<<"CVR_Read_FPContent 解析失败"<<lib.errorString();

        func_cvr_CloseComm = (CVR_CloseComm)lib.resolve("CVR_CloseComm");
        if(!func_cvr_CloseComm)
            qDebug()<<"CVR_CloseComm 解析失败"<<lib.errorString();
    }
    else
      qDebug()<<"termb.dll load faild."<<lib.errorString();
    return (func_cvr_InitComm && func_cvr_Authenticate && func_cvr_Read_FPContent && func_cvr_GetPeopleName && func_cvr_GetPeopleIDCode && func_cvr_CloseComm);
}


//初始化
bool ReadCard::init_cvr()
{
    if(!load_function())    //加载函数失败
        return false;
    int iPort, iRetUSB = 0, iRetCOM = 0;
    for (iPort = 1001; iPort <= 1004; iPort++)  //USB
    {
        iRetUSB = func_cvr_InitComm(iPort);
        if (iRetUSB == 1)
            break;
    }
    if (iRetUSB != 1)
    {
        for (iPort = 1; iPort <= 4; iPort++)    //port
        {
            iRetCOM = func_cvr_InitComm(iPort);
            if (iRetCOM == 1)
                break;
        }
    }
    if ((iRetCOM == 1) || (iRetUSB == 1))
        return true;
    return false;
}

void ReadCard::startRead()
{
    this->start();
}
//读卡姓名身份证号
bool ReadCard::read_card_info()
{
    if(func_cvr_Read_FPContent() == 1)
    {
        memset(m_buf,0,m_buf_size);
        if(1 == func_cvr_GetPeopleName(m_buf,m_buf_size))
        {
            m_info.name = QString::fromLocal8Bit(m_buf);
            memset(m_buf,0,m_buf_size);
            if(1 == func_cvr_GetPeopleIDCode(m_buf,m_buf_size))
            {
                m_info.ID = m_buf;
                memset(m_buf,0,m_buf_size);
                if(func_cvr_GetPeopleAddressU(m_buf,&m_buf_size) == 1)
                {
                    m_info.address =  QString::fromLocal8Bit(m_buf);
                    emit read_finished(m_info);
                    //删除读取身份证生成的文件
                    QFile::remove("wz.txt");
                    QFile::remove("fp.dat");
                    QFile::remove("zp.bmp");
                    return true;
                }
            } 
        }
    }
    emit read_error();
    return false;
}
//读卡，尝试读，读失败尝试认证，认证成功再读一次
void ReadCard::read_card()
{
    if(!read_card_info())
    {
        if(1 == func_cvr_Authenticate())
        {
            read_card_info();
        }
    }
}

void ReadCard::run()
{
    while(1)
    {
        read_card();
        msleep(1000);
    }
}

