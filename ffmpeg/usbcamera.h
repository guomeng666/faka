#ifndef USBCAMERA_H
#define USBCAMERA_H

/**
 * USB摄像机采集类 作者:feiyangqingyun(QQ:517216493) 2018-5-1
 * 1:可获取摄像机实时视频
 * 2:支持X86和嵌入式linux
 * 3:多线程显示图像,不卡主界面
 * 4:自动查找在线的USB摄像头
 * 5:自动重连USB摄像头
 * 6:可设置边框大小即偏移量和边框颜色
 * 7:可设置是否绘制OSD标签即标签文本或图片和标签位置
 * 8:可设置两种OSD位置和风格
 * 9:可设置是否显示实时人脸区域
 * 10:可设置查找当前图片内的最大相似人脸
 * 11:可暂停采集和继续采集
 * 12:可设置不同的分辨率
 */

#include <QtGui>
#if (QT_VERSION > QT_VERSION_CHECK(5,0,0))
#include <QtWidgets>
#endif

#ifdef __arm__
#else
#if (QT_VERSION > QT_VERSION_CHECK(5,5,0))
#include <QtMultimedia>
#include <QtMultimediaWidgets>
#endif
#endif

class USBCameraWidget;

//USB摄像机采集线程
class USBCameraThread : public QThread
{
    typedef struct ImgBuffer {
        quint8 *start;
        size_t length;
    } ImgBuffer;

    Q_OBJECT

public:
    explicit USBCameraThread(QObject *parent = 0);

protected:
    void run();
    bool eventFilter(QObject *watched, QEvent *event);

private:
    volatile bool stopped;              //停止线程标志位
    volatile bool isPause;              //暂停采集标志位
    volatile bool isSnap;               //是否需要截图标志位

    QMutex mutex;                       //锁对象
    QDateTime lastTime;                 //最后的消息时间
    QString cameraName;                 //摄像头设备文件名
    int cameraWidth;                    //USB摄像头宽度
    int cameraHeight;                   //USB摄像头高度
    int interval;                       //采集间隔
    int borderWidth;                    //边框宽度

    bool fillImage;                     //自动拉伸填充
    bool findFaceRect;                  //显示实时人脸区域
    bool findFast;                      //快速查找人脸
    bool findFaceOne;                   //查找最大相似人脸

#ifdef __arm__    
    int camera_fd;                      //摄像头指针
    bool camera_ok;                     //摄像头是否打开正常

    uchar *yuyv_buff;                   //yuyv图片数据
    uchar *yuyv_out;                    //yuyv图片输出数据
    uchar *rgb_buff;                    //rgb图片数据
    ImgBuffer *img_buffers;             //图片数据

    //读取文件
    QString readFile(const QString &fileName);
    //寻找摄像头
    void findCamera();
    //打开摄像头
    bool openCamera();
    //初始化摄像头
    bool initCamera();
    //关闭摄像头
    void closeCamera();

    //读取一帧图片数据
    int readFrame();
    //读取一张图片
    QImage readImage();

    //绘制人脸区域
    QImage drawFace(QImage image);
    //查找最大相似人脸
    void findFace(const QImage &image);

    //yuv422格式转yuv420
    void yuv422toyuv420(uchar *out, const uchar *in, uint width, uint height);

    //yuyv格式转rgb24
    bool yuyvtorgb24(uchar *yuv, uchar *rgb24);
#else
#if (QT_VERSION > QT_VERSION_CHECK(5,5,0))
    QCamera *camera;                    //摄像头对象
    QCameraViewfinder *viewfinder;      //视频画布
    QCameraImageCapture *imageCapture;  //抓图对象
#endif
#endif

public:
    //获取最后的活动时间
    QDateTime getLastTime();
    //获取摄像头设备文件名
    QString getCameraName();
    //获取USB摄像头宽度
    int getCameraWidth();
    //获取USB摄像头高度
    int getCameraHeight();

private slots:
    //呈现图片
    void displayImage(int , const QImage &image);
    //本地摄像头状态改变
    void stateChanged();

signals:
    //抓拍图片信号
    void snapImage(const QImage &image);
    //收到图片信号
    void receiveImage(const QImage &image);
    //主动请求重新加载USB摄像头
    void restart();

public slots:
    //设置摄像头设备文件名
    void setCameraName(const QString &cameraName);

    //设置USB摄像头宽度
    void setCameraWidth(int cameraWidth);

    //设置USB摄像头高度
    void setCameraHeight(int cameraHeight);

