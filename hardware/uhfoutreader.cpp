#include "uhfoutreader.h"
#include <QByteArray>

UhfOutReader::UhfOutReader(QTcpSocket *parent) : QTcpSocket(parent)
{
    connect(this,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
}

bool UhfOutReader::connectReader(QString ip, qint16 port)
{
    abort();
    connectToHost(ip,port);
    return true;
}

/*0A 00 09 88 00 00 00 00 02 03 09 57*/

unsigned char ReadCmd[12] = {0x0A, 0x00, 0x09, 0x88, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x09, 0x57};
void UhfOutReader::readTID()
{
    if(state() == QAbstractSocket::SocketState::ConnectedState)
        write((char*)ReadCmd,12);
}

quint8 UhfOutReader::crcCheck(unsigned char *data, quint32 len)
{
    quint32 sum = 0;
    for(int i= 0; i < len; i++)
        sum += data[i];

    qDebug()<<sum;

    sum = ~sum;
    sum += 1;
    return sum&0xff;
}

void UhfOutReader::onReadyRead()
{
    QByteArray recvdata = readAll();
    if(recvdata[0] == 0x02 && recvdata.length() > 30)
    {
        recvdata = recvdata.mid(7,36);
        QString tidStr = recvdata;
        qDebug() << "主动上传的数据:" << tidStr;
        QByteArray tidHex = QByteArray::fromHex(tidStr.toLatin1());

        qDebug()<< tidHex.toHex();
        TagInfo tagInfo = parseTagInfo(tidHex);
        emit recvTagInfo(tagInfo);
    }
    else if(recvdata[0] == 0x0b && recvdata.length() > 7)
    {
        qDebug()<<"读取的TID"<<recvdata.toHex();

        TagInfo tagInfo = parseTagInfo(recvdata.mid(5,18));
        emit recvTagInfo(tagInfo);
    }
}

TagInfo UhfOutReader::parseTagInfo(QByteArray tid)
{
    TagInfo tagInfo;

    //qDebug()<<"parsetid:"<<tid.toHex();

    tagInfo.tagID = tid.mid(0,6);
    char *bits = tid.data()+6;

    tagInfo.communicationStat = bits[0] & 0x80;

    tagInfo.warningStat = bits[0] & 0x40;

    tagInfo.workStat = (bits[0] & 0x30) >> 4;

    tagInfo.warningTriggerValue = (bits[0] & 0x0f);

    tagInfo.temperatureCompensation = (bits[1] & 0xf0) >> 4;

    tagInfo.activeTime = (bits[1] & 0x0f);

    tagInfo.magneticTrigger =   ((bits[2]&0xff ) << 8) | (bits[3]&0xff);

    tagInfo.magneticReference = ((bits[4]&0xff ) << 8) | (bits[5]&0xff);

    tagInfo.magneticRealTime =  ((bits[6]&0xff)  << 8) | (bits[7]&0xff);

    tagInfo.power = ((bits[8] & 0xff) << 8) | (bits[9] & 0xff) ;

    return tagInfo;
}
