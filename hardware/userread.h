#ifndef READER_H
#define READER_H

#include <QObject>
#include <QThread>
#include <QLibrary>
#include <QDebug>

typedef int __stdcall (*CVR_InitComm)(int);                  //初始化
typedef int __stdcall (*CVR_Authenticate)();                  //卡认证
typedef int __stdcall (*CVR_Read_FPContent)();                //读卡
typedef int __stdcall (*CVR_GetPeopleName)(char *,int &);    //获取姓名
typedef int __stdcall (*CVR_GetPeopleIDCode)(char *,int &);  //获取身份证号
typedef int __stdcall (*CVR_GetPeopleAddressU)(char *, int *);//获取地址
typedef int __stdcall (*CVR_CloseComm)();                    //关闭链接


//身份证信息
struct stu_card_info
{
    QString name;
    QString ID;
    QString address;
};

class ReadCard:public QThread
{
    Q_OBJECT
public:
    ReadCard();
    ~ReadCard();
    bool init_cvr();
    void startRead();
    void read_card();
private:
    bool read_card_info();
    bool load_function();
    void run();
signals:
    void read_finished(stu_card_info info);
    void read_error();

public slots:

private:
    //身份证读取函数指针
    CVR_InitComm          func_cvr_InitComm          {nullptr};
    CVR_Authenticate      func_cvr_Authenticate      {nullptr};
    CVR_Read_FPContent    func_cvr_Read_FPContent    {nullptr};
    CVR_GetPeopleName     func_cvr_GetPeopleName     {nullptr};
    CVR_GetPeopleIDCode   func_cvr_GetPeopleIDCode   {nullptr};
    CVR_GetPeopleAddressU func_cvr_GetPeopleAddressU {nullptr};
    CVR_CloseComm         func_cvr_CloseComm         {nullptr};

    //读取身份证的缓冲区
    char                m_buf[1024];
    int                  m_buf_size = 1024;
    stu_card_info        m_info;
};


#endif
