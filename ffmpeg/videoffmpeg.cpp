#include "videoffmpeg.h"

#define TIMEMS  qPrintable(QTime::currentTime().toString("HH:mm:ss zzz"))
#define QDATE   qPrintable(QDate::currentDate().toString("yyyy-MM-dd"))
#define QTIME   qPrintable(QTime::currentTime().toString("HH-mm-ss"))

QScopedPointer<VideoFFmpeg> VideoFFmpeg::self;
VideoFFmpeg *VideoFFmpeg::Instance()
{
    if (self.isNull()) {
        static QMutex mutex;
        QMutexLocker locker(&mutex);
        if (self.isNull()) {
            self.reset(new VideoFFmpeg);
        }
    }

    return self.data();
}

VideoFFmpeg::VideoFFmpeg(QObject *parent) : QObject(parent)
{
    timeout = 10;
    videoCount = 16;
    saveVideo = false;
    savePath = qApp->applicationDirPath();

    timerOpen = new QTimer(this);
    connect(timerOpen, SIGNAL(timeout()), this, SLOT(openVideo()));
    timerOpen->setInterval(1 * 1000);

    timerCheck = new QTimer(this);
    connect(timerCheck, SIGNAL(timeout()), this, SLOT(checkVideo()));
    timerCheck->setInterval(5 * 1000);
}

VideoFFmpeg::~VideoFFmpeg()
{
    if (timerOpen->isActive()) {
        timerOpen->stop();
    }

    if (timerCheck->isActive()) {
        timerCheck->stop();
    }
}

void VideoFFmpeg::openVideo()
{
    if (index < videoCount) {
        //取出一个进行打开,跳过为空的立即下一个
        QString url = videoUrls.at(index);
        if (!url.isEmpty()) {
            open(index);
            index++;
        } else {
            index++;
            openVideo();
        }
    } else {
        //全部取完则关闭定时器
        timerOpen->stop();
    }
}

void VideoFFmpeg::checkVideo()
{
    QDateTime now = QDateTime::currentDateTime();
    for (int i = 0; i < videoCount; i++) {
        //只有url不为空的才需要处理重连
        QString url = videoUrls.at(i);
        if (url.isEmpty()) {
            continue;
        }

        //如果10秒内已经处理过重连则跳过当前这个,防止多个掉线一直处理第一个掉线的
        if (lastTimes.at(i).secsTo(now) < 10) {
            continue;
        }

        //计算超时时间
        QDateTime lastTime = videoWidgets.at(i)->getLastTime();
        int sec = lastTime.secsTo(now);
        if (sec >= timeout) {
            //重连该设备
            videoWidgets.at(i)->restart();
            //每次只重连一个,并记住最后重连时间
            lastTimes[i] = now;
            break;
        }
    }
}

void VideoFFmpeg::newDir(const QString &dirName)
{
    //如果路径中包含斜杠字符则说明是绝对路径
    //linux系统路径字符带有 /  windows系统 路径字符带有 :/
    QString strDir = dirName;
    if (!strDir.startsWith("/") && !strDir.contains(":/")) {
        strDir = QString("%1/%2").arg(qApp->applicationDirPath()).arg(strDir);
    }

    QDir dir(strDir);
    if (!dir.exists()) {
        dir.mkpath(strDir);
    }
}

void VideoFFmpeg::setVideoCount(int videoCount)
{
    this->videoCount = videoCount;
}

void VideoFFmpeg::setSaveVideo(bool saveVideo)
{
    this->saveVideo = saveVideo;
}

void VideoFFmpeg::setSavePath(const QString &savePath)
{
    this->savePath = savePath;
}

void VideoFFmpeg::setUrls(const QList<QString> &videoUrls)
{
    this->videoUrls = videoUrls;
}

void VideoFFmpeg::setWidgets(QList<FFmpegWidget *> videoWidgets)
{
    this->videoWidgets = videoWidgets;
}

void VideoFFmpeg::start()
{
    if (videoWidgets.count() != videoCount) {
        return;
    }

    lastTimes.clear();
    for (int i = 0; i < videoCount; i++) {
        lastTimes.append(QDateTime::currentDateTime());
        QString url = videoUrls.at(i);

        if (!url.isEmpty()) {
            FFmpegWidget *w = videoWidgets.at(i);
            //设置文件url地址
            w->setUrl(url);
            //设置是否存储文件
            w->setSaveFile(saveVideo);
            if (saveVideo) {
                QString path = QString("%1/%2").arg(savePath).arg(QDATE);
                newDir(path);
                QString fileName = QString("%1/%2-%3-Ch%4.mp4").arg(path).arg(QDATE).arg(QTIME).arg(i + 1);
                w->setFileName(fileName);
            }

            //设置不自动重连,默认会自动重连,这个交给本类处理
            //因为如果是断电引起的掉线,同一时间可能都在处理重连,会导致可能的意外崩溃
            w->setCheckLive(false);
            //open(i);
        }
    }

    //启动定时器挨个排队打开
    index = 0;
    timerOpen->start();
    //启动定时器排队处理重连
    //timerCheck->start();
}

void VideoFFmpeg::stop()
{
    if (videoWidgets.count() != videoCount) {
        return;
    }

    timerOpen->stop();
    timerCheck->stop();
    for (int i = 0; i < videoCount; i++) {
        close(i);
    }
}

void VideoFFmpeg::open(int index)
{
    videoWidgets.at(index)->open();
}

void VideoFFmpeg::close(int index)
{
    videoWidgets.at(index)->close();
}

void VideoFFmpeg::snap(int index, const QString &fileName)
{
    QImage img = videoWidgets.at(index)->getImage();
    if (!img.isNull()) {
        img.save(fileName, "jpg");
    }
}
