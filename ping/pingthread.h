#ifndef PINGTHREAD_H
#define PINGTHREAD_H

#include <QThread>
#include <QString>


class PingThread : public QThread
{
    Q_OBJECT
public:
    PingThread();
    void setPingList(QStringList list) { mPingList = list; }

protected:
    void run();
signals:
    void networkGood(const QString,int);
    void networkBad(const QString,int);
    void networkDisconnect(const QString,int);
private:
    QStringList mPingList;
};

#endif // PINGTHREAD_H
