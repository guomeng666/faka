#ifndef FFMPEG_H
#define FFMPEG_H

/**
 * ffmpeg视频播放类 作者:feiyangqingyun(QQ:517216493) 2018-5-1
 * 1:多线程实时播放rtsp视频流
 * 2:支持X86和嵌入式linux
 * 3:多线程显示图像,不卡主界面
 * 4:自动重连网络摄像头
 * 5:可设置边框大小即偏移量和边框颜色
 * 6:可设置是否绘制OSD标签即标签文本或图片和标签位置
 * 7:可设置两种OSD位置和风格
 * 8:可设置是否保存到文件以及文件名
 * 9:可设置间隔时间段保存文件到指定目录
 * 10:可播放本地视频文件,支持设置帧率
 * 11:支持h265视频流+rtmp等常见视频流
 * 12:可暂停播放和继续播放
 * 13:支持定时存储文件,包括音频和视频
 * 14:支持sdl播放音频
 * 15:支持外部拖曳文件+拖曳节点数据进行播放
 * 16:自定义顶部悬浮条,发送单击信号通知,可设置是否启用
 * 17:支持qsv dxva d3d 硬解码
 */

#include <QtGui>
#include <QtNetwork>
#if (QT_VERSION > QT_VERSION_CHECK(5,0,0))
#include <QtWidgets>
#endif

#include "ffmpeghead.h"

class FFmpegWidget;
class AVCodecParameters;

typedef struct DecodeContext {
    AVBufferRef *hw_device_ref;
} DecodeContext;

class FFmpegThread : public QThread
{
    Q_OBJECT
public:
    explicit FFmpegThread(QObject *parent = 0);
    static void initlib();

protected:
    void run();

private:
    volatile bool stopped;              //线程停止标志位
    volatile bool isPlay;               //播放视频标志位
    volatile bool isPause;              //暂停播放标志位
    volatile bool isRtsp;               //是否是视频流

    QMutex mutex;                       //锁对象
    QDateTime lastTime;                 //最后的消息时间
    int frameCount;                     //帧数统计
    int frameFinish;                    //一帧完成
    int videoWidth;                     //视频宽度
    int videoHeight;                    //视频高度
    int oldWidth;                       //上一次视频宽度
    int oldHeight;                      //上一次视频高度
    int videoStreamIndex;               //视频流索引
    int audioStreamIndex;               //音频流索引
    int videoFps;                       //视频流帧率

    int interval;                       //采集间隔
    int sleepTime;                      //休眠时间
    int checkTime;                      //检测超时时间
    bool checkConn;                     //检测视频流连接
    QString url;                        //视频流地址
    QString hardware;                   //硬解码解码器类型

    bool saveFile;                      //是否保存文件
    int saveInterval;                   //保存文件间隔,单位秒钟
    QString savePath;                   //保存文件夹
    QString fileName;                   //保存文件名称
    QTimer *timerSave;                  //定时器隔段时间存储文件
    QFile fileVideo;                    //保存视频文件
    QFile fileAudio;                    //保存音频文件

    uint8_t *buffer;                    //存储解码后图片buffer
    AVPacket *avPacket;                 //包对象
    AVFrame *avFrame;                   //帧对象
    AVFrame *avFrame2;                  //帧对象
    AVFrame *avFrame3;                  //帧对象
    AVFormatContext *avFormatContext;   //格式对象
    AVCodecContext *videoCodec;         //视频解码器
    AVCodecContext *audioCodec;         //音频解码器
    SwsContext *swsContext;             //处理图片数据对象
    DecodeContext decode;               //qsv解码所用结构体

    char *dtsData;
    const AVBitStreamFilter *filter;
    int av_bsf_filter(const AVBitStreamFilter *filter, AVPacket *pPacket, const AVCodecParameters *src);
    int decode_packet(AVCodecContext *avctx, AVPacket *packet);

private slots:
    //存储文件
    void startSave();
    void stopSave();
    void save();

public:
    //获取最后的活动时间
    QDateTime getLastTime();
    //获取url地址
    QString getUrl();
    //获取视频宽度
    int getVideoWidth();
    //获取视频高度
    int getVideoHeight();

signals:
    //播放成功
    void receivePlayOk();
    //播放失败
    void receivePlayError();
    //播放结束
    void receivePlayFinsh();

    //收到图片信号
    void receiveImage(const QImage &image);

    //启动和停止存储定时器
    void sig_startSave();
    void sig_stopSave();

public slots:
    //设置显示间隔
    void setInterval(int interval);
    //设置休眠时间
    void setSleepTime(int sleepTime);
    //设置检测连接超时
    void setCheckTime(int checkTime);
    //设置是否检测连接
    void setCheckConn(bool checkConn);
    //设置视频流地址
    void setUrl(const QString &url);
    //设置硬件解码器名称
    void setHardware(const QString &hardware);

    //设置是否保存文件
    void setSaveFile(bool saveFile);
    //设置保存间隔
    void setSaveInterval(int saveInterval);
    //设置保存路径
    void setSavePath(const QString &savePath);
    //设置保存文件名称
    void setFileName(const QString &fileName);

