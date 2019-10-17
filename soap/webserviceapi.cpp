#include "webserviceapi.h"
#include <QStringList>
#include <QString>
#include <QDebug>
#include <iostream>

WebServiceAPI::WebServiceAPI()
{
    soap_init(&mAdd_soap);
    mAdd_soap.send_timeout = 5;
    mAdd_soap.recv_timeout = 5;
    soap_set_mode(&mAdd_soap, SOAP_C_UTFSTRING);
}

std::string WebServiceAPI::stringToUtf8(const std::string &str)
{
    int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

    wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
    ZeroMemory(pwBuf, nwLen * 2 + 2);

    ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

    int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

    char * pBuf = new char[nLen + 1];
    ZeroMemory(pBuf, nLen + 1);

    ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

    std::string retStr(pBuf);

    delete []pwBuf;
    delete []pBuf;

    pwBuf = NULL;
    pBuf = NULL;

    return retStr;
}
std::string WebServiceAPI::UTF8_To_string(const std::string & str)
{
    int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

    wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
    memset(pwBuf, 0, nwLen * 2 + 2);

    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);

    int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

    char * pBuf = new char[nLen + 1];
    memset(pBuf, 0, nLen + 1);

    WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

    std::string retStr = pBuf;

    delete []pBuf;
    delete []pwBuf;

    pBuf = NULL;
    pwBuf = NULL;

    return retStr;
}

void WebServiceAPI::setWebServiceAddress(QString addr)
{
    mAddr = addr;
}

QString WebServiceAPI::saveVehicleInfo(QStringList argList)
{
    QString resStr;
    web__saveVehicleInfo *in = new web__saveVehicleInfo;
    in->arg0 = new std::string;
    in->arg1 = new std::string;
    in->arg2 = new std::string;
    in->arg3 = new std::string;
    in->arg4 = new std::string;
    in->arg5 = new std::string;
    in->arg6 = new std::string;

    in->arg0->append(argList.at(0).toStdString()) ;
    in->arg1->append(argList.at(1).toStdString());
    in->arg2->append(argList.at(2).toStdString());
    in->arg3->append(argList.at(3).toStdString());
    in->arg4->append(argList.at(4).toStdString()) ;
    in->arg5->append(argList.at(5).toStdString());
    in->arg6->append(argList.at(6).toStdString());
    in->soap = &mAdd_soap;

    web__saveVehicleInfoResponse out;
    out.return_ = new std::string;
    out.soap = &mAdd_soap;
    soap_call___web__saveVehicleInfo(&mAdd_soap, mAddr.toLocal8Bit().data(), NULL, in, out);

    char errorbuf[200];
    if(mAdd_soap.error)
    {
        sprintf(errorbuf,"soap error:%d,%s,%s\n",mAdd_soap.error, *soap_faultcode(&mAdd_soap), *soap_faultstring(&mAdd_soap));
        resStr = QString::fromLocal8Bit(errorbuf);
    }else
    {
        resStr = QString::fromUtf8((const char*)(out.return_->data()));
    }
    soap_end(&mAdd_soap);
    soap_done(&mAdd_soap);
    qDebug()<<resStr;
    return resStr;
}

