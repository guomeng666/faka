#ifndef VIDEOFFMPEG_H
#define VIDEOFFMPEG_H

/**
 * ffmpeg视频监控类 作者:feiyangqingyun(QQ:517216493) 2018-5-2
 * 1:多线程多路实时播放rtsp视频流
 * 2:自动排队处理掉线重连
 * 3:可设置是否转储文件及存储文件名称
 * 4:可设置画面通道数量
 * 5:可对单个画面截图
 * 6:支持设置url地址集合和播放载体集合
 * 7:url地址集合来源可以是配置文件或者数据库等多种方式
 */

#include <QtGui>
#include <QtNetwork>
#if (QT_VERSION > QT_VERSION_CHECK(5,0,0))
#include <QtWidgets>
#endif

#include "ffmpeg.h"

class VideoFFmpeg : public QObject
{
    Q_OBJECT
public:
    static VideoFFmpeg *Instance();
    explicit VideoFFmpeg(QObject *parent = 0);
    ~VideoFFmpeg();

private:
    static QScopedPointer<VideoFFmpeg> self;

    //视频流地址链表
    QList<QString> videoUrls;
    //视频播放窗体链表
    QList<FFmpegWidget *> videoWidgets;
    //最后的重连时间
    QList<QDateTime> lastTimes;

    //超时时间
    int timeout;
    //视频数量
    int videoCount;
    //是否存储视频文件
    bool saveVideo;
    //存储路径
    QString savePath;

    //定时器排队打开视频
    int index;
    QTimer *timerOpen;
    //定时器重连
    QTimer *timerCheck;

public slots:
    //排队打开视频
    void openVideo();
    //处理重连
    void checkVideo();

    //新建目录
    void newDir(const QString &dirName);
    //设置视频通道数
    void setVideoCount(int videoCount);
    //设置是否存储视频
    void setSaveVideo(bool saveVideo);
    //设置存储文件夹
    void setSavePath(const QString &savePath);

    //设置地址集合
    void setUrls(const QList<QString> &videoUrls);
    //设置对象集合
    void setWidgets(QList<FFmpegWidget *> videoWidgets);

    //启动
    void start();
    //停止
    void stop();

    //打开
    void open(int index);
    //关闭
    void close(int index);

    //快照
    void snap(int index, const QString &fileName);

};

#endif // VIDEOFFMPEG_H
