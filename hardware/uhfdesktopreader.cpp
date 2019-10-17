#include "uhfdesktopreader.h"
#include "qdebug.h"

UHFDeskTopReader::UHFDeskTopReader(QSerialPort *parent) : QSerialPort(parent)
{

}


#define PRESET_VALUE 0xFFFF
#define POLYNOMIAL 0x8408

unsigned int UHFDeskTopReader::uiCrc16Cal(unsigned char *pucY, unsigned char ucX)
{
    unsigned char ucI,ucJ;
    unsigned short int uiCrcValue = PRESET_VALUE;
    for(ucI = 0; ucI < ucX; ucI++)
    {
        printf("%x ",pucY[ucI]);
        uiCrcValue = uiCrcValue ^ *(pucY + ucI);
        for(ucJ = 0; ucJ < 8; ucJ++)
        {
            if(uiCrcValue & 0x0001)
            {
                uiCrcValue = (uiCrcValue >> 1) ^ POLYNOMIAL;
            }
            else
            {
                uiCrcValue = (uiCrcValue >> 1);
            }
        }
    }
    fflush(stdout);
    return uiCrcValue;
}

bool UHFDeskTopReader::connectReader(QString com, qint32 baudRate)
{
    setPortName(com);
    if(open(QIODevice::ReadWrite))
    {
        setBaudRate(baudRate);
        setDataBits(QSerialPort ::Data8);
        setFlowControl(QSerialPort ::NoFlowControl);
        setParity(QSerialPort ::NoParity);
        setStopBits(QSerialPort ::OneStop);
        return true;
    }
    return false;
}

const unsigned char cmd_EPC[6] = {0x04, 0x00, 0x01, 0xdb, 0x4b}; //读取ECP命令

QByteArray UHFDeskTopReader::readEPC(bool &ok)
{
    QByteArray epc;
    char resData[30];
    int epclen;
    write((const char *)cmd_EPC,sizeof(cmd_EPC));
    memset(resData,0,30);
    if(waitForReadyRead() == true)
    {
        epc = readAll();
        if(epc.length() > 6) //如果不大于6个字节说明没有读取到电子标签
        {
            epclen = epc[5]&0xff;
            epc = epc.mid(6,epclen);
            ok = true;
            return epc;
        }else
        {
            ok = false;
            return epc;
        }
    }else
    {
        ok = false;
        return epc;
    }
}

