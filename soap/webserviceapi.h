#ifndef WEBSERVICEAPI_H
#define WEBSERVICEAPI_H
#include "soapH.h"
#include "soapStub.h"
#include "stdsoap2.h"
#include <stdio.h>
#include <QString>


class WebServiceAPI
{
public:
    WebServiceAPI();

    static std::string stringToUtf8(const std::string & str);
    static std::string UTF8_To_string(const std::string & str);
    void setWebServiceAddress(QString addr);

    QString saveVehicleInfo(QStringList argList);  //车牌
    QString saveGPOrders(QStringList argList);     //发卡
    QString savePoundInfoA(QStringList argList);   //自动过磅
    QString saveGrainUser(QStringList argList);    //身份证
    QString saveSamplers(QStringList argList);     //扦样
    QString saveCheckingInfo(QStringList argList); //化验数据
    QString savePoundInfo(QStringList argList);    //手动过磅
    QString webLogin(QStringList argList, web__user &webParameter);      //登录

    QString updatePassword(QStringList argList);
private:
    struct soap mAdd_soap;
    QString mAddr;
};

#endif // WEBSERVICEAPI_H