    //设置是否显示实时人脸区域
    void setFindFaceRect(bool findFaceRect, bool findFast = true);

    //设置是否检索最大相似度人脸
    void setFindFaceOne(bool findFaceOne);

    //设置采集间隔
    void setInterval(int interval);

    //设置边框宽度
    void setBorderWidth(int borderWidth);

    //设置是否实时绘制图片
    void setDrawImage(bool drawImage);

    //设置是否拉伸填充
    void setFillImage(bool fillImage);

    //初始化摄像头
    bool init();

    //打开摄像头
    void open();

    //暂停播放
    void pause();

    //继续播放
    void next();

    //停止采集服务
    void stop();

    //抓拍截图
    void snap();

};

//USB摄像机图像显示窗体类
class USBCameraWidget : public QWidget
{
    Q_OBJECT
    Q_ENUMS(OSDFormat)
    Q_ENUMS(OSDPosition)

    Q_PROPERTY(int cameraWidth READ getCameraWidth WRITE setCameraWidth)
    Q_PROPERTY(int cameraHeight READ getCameraHeight WRITE setCameraHeight)

    Q_PROPERTY(bool copyImage READ getCopyImage WRITE setCopyImage)
    Q_PROPERTY(bool checkLive READ getCheckLive WRITE setCheckLive)
    Q_PROPERTY(bool drawImage READ getDrawImage WRITE setDrawImage)
    Q_PROPERTY(bool fillImage READ getFillImage WRITE setFillImage)
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

    explicit USBCameraWidget(QWidget *parent = 0);
    ~USBCameraWidget();

protected:
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
    USBCameraThread *usbCamera;     //USB摄像机采集对象
    QTimer *timerCheck;             //定时器检查设备是否在线
    QImage image;                   //要显示的图片

    int cameraWidth;                //USB摄像头宽度
    int cameraHeight;               //USB摄像头高度

    bool copyImage;                 //是否拷贝图片
    bool checkLive;                 //检测是否活着
    bool drawImage;                 //是否绘制图片
    bool fillImage;                 //自动拉伸填充
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

public slots:
    QImage getImage()               const;
    QImage getImageOnly()           const;

public:
    QDateTime getLastTime()         const;

    QString getCameraName()         const;
    int getCameraWidth()            const;
    int getCameraHeight()           const;

    bool getCopyImage()             const;
    bool getCheckLive()             const;
    bool getDrawImage()             const;
    bool getFillImage()             const;
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
    void checkCamera();

signals:
    //抓拍图片信号
    void snapImage(const QImage &image);
    //收到图片信号
    void receiveImage(const QImage &image);

public slots:
    //设置是否显示实时人脸区域
    void setFindFaceRect(bool findFaceRect, bool findFast = true);
    //设置是否检索最大相似度人脸
    void setFindFaceOne(bool findFaceOne);
    //设置采集间隔
    void setInterval(int interval);

    //设置摄像头设备文件名
    void setCameraName(const QString &cameraName);
    //设置USB摄像头宽度
    void setCameraWidth(int cameraWidth);
    //设置USB摄像头高度
    void setCameraHeight(int cameraHeight);

    //设置是否拷贝图片
    void setCopyImage(bool copyImage);
    //设置是否检测活着
    void setCheckLive(bool checkLive);
    //设置是否实时绘制图片
    void setDrawImage(bool drawImage);
    //设置是否拉伸填充
    void setFillImage(bool fillImage);
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
    void setOSD1Visible(bool visible);
    //设置标签1文字字号
    void setOSD1FontSize(int fontSize);
    //设置标签1文本
    void setOSD1Text(const QString &text);
    //设置标签1文字颜色
    void setOSD1Color(const QColor &color);
    //设置标签1图片
    void setOSD1Image(const QImage &osdImage);
    //设置标签1格式
    void setOSD1Format(const OSDFormat &osdFormat);
    //设置标签1位置
    void setOSD1Position(const OSDPosition &osdPosition);

    //设置标签2是否可见
    void setOSD2Visible(bool visible);
    //设置标签2文字字号
    void setOSD2FontSize(int fontSize);
    //设置标签2文本
    void setOSD2Text(const QString &text);
    //设置标签2文字颜色
    void setOSD2Color(const QColor &color);
    //设置标签2图片
    void setOSD2Image(const QImage &osdImage);
    //设置标签2格式
    void setOSD2Format(const OSDFormat &osdFormat);
    //设置标签2位置
    void setOSD2Position(const OSDPosition &osdPosition);

    //打开设备
    bool open();
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

#endif // USBCAMERA_H
