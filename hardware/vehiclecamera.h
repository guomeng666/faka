#ifndef VEHICLECAMERA_H
#define VEHICLECAMERA_H

#include <QTcpSocket>

class VehicleCamera : public QTcpSocket
{
    Q_OBJECT
public:
    explicit VehicleCamera(QObject *parent = nullptr);
    ~VehicleCamera();

public:
    void connectVehicleCamera(QString ip,quint16 port);//连接摄像机
    void setCameraWorkMode(bool enable, QString format, bool image, quint8 image_type);//设置摄像机工作模式
    void getivsresult();//获取最近一次识别结果
    void getVideo();
    void manualTrigger();//手动触发车牌识别仪
    void sendHeart();
    bool isConnected();

private slots:
    void onReadyRead();

signals:
    void readyCarData(const QByteArray&);

private:
    QByteArray m_recvBuf;
    qint32 m_recvLen;
};

#endif // VEHICLECAMERA_H