QString WebServiceAPI::saveGPOrders(QStringList argList)
{
    QString resStr;
    web__saveGPOrders *in = new web__saveGPOrders;
    in->arg0 = new web__gpOrders;

    in->arg0->address1 = new std::string;
    in->arg0->color = new std::string;
    in->arg0->docaction = new std::string;
    in->arg0->docstatus = new std::string;
    in->arg0->documentno = new std::string;
    in->arg0->idcard = new std::string;
    in->arg0->idcardbutton = new std::string;
    in->arg0->license = new std::string;
    in->arg0->packingtype = new std::string;
    in->arg0->recheckbutton = new std::string;
    in->arg0->settlementcardbutton = new std::string;
    in->arg0->settlementcardid = new std::string;
    in->arg0->tagbutton = new std::string;
    in->arg0->tid = new std::string;
    in->arg0->type = new std::string;
    in->arg0->vehiclebutton = new std::string;
    in->arg0->vehicleImage = new std::string;

    in->arg0->ad_USCOREclient_USCOREid = argList[0].toInt();
    in->arg0->ad_USCOREorg_USCOREid = argList[1].toInt();
    in->arg0->ad_USCORErole_USCOREid = argList[2].toInt();
    in->arg0->ad_USCOREuser_USCOREid = argList[3].toInt();
    in->arg0->address1->append(argList[4].toStdString());
    in->arg0->c_USCOREcontract_USCOREid = argList[5].toInt();
    in->arg0->c_USCOREdoctype_USCOREid = argList[6].toInt();
    in->arg0->c_USCOREdoctypetarget_USCOREid = argList[7].toInt();
    in->arg0->c_USCOREgporders_USCOREid = argList[8].toInt();
    in->arg0->c_USCOREgrainuser_USCOREid = argList[9].toInt();
    in->arg0->c_USCORElocation_USCOREid = argList[10].toInt();
    in->arg0->c_USCOREtargetvehicle_USCOREid = argList[11].toInt();
    in->arg0->c_USCOREvehicleinfo_USCOREid = argList[12].toInt();
    in->arg0->c_USCOREvendor_USCOREid = argList[13].toInt();
    in->arg0->color->append(argList[14].toStdString());
    in->arg0->currentelectricity = argList[15].toInt();
    in->arg0->docaction->append(argList[16].toStdString());
    in->arg0->docstatus->append(argList[17].toStdString());
    in->arg0->documentno->append(argList[18].toStdString());
    in->arg0->idcard->append(argList[19].toStdString());
    in->arg0->idcardbutton->append(argList[20].toStdString());

    in->arg0->isapproved = argList[21].contains('Y') ? true : false;
    in->arg0->isvendor   = argList[22].contains('Y') ? true : false;

    in->arg0->license->append(argList[23].toStdString());
    in->arg0->packingtype->append(argList[24].toStdString());

    in->arg0->processed =  argList[25].contains('Y') ? true : false;
    in->arg0->processing = argList[26].contains('Y') ? true : false;

    in->arg0->recheckbutton->append(argList[27].toStdString());
    in->arg0->settlementcardbutton->append(argList[28].toStdString());
    in->arg0->settlementcardid->append(argList[29].toStdString());
    in->arg0->tagbutton->append(argList[30].toStdString());
    in->arg0->tid->append(argList[31].toStdString());
    in->arg0->type->append(argList[32].toStdString());
    in->arg0->vehicleImage->append(argList[33].toStdString());
    in->arg0->vehiclebutton->append(argList[34].toStdString());

    in->soap = &mAdd_soap;

    web__saveGPOrdersResponse out;
    out.return_ = new std::string;
    out.soap = &mAdd_soap;
    soap_call___web__saveGPOrders(&mAdd_soap, mAddr.toLocal8Bit().data(), NULL, in, out);

    char errorbuf[200];
    if(mAdd_soap.error)
    {
        sprintf(errorbuf,"soap error:%d,%s,%s\n",mAdd_soap.error, *soap_faultcode(&mAdd_soap), *soap_faultstring(&mAdd_soap));
        resStr = QString::fromLocal8Bit(errorbuf);
    }else
    {
        resStr = QString::fromUtf8((const char*)(out.return_->data()));
    }
    soap_end(&mAdd_soap);
    soap_done(&mAdd_soap);
    qDebug()<<resStr;
    return resStr;
}

