#ifndef UHFOUTREADER_H
#define UHFOUTREADER_H

#include <QObject>
#include <QTcpSocket>

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

class UhfOutReader : public QTcpSocket
{
    Q_OBJECT
public:
    explicit UhfOutReader(QTcpSocket *parent = nullptr);

    bool connectReader(QString ip,qint16 port);
    void readTID();
    TagInfo parseTagInfo(QByteArray tid);
    quint8 crcCheck(unsigned char *data, quint32 len);
private slots:
    void onReadyRead();

signals:
    void recvTagInfo(TagInfo);

public slots:
};

#endif // UHFOUTREADER_H
