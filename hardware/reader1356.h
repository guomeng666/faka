#ifndef READER1356_H
#define READER1356_H
#include <QThread>
#include <QSerialPort>


class Reader1356 : public QSerialPort
{
    Q_OBJECT
public:
    Reader1356();

    bool connectReader(QString com,qint32 baudRate);

    void readUid();

private:
};

#endif // READER1356_H
