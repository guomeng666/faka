#include "vehiclecamera.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QtEndian>

VehicleCamera::VehicleCamera(QObject *parent) : QTcpSocket(parent),m_recvLen(0)
{
    m_recvBuf.resize(1024*512);
    connect(this, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

VehicleCamera::~VehicleCamera()
{

}

void VehicleCamera::connectVehicleCamera(QString ip, quint16 port)
{
    if(state() == QAbstractSocket::SocketState::ConnectedState)
    {
        return;
    }
   connectToHost(ip,port);
}

void VehicleCamera::setCameraWorkMode(bool enable, QString format, bool image, quint8 image_type)
{
    QByteArray jsonData;
    QByteArray sendData;
    quint8 packageHead[8];
    QJsonDocument doc;
    QJsonObject rootObj;
    rootObj.insert("cmd",QJsonValue("ivsresult"));
    rootObj.insert("id",QJsonValue("123"));
    rootObj.insert("enable",QJsonValue(enable));
    rootObj.insert("format",QJsonValue(format));
    rootObj.insert("image",QJsonValue(image));
    rootObj.insert("image_type",QJsonValue(image_type));
    doc.setObject(rootObj);

    jsonData = doc.toJson(QJsonDocument::Compact);

    packageHead[0] = 'V';
    packageHead[1] = 'Z';
    packageHead[2] = 0x00;
    packageHead[3] = 0x00;
    quint32 len = jsonData.length();
    qToBigEndian<quint32>(len,packageHead+4);
    sendData.append((const char *)packageHead,8);
    sendData.append(jsonData);

    write(sendData);
}

void VehicleCamera::getivsresult()
{

}

void VehicleCamera::getVideo()
{
    QByteArray jsonData;
    QByteArray sendData;
    quint8 packageHead[8];
    QJsonDocument doc;
    QJsonObject rootObj;
    rootObj.insert("cmd",QJsonValue("get_rtsp_uri"));
    rootObj.insert("id",QJsonValue("132156"));
    doc.setObject(rootObj);

    jsonData = doc.toJson(QJsonDocument::Compact);

    packageHead[0] = 'V';
    packageHead[1] = 'Z';
    packageHead[2] = 0x00;
    packageHead[3] = 0x00;
    quint32 len = jsonData.length();
    qToBigEndian<quint32>(len,packageHead+4);
    sendData.append((const char *)packageHead,8);
    sendData.append(jsonData);

    write(sendData);
}

void VehicleCamera::manualTrigger()
{

}

void VehicleCamera::sendHeart()
{
    quint8 packageHead[8];
    packageHead[0] = 'V';
    packageHead[1] = 'Z';
    packageHead[2] = 0x01;
    packageHead[3] = 0x00;
    packageHead[4] = 0x00;
    packageHead[5] = 0x00;
    packageHead[6] = 0x00;
    packageHead[7] = 0x00;
    if(isConnected())
        write((const char*)packageHead, 8);

}

bool VehicleCamera::isConnected()
{
    if(state() == QAbstractSocket::SocketState::ConnectedState)
        return true;
    else
        return false;
}

void VehicleCamera::onReadyRead()
{
    QByteArray recvData = readAll();

    if((recvData[0] == 'V') && (recvData[1] == 'Z') && (recvData[2] == 0x01))//心跳回应
    {
        return;
    }
    else if((recvData[0] == 'V') && (recvData[1] == 'Z') && (recvData[2] == 0x00))//包头开始
    {
        m_recvBuf.clear();
        m_recvLen = 0;
        char *str = recvData.data();
        m_recvLen = (0xff&str[4])<<24 | (0xff&str[5]) << 16 | (0xff&str[6]) << 8 | (0xff&str[7]);
        m_recvBuf.append(recvData.mid(8));
    }
    else//包体接收
    {
        m_recvBuf.append(recvData);

    }
    if(m_recvBuf.length() == m_recvLen)//接收完成
    {
        //qDebug()<<"abc:"<<m_recvBuf.toHex();
        emit readyCarData(m_recvBuf);
    }
}


