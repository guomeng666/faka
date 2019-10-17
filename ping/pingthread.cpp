#include "pingthread.h"
#include "Pinger.h"
#include <QDebug>
#include <iostream>

PingThread::PingThread()
{
}

void PingThread::run()
{
    Pinger p;
    int n = 0;
    while(1)
    {
        if(mPingList.count() == 0)
            msleep(1000);
        foreach (QString ip, mPingList)
        {
            n = p.ping(ip.toLocal8Bit().data(),10,200,200);
            if(n == 10)
            {
                emit networkGood(ip, n);
            }
            else if (n > 5 && n < 10 )
            {
                emit networkBad(ip, n);
            }
            else if(n < 5)
            {
                emit networkDisconnect(ip, n);
            }
            msleep(1000);
            //qDebug()<<QString::fromStdString(p.getTips());
        }  
    }

}