/*
8.2.2 读数据
这个命令读取标签的保留区、 EPC 存储区、 TID 存储区或用户存储区中的数据。从指定的
地址开始读，以字为单位。
命令：
Len Adr Cmd Data[] CRC-16
0xXX 0xXX 0x02 —— LSB MSB
Data 参数如下：
Data[]
ENum EPC Mem WordPtr Num Pwd MaskAdr MaskLen
0xXX 变长 0xXX 0xXX 0xXX 4Byte 0xXX 0xXX
参数解析：
ENum： EPC 号长度， 以字为单位。 EPC 的长度在 15 个字以内，可以为 0。超出范围，
将返回参数错误信息。
EPC：要读取数据的标签的 EPC 号。长度根据所给的 EPC 号决定， EPC 号以字为单位，
且必须是整数个长度。高字在前，每个字的高字节在前。这里要求给出的是完整的 EPC 号。D2180U 通讯协议 v1.1
15
Mem：一个字节。选择要读取的存储区。 0x00：保留区； 0x01： EPC 存储区； 0x02： TID
存储区； 0x03：用户存储区。其他值保留。若命令中出现了其它值，将返回参数出错的消息。
WordPtr：一个字节。指定要读取的字起始地址。 0x00 表示从第一个字(第一个 16 位存储
区)开始读， 0x01 表示从第 2 个字开始读，依次类推。
Num：一个字节。要读取的字的个数。不能设置为 0x00，否则将返回参数错误信息。 Num
不能超过 120，即最多读取 120 个字。若 Num 设置为 0 或者超过了 120，将返回参数出错的
消息。
Pwd：四个字节，这四个字节是访问密码。 32 位的访问密码的最高位在 Pwd 的第一字节(从
左往右)的最高位，访问密码最低位在 Pwd 第四字节的最低位， Pwd 的前两个字节放置访问密
码的高字。只有当读保留区，并且相应存储区设置为密码锁、且标签的访问密码为非 0 的时
候，才需要使用正确的访问密码。在其他情况下， Pwd 为零或正确的访问密码。
MaskAdr：一个字节，掩模 EPC 号的起始字节地址。 0x00 表示从 EPC 号的最高字节开始
掩模， 0x01 表示从 EPC 号的第二字节开始掩模，以此类推。
MaskLen：一个字节，掩模的字节数。掩模起始字节地址+掩模字节数不能大于 EPC 号字
节长度，否则返回参数错误信息。
注：当 MaskAdr、 MaskLen 为空时表示以完整的 EPC 号掩模。
应答：
Len Adr reCmd Status Data[] CRC-16
0xXX 0xXX 0x02 0x00 Word1， Word2,… LSB MSB
参数解析：
Word1,Word2,….： 以字为单位。每个字都是 2 个字节，高字节在前。 Word1 是从起
始地址读到的字， Word2 是起始地址后一个字地址上读到的字，以此类推。
*/
/*1A 00 02 06 30 08 33 B2 DD D9 01 40 00 00 00 00 02 06 06 00 00 00 00 00 00 6d 99*/
//读取ECP命令
//第4字节-16字节 = EPC码
unsigned char cmd_TID[] = {0x1A, 0x00, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
QByteArray UHFDeskTopReader::readTID(bool &ok)
{
    QByteArray epc;
    QByteArray tid;
    bool readok = false;
    epc = readEPC(readok);
    if(readok == true)
    {
        if(epc.length() != 12)
        {
            ok = false;
            return tid;
        }
        memcpy(cmd_TID+4, epc.data(),12);//填充ECP码
        unsigned short crc1 = uiCrc16Cal((unsigned char *)cmd_TID,sizeof(cmd_TID)-2);
        cmd_TID[25] = *((unsigned char*)&crc1);
        cmd_TID[26] = *((unsigned char*)&crc1+1);
        write((const char *)cmd_TID,sizeof(cmd_TID));
        if(waitForReadyRead() == false)
        {
            ok = false;
            return tid;
        }
        tid = readAll();
        qDebug()<<tid.toHex();
        if(tid.length() > 10)
        {
            tid = tid.mid(4,18);
            ok = true;
            return tid;
        }
    }
    ok = false;
    return tid;
}

TagInfo UHFDeskTopReader::readTagStatus(bool &ok)
{
    QByteArray tid;
    TagInfo tagInfo;
    bool readok;
    tid = readTID(readok);
    if(readok)
    {
        qDebug()<<"tid:"<<tid.toHex();
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

        ok = true;

        return tagInfo;
    }
    ok = false;
    return tagInfo;
}

unsigned char cmd_WriteTID[100];

bool UHFDeskTopReader::writeTID(unsigned char *data, char len)
{
    int num = 0;
    cmd_WriteTID[num++] = 0x00;//长度先占位
    cmd_WriteTID[num++] = 0x00;
    cmd_WriteTID[num++] = 0x03;
    cmd_WriteTID[num++] = len/2;
    QByteArray epc;
    bool readok = false;
    epc = readEPC(readok);
    if(readok)
    {
        if(epc.length() != 12)
        {
            return false;
        }
        qDebug()<<epc.toHex();
        cmd_WriteTID[num++] = 12/2;
        memcpy(cmd_WriteTID + 5, epc.data() , 12);
        num += 12;
        cmd_WriteTID[num++] = 0x02; //写TID区
        cmd_WriteTID[num++] = 6;//要写入的起始地址
        memcpy(cmd_WriteTID + 19, data , len);
        num += len;
        cmd_WriteTID[num++] = 0x00; //密码区
        cmd_WriteTID[num++] = 0x00; //密码区
        cmd_WriteTID[num++] = 0x00; //密码区
        cmd_WriteTID[num++] = 0x00; //密码区

        cmd_WriteTID[num++] = 0x00;
        cmd_WriteTID[num++] = 0x00;
        cmd_WriteTID[0] = num+1;

        unsigned short crc1 = uiCrc16Cal((unsigned char *)cmd_WriteTID,num);
        cmd_WriteTID[num++] = *((unsigned char*)&crc1);; //CRC校验区
        cmd_WriteTID[num++] = *((unsigned char*)&crc1+1);;   //CRC校验区

        qDebug()<<"   ";
        for(int i = 0; i < num; i++)
            printf("0x%x ",cmd_WriteTID[i]);
        fflush(stdout);
        write((const char *)cmd_WriteTID,num);
        if(waitForReadyRead() == true)
        {
               QByteArray res = readAll();
               qDebug()<<res.toHex();
               if(res[3] == (char)0)
                   return true;
        }
    }
    return false;
}