    //校验url
    bool checkUrl();

    //初始化视频对象
    bool init();

    //释放对象
    void free();

    //播放视频对象
    void play();

    //暂停播放
    void pause();

    //继续播放
    void next();

    //停止采集线程
    void stop();

};

//实时视频显示窗体类
#ifdef opengl
class FFmpegWidget : public QOpenGLWidget
#else
class FFmpegWidget : public QWidget
#endif
{
    Q_OBJECT
    Q_ENUMS(OSDFormat)
    Q_ENUMS(OSDPosition)

    Q_PROPERTY(bool copyImage READ getCopyImage WRITE setCopyImage)
    Q_PROPERTY(bool checkLive READ getCheckLive WRITE setCheckLive)
    Q_PROPERTY(bool drawImage READ getDrawImage WRITE setDrawImage)
    Q_PROPERTY(bool fillImage READ getFillImage WRITE setFillImage)
    Q_PROPERTY(bool flowEnable READ getFlowEnable WRITE setFlowEnable)

    Q_PROPERTY(int timeout READ getTimeout WRITE setTimeout)
    Q_PROPERTY(int borderWidth READ getBorderWidth WRITE setBorderWidth)
    Q_PROPERTY(QColor borderColor READ getBorderColor WRITE setBorderColor)
    Q_PROPERTY(QColor focusColor READ getFocusColor WRITE setFocusColor)
    Q_PROPERTY(QString bgText READ getBgText WRITE setBgText)
    Q_PROPERTY(QImage bgImage READ getBgImage WRITE setBgImage)

    Q_PROPERTY(bool osd1Visible READ getOSD1Visible WRITE setOSD1Visible)
    Q_PROPERTY(int osd1FontSize READ getOSD1FontSize WRITE setOSD1FontSize)
    Q_PROPERTY(QString osd1Text READ getOSD1Text WRITE setOSD1Text)
    Q_PROPERTY(QColor osd1Color READ getOSD1Color WRITE setOSD1Color)
    Q_PROPERTY(QImage osd1Image READ getOSD1Image WRITE setOSD1Image)
    Q_PROPERTY(OSDFormat osd1Format READ getOSD1Format WRITE setOSD1Format)
    Q_PROPERTY(OSDPosition osd1Position READ getOSD1Position WRITE setOSD1Position)

    Q_PROPERTY(bool osd2Visible READ getOSD2Visible WRITE setOSD2Visible)
    Q_PROPERTY(int osd2FontSize READ getOSD2FontSize WRITE setOSD2FontSize)
    Q_PROPERTY(QString osd2Text READ getOSD2Text WRITE setOSD2Text)
    Q_PROPERTY(QColor osd2Color READ getOSD2Color WRITE setOSD2Color)
    Q_PROPERTY(QImage osd2Image READ getOSD2Image WRITE setOSD2Image)
    Q_PROPERTY(OSDFormat osd2Format READ getOSD2Format WRITE setOSD2Format)
    Q_PROPERTY(OSDPosition osd2Position READ getOSD2Position WRITE setOSD2Position)

public:
    //标签格式
    enum OSDFormat {
        OSDFormat_Text = 0,             //文本
        OSDFormat_Date = 1,             //日期
        OSDFormat_Time = 2,             //时间
        OSDFormat_DateTime = 3,         //日期时间
        OSDFormat_Image = 4             //图片
    };

    //标签位置
    enum OSDPosition {
        OSDPosition_Left_Top = 0,       //左上角
        OSDPosition_Left_Bottom = 1,    //左下角
        OSDPosition_Right_Top = 2,      //右上角
        OSDPosition_Right_Bottom = 3    //右下角
    };

    explicit FFmpegWidget(QWidget *parent = 0);
    ~FFmpegWidget();

protected:
    void resizeEvent(QResizeEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void paintEvent(QPaintEvent *);
    void drawBorder(QPainter *painter);
    void drawBg(QPainter *painter);
    void drawImg(QPainter *painter, QImage img);
    void drawOSD(QPainter *painter,
                 int osdFontSize,
                 const QString &osdText,
                 const QColor &osdColor,
                 const QImage &osdImage,
                 const OSDFormat &osdFormat,
                 const OSDPosition &osdPosition);

private:
    FFmpegThread *ffmpeg;           //实时视频采集对象
    QTimer *timerCheck;             //定时器检查设备是否在线
    QImage image;                   //要显示的图片
    QWidget *flowPanel;             //悬浮条面板

    bool copyImage;                 //是否拷贝图片
    bool checkLive;                 //检测是否活着
    bool drawImage;                 //是否绘制图片
    bool fillImage;                 //自动拉伸填充
    bool flowEnable;                //是否显示悬浮条

    int timeout;                    //超时时间
    int borderWidth;                //边框宽度
    QColor borderColor;             //边框颜色
    QColor focusColor;              //有焦点边框颜色
    QString bgText;                 //默认无图像显示文字
    QImage bgImage;                 //默认无图像背景图片

    bool osd1Visible;               //显示标签1
    int osd1FontSize;               //标签1字号
    QString osd1Text;               //标签1文本
    QColor osd1Color;               //标签1颜色
    QImage osd1Image;               //标签1图片
    OSDFormat osd1Format;           //标签1文本格式
    OSDPosition osd1Position;       //标签1位置

    bool osd2Visible;               //显示标签2
    int osd2FontSize;               //标签2字号
    QString osd2Text;               //标签2文本
    QColor osd2Color;               //标签2颜色
    QImage osd2Image;               //标签2图片
    OSDFormat osd2Format;           //标签2文本格式
    OSDPosition osd2Position;       //标签2位置

public:
    QImage getImage()               const;
    QDateTime getLastTime()         const;
    QString getUrl()                const;

    bool getCopyImage()             const;
    bool getCheckLive()             const;
    bool getDrawImage()             const;
    bool getFillImage()             const;
    bool getFlowEnable()            const;

    int getTimeout()                const;
    int getBorderWidth()            const;
    QColor getBorderColor()         const;
    QColor getFocusColor()          const;
    QString getBgText()             const;
    QImage getBgImage()             const;

    bool getOSD1Visible()           const;
    int getOSD1FontSize()           const;
    QString getOSD1Text()           const;
    QColor getOSD1Color()           const;
    QImage getOSD1Image()           const;
    OSDFormat getOSD1Format()       const;
    OSDPosition getOSD1Position()   const;

    bool getOSD2Visible()           const;
    int getOSD2FontSize()           const;
    QString getOSD2Text()           const;
    QColor getOSD2Color()           const;
    QImage getOSD2Image()           const;
    OSDFormat getOSD2Format()       const;
    OSDPosition getOSD2Position()   const;

private slots:
    //接收图像并绘制
    void updateImage(const QImage &image);
    //校验设备
    void checkVideo();
    //处理按钮单击
    void btnClicked();

signals:
    //播放成功
    void receivePlayOk();
    //播放失败
    void receivePlayError();
    //播放结束
    void receivePlayFinsh();

    //收到图片信号
    void receiveImage(const QImage &image);

    //接收到拖曳文件
    void fileDrag(const QString &url);

    //工具栏单击
    void btnClicked(const QString &objName);

public slots:
    //设置显示间隔
    void setInterval(int interval);
    //设置休眠时间
    void setSleepTime(int sleepTime);
    //设置检测连接超时
    void setCheckTime(int checkTime);
    //设置是否检测连接
    void setCheckConn(bool checkConn);
    //设置视频流地址
    void setUrl(const QString &url);
    //设置硬件解码器名称
    void setHardware(const QString &hardware);

    //设置是否保存文件
    void setSaveFile(bool saveFile);
    //设置保存间隔
    void setSaveInterval(int saveInterval);
    //设置保存文件夹
    void setSavePath(const QString &savePath);
    //设置保存文件名称
    void setFileName(const QString &fileName);

    //设置是否拷贝图片
    void setCopyImage(bool copyImage);
    //设置是否检测活着
    void setCheckLive(bool checkLive);
    //设置是否实时绘制图片
    void setDrawImage(bool drawImage);
    //设置是否拉伸填充
    void setFillImage(bool fillImage);
    //设置是否启用悬浮条
    void setFlowEnable(bool flowEnable);

    //设置超时时间
    void setTimeout(int timeout);
    //设置边框宽度
    void setBorderWidth(int borderWidth);
    //设置边框颜色
    void setBorderColor(const QColor &borderColor);
    //设置有焦点边框颜色
    void setFocusColor(const QColor &focusColor);
    //设置无图像文字
    void setBgText(const QString &bgText);
    //设置无图像背景图
    void setBgImage(const QImage &bgImage);

    //设置标签1是否可见
    void setOSD1Visible(bool osdVisible);
    //设置标签1文字字号
    void setOSD1FontSize(int osdFontSize);
    //设置标签1文本
    void setOSD1Text(const QString &osdText);
    //设置标签1文字颜色
    void setOSD1Color(const QColor &osdColor);
    //设置标签1图片
    void setOSD1Image(const QImage &osdImage);
    //设置标签1格式
    void setOSD1Format(const OSDFormat &osdFormat);
    //设置标签1位置
    void setOSD1Position(const OSDPosition &osdPosition);

    //设置标签2是否可见
    void setOSD2Visible(bool osdVisible);
    //设置标签2文字字号
    void setOSD2FontSize(int osdFontSize);
    //设置标签2文本
    void setOSD2Text(const QString &osdText);
    //设置标签2文字颜色
    void setOSD2Color(const QColor &osdColor);
    //设置标签2图片
    void setOSD2Image(const QImage &osdImage);
    //设置标签2格式
    void setOSD2Format(const OSDFormat &osdFormat);
    //设置标签2位置
    void setOSD2Position(const OSDPosition &osdPosition);

    //打开设备
    void open();
    //暂停
    void pause();
    //继续
    void next();
    //关闭设备
    void close();
    //重新加载
    void restart();
    //清空
    void clear();

};

#endif // FFMPEG_H
