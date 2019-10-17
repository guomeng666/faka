#include "reader1356.h"

Reader1356::Reader1356()
{

}

bool Reader1356::connectReader(QString com, qint32 baudRate)
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


unsigned char CMDREAD[3] = {0x10, 0xFF ,0x4D};
void Reader1356::readUid()
{
    if(isOpen())
        write((char*)CMDREAD,3);
}