QString WebServiceAPI::saveGrainUser(QStringList argList)
{
    QString resStr;
    web__saveGrainUser *in = new web__saveGrainUser;
    in->arg0 = new std::string;
    in->arg1 = new std::string;
    in->arg2 = new std::string;
    in->arg3 = new std::string;
    in->arg4 = new std::string;
    in->arg5 = new std::string;
    in->arg6 = new std::string;

    in->arg0->append(argList.at(0).toStdString());
    in->arg1->append(argList.at(1).toStdString());
    in->arg2->append(argList.at(2).toStdString());
    in->arg3->append(argList.at(3).toStdString());
    in->arg4->append(argList.at(4).toStdString());
    in->arg5->append(argList.at(5).toStdString());
    in->arg6->append(argList.at(6).toStdString());
    in->soap = &mAdd_soap;

    web__saveGrainUserResponse out;
    out.return_ = new std::string;
    out.soap = &mAdd_soap;
    soap_call___web__saveGrainUser(&mAdd_soap, mAddr.toLocal8Bit().data(), NULL, in, out);

    char errorbuf[200];
    if(mAdd_soap.error)
    {
        sprintf(errorbuf,"soap error:%d,%s,%s\n",mAdd_soap.error, *soap_faultcode(&mAdd_soap), *soap_faultstring(&mAdd_soap));
        resStr = QString::fromLocal8Bit(errorbuf);
    }else
    {
        resStr = QString::fromUtf8((const char*)(out.return_->data()));
    }
    soap_end(&mAdd_soap);
    soap_done(&mAdd_soap);
    qDebug()<<resStr;
    return resStr;
}

QString WebServiceAPI::webLogin(QStringList argList, web__user &webParameter)
{
    QString resStr;
    web__login *in = new web__login;
    in->arg0 = new std::string;
    in->arg1 = new std::string;
    in->soap = &mAdd_soap;

    in->arg0->append(argList.at(0).toStdString());
    in->arg1->append(argList.at(1).toStdString());

    web__loginResponse out;
    out.return_ = new web__user;
    out.return_->MESSAGE = new std::string;
    out.return_->NAME    = new std::string;
    out.soap = &mAdd_soap;

    soap_call___web__login(&mAdd_soap, mAddr.toLocal8Bit().data(), NULL, in, out);

    char errorbuf[200];
    if(mAdd_soap.error)
    {
        sprintf(errorbuf,"soap error:%d,%s,%s\n",mAdd_soap.error, *soap_faultcode(&mAdd_soap), *soap_faultstring(&mAdd_soap));
        resStr = QString::fromLocal8Bit(errorbuf);
    }else
    {
        webParameter = *(out.return_);
        resStr = QString::fromUtf8((const char*)(out.return_->MESSAGE->data()));;
    }
    qDebug()<<resStr;
    soap_end(&mAdd_soap);
    soap_done(&mAdd_soap);
    return resStr;
}

//修改登录密码
QString WebServiceAPI::updatePassword(QStringList argList)
{
    QString resStr;
    web__updatePassword *in = new web__updatePassword;
    in->arg0 = new std::string;
    in->arg1 = new std::string;
    in->arg2 = new std::string;
    in->soap = &mAdd_soap;

    in->arg0->append(argList.at(0).toStdString());
    in->arg1->append(argList.at(1).toStdString());
    in->arg2->append(argList.at(2).toStdString());

    web__updatePasswordResponse out;
    out.return_ = new std::string;
    out.soap = &mAdd_soap;

    soap_call___web__updatePassword(&mAdd_soap, mAddr.toLocal8Bit().data(), NULL, in, out);

    char errorbuf[200];
    if(mAdd_soap.error)
    {
        sprintf(errorbuf,"soap error:%d,%s,%s\n",mAdd_soap.error, *soap_faultcode(&mAdd_soap), *soap_faultstring(&mAdd_soap));
        resStr = QString::fromLocal8Bit(errorbuf);
    }else
    {
       resStr = QString::fromUtf8((const char*)(out.return_->data()));
    }
    qDebug()<<resStr;
    soap_end(&mAdd_soap);
    soap_done(&mAdd_soap);
    return resStr;
}
