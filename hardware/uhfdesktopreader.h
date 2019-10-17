#ifndef UHFDESKTOPREADER_H
#define UHFDESKTOPREADER_H

#include <QObject>
#include <QSerialPort>

#ifndef TAGINFO
#define TAGINFO
struct TagInfo
{
    QByteArray tagID;
    bool communicationStat;
    bool warningStat;
    quint8 workStat;
    quint8 warningTriggerValue;
    quint8 temperatureCompensation;
    quint8 activeTime;
    quint16 magneticTrigger;
    quint16 magneticReference;
    quint16 magneticRealTime;
    quint16 power;
};
#endif

class UHFDeskTopReader : public QSerialPort
{
    Q_OBJECT
public:
    explicit UHFDeskTopReader(QSerialPort *parent = nullptr);
    bool connectReader(QString com,qint32 baudRate); //连接读写器
    QByteArray readTID(bool &ok);
    QByteArray readEPC(bool &ok);
    TagInfo readTagStatus(bool &ok);
    bool writeTID(unsigned char *data, char len);
private:
    unsigned int uiCrc16Cal(unsigned char *pucY, unsigned char ucX);

signals:

public slots:
};

#endif // UHFDESKTOPREADER_H
