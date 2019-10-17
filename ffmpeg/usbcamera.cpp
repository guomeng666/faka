#include "usbcamera.h"

#ifdef __arm__
#include "fcntl.h"
#include "errno.h"
#include "malloc.h"
#include "unistd.h"
#include "sys/stat.h"
#include "sys/mman.h"
#include "sys/ioctl.h"
#include "linux/videodev2.h"
#include "ffmpeghead.h"
#endif

#ifdef facesdk          //如果启用了人脸识别
#include "facesdk.h"    //引入人脸识别头文件
#include "facelocal.h"  //引入人脸识别处理头文件
using namespace CRface; //引入命名空间
#endif

#define TIMEMS qPrintable(QTime::currentTime().toString("HH:mm:ss zzz"))

USBCameraThread::USBCameraThread(QObject *parent) : QThread(parent)
{
    setObjectName("USBCameraThread");
    stopped = false;
    isPause = false;
    isSnap = false;

    lastTime = QDateTime::currentDateTime();
    cameraName = "auto";
    cameraWidth = 640;
    cameraHeight = 480;
    interval = 10;
    borderWidth = 5;

    fillImage = true;
    findFaceRect = false;
    findFast = true;
    findFaceOne = false;

#ifdef __arm__
#else
#if (QT_VERSION > QT_VERSION_CHECK(5,5,0))
    camera = NULL;
    viewfinder = NULL;
    imageCapture = NULL;
#endif
#endif
}

void USBCameraThread::run()
{
    while(!stopped) {
#ifdef __arm__
        if (camera_ok) {
            if (isPause) {
                //这里需要假设正常,暂停期间继续更新时间
                lastTime = QDateTime::currentDateTime();
                msleep(1);
                continue;
            }

            QImage image = readImage();
            if (!image.isNull()) {
                if (isSnap) {
                    emit snapImage(image);
                    isSnap = false;
                }

                if (findFaceOne) {
                    findFace(image);
                }

                if (findFaceRect) {
                    image = drawFace(image);
                }

                lastTime = QDateTime::currentDateTime();
                emit receiveImage(image);
            }
        }
#else
#if (QT_VERSION > QT_VERSION_CHECK(5,5,0))
        if (camera->isAvailable()) {
            //这里需要假设正常,暂停期间继续更新时间
            lastTime = QDateTime::currentDateTime();
        }
#endif
#endif
        msleep(interval);
    }

    stopped = false;
    isPause = false;
    isSnap = false;
}

bool USBCameraThread::eventFilter(QObject *watched, QEvent *event)
{
#ifdef __arm__
#else
#if (QT_VERSION > QT_VERSION_CHECK(5,5,0))
    //自动根据窗体大小变化而变化
    if (watched == viewfinder) {
        if (event->type() == QEvent::Paint) {
            QWidget *frm = (QWidget *)parent();
            QRect rect = frm->rect();
            int offset = borderWidth * 1 + 0;
            viewfinder->setGeometry(offset / 2, offset / 2, rect.width() - offset, rect.height() - offset);
        }
    }
#endif
#endif
    return QObject::eventFilter(watched, event);
}

QDateTime USBCameraThread::getLastTime()
{
    return this->lastTime;
}

QString USBCameraThread::getCameraName()
{
    return this->cameraName;
}

int USBCameraThread::getCameraWidth()
{
    return this->cameraWidth;
}

int USBCameraThread::getCameraHeight()
{
    return this->cameraHeight;
}

#ifdef __arm__
QString USBCameraThread::readFile(const QString &fileName)
{
    QString result;
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        result = QString(file.readLine()).trimmed();
        file.close();
    }

    return result;
}

void USBCameraThread::findCamera()
{
    if (cameraName != "auto") {
        return;
    }

    //执行命令查找出USB摄像头路径  将临时文件存储到当前应用程序目录下
    QString strPath = QString("%1/path.txt").arg(qApp->applicationDirPath());
    QString strDevice = QString("%1/device.txt").arg(qApp->applicationDirPath());
    QString cmd = QString("rm -rf %1 %2").arg(strPath).arg(strDevice);
    system(cmd.toLatin1().data());

#ifdef arma7
    cmd = QString("find /sys/devices/platform/ -name video4linux > %1").arg(strPath);
#else
    cmd = QString("find /sys/devices/platform/fsl-ehci.1 -name video4linux > %1").arg(strPath);
#endif
    system(cmd.toLatin1().data());

    //执行命令查找出USB摄像头设备文件名称
    QString cameraPath = readFile(strPath);
    if (cameraPath.isEmpty()) {
        qDebug() << TIMEMS << "not find camera path";
    } else {
        QString cmd = QString("ls %1 > %2").arg(cameraPath).arg(strDevice);
        system(cmd.toLatin1().data());
        cameraName = QString("/dev/%1").arg(readFile(strDevice));
        if (cameraName.length() > 5) {
            qDebug() << TIMEMS << "cameraName = " << cameraName;
        }
    }
}

bool USBCameraThread::openCamera()
{
    camera_fd = -1;
    if (cameraName.length() > 5) {
        camera_fd = ::open(cameraName.toLatin1().data(), O_RDWR | O_NONBLOCK, 0);
        if(camera_fd == -1) {
            qDebug() << TIMEMS << "open camera error";
        } else {
            qDebug() << TIMEMS << "open camera ok";
        }
    }

    return camera_fd > 0;
}

bool USBCameraThread::initCamera()
{
    //查询设备属性
    struct v4l2_capability cap;
    if(ioctl(camera_fd, VIDIOC_QUERYCAP, &cap) < 0) {
        qDebug() << TIMEMS << "error in VIDIOC_QUERYCAP";
        close(camera_fd);
        return false;
    }

    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        qDebug() << TIMEMS << "it is not a video capture device";
        close(camera_fd);
        return false;
    }

    if(!(cap.capabilities & V4L2_CAP_STREAMING)) {
        qDebug() << TIMEMS << "it can not streaming";
        close(camera_fd);
        return false;
    }

    if(cap.capabilities == 0x4000001) {
        qDebug() << TIMEMS << "capabilities:" << "V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING";
    }

    //设置视频输入源
    int input = 0;
    if(ioctl(camera_fd, VIDIOC_S_INPUT, &input) < 0) {
        qDebug() << TIMEMS << "error in VIDIOC_S_INPUT";
        close(camera_fd);
        return false;
    }

    //设置图片格式和分辨率
    struct v4l2_format fmt;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV ;//V4L2_PIX_FMT_YUV420  V4L2_PIX_FMT_YUYV
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    fmt.fmt.pix.width = cameraWidth;
    fmt.fmt.pix.height = cameraHeight;

    if(ioctl(camera_fd, VIDIOC_S_FMT, &fmt) < 0) {
        close(camera_fd);
        return false;
    }

    //查看图片格式和分辨率,判断是否设置成功
    if(ioctl(camera_fd, VIDIOC_G_FMT, &fmt) < 0) {
        qDebug() << TIMEMS << "error in VIDIOC_G_FMT";
        close(camera_fd);
        return false;
    }

    //重新打印下宽高看下是否真正设置成功
    qDebug() << TIMEMS << "cameraWidth =" << cameraWidth << "cameraHeight =" << cameraHeight << "  width =" << fmt.fmt.pix.width << "height =" << fmt.fmt.pix.height;
    qDebug() << TIMEMS << "pixelformat =" << QString("%1%2%3%4").arg(QChar(fmt.fmt.pix.pixelformat & 0xFF)).arg(QChar((fmt.fmt.pix.pixelformat >> 8) & 0xFF)).arg(QChar((fmt.fmt.pix.pixelformat >> 16) & 0xFF)).arg(QChar((fmt.fmt.pix.pixelformat >> 24) & 0xFF));

    //重新设置宽高为真实的宽高
    cameraWidth = fmt.fmt.pix.width;
    cameraHeight = fmt.fmt.pix.height;

    //设置帧格式
    struct v4l2_streamparm parm;
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = 25;
    parm.parm.capture.capturemode = 0;

    if(ioctl(camera_fd, VIDIOC_S_PARM, &parm) < 0) {
        qDebug() << TIMEMS << "error in VIDIOC_S_PARM";
        close(camera_fd);
        return false;
    }

    if(ioctl(camera_fd, VIDIOC_G_PARM, &parm) < 0) {
        qDebug() << TIMEMS << "error in VIDIOC_G_PARM";
        close(camera_fd);
        return false;
    }

    //申请和管理缓冲区
    struct v4l2_requestbuffers reqbuf;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    reqbuf.count = 1;

    if(ioctl(camera_fd, VIDIOC_REQBUFS, &reqbuf) < 0) {
        qDebug() << TIMEMS << "error in VIDIOC_REQBUFS";
        close(camera_fd);
        return false;
    }

    img_buffers = (ImgBuffer *)calloc(1, sizeof(ImgBuffer));
    if(img_buffers == NULL) {
        qDebug() << TIMEMS << "error in calloc";
        close(camera_fd);
        return false;
    }

    struct v4l2_buffer buffer;
    for(int numBufs = 0; numBufs < 1; numBufs++) {
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = numBufs;

        if(ioctl(camera_fd, VIDIOC_QUERYBUF, &buffer) < 0) {
            qDebug() << TIMEMS << "error in VIDIOC_QUERYBUF";
            free(img_buffers);
            close(camera_fd);
            return false;
        }

        img_buffers[numBufs].length = buffer.length;
        img_buffers[numBufs].start = (quint8 *)mmap (NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, camera_fd, buffer.m.offset);
        if(MAP_FAILED == img_buffers[numBufs].start) {
            qDebug() << TIMEMS << "error in mmap";
            free(img_buffers);
            close(camera_fd);
            return false;
        }

        //把缓冲帧放入队列
        if(ioctl(camera_fd, VIDIOC_QBUF, &buffer) < 0) {
            qDebug() << TIMEMS << "error in VIDIOC_QBUF";
            for(int i = 0; i <= numBufs; i++) {
                munmap(img_buffers[i].start, img_buffers[i].length);
            }

            free(img_buffers);
            close(camera_fd);
            return false;
        }
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(camera_fd, VIDIOC_STREAMON, &type) < 0) {
        qDebug() << TIMEMS << "error in VIDIOC_STREAMON";
        for(int i = 0; i < 1; i++) {
            munmap(img_buffers[i].start, img_buffers[i].length);
        }

        free(img_buffers);
        close(camera_fd);
        return false;
    }

    qDebug() << TIMEMS << "init camera ok";
    return true;
}

void USBCameraThread::closeCamera()
{
    if(camera_ok && img_buffers != NULL) {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if(ioctl(camera_fd, VIDIOC_STREAMOFF, &type) < 0) {
            qDebug() << TIMEMS << "error in VIDIOC_STREAMOFF";
        }

        for(int i = 0; i < 1; i++) {
            munmap((img_buffers)[i].start, (img_buffers)[i].length);
        }

        close(camera_fd);
        free(img_buffers);
        img_buffers = NULL;
        qDebug() << TIMEMS << "close camera ok";
    }

    camera_fd = -1;
    camera_ok = false;
}

int USBCameraThread::readFrame()
{
    //等待摄像头采集到一桢数据
    for(;;) {
        fd_set fds;
        struct timeval tv;
        FD_ZERO(&fds);
        FD_SET(camera_fd, &fds);
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        int r = select(camera_fd + 1, &fds, NULL, NULL, &tv);
        if(-1 == r) {
            if(EINTR == errno) {
                continue;
            }
            return -1;
        } else if(0 == r) {
            return -1;
        } else { //采集到一张图片
            break;
        }
    }

    //从缓冲区取出一个缓冲帧
    struct v4l2_buffer buffer;
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    if(ioctl(camera_fd, VIDIOC_DQBUF, &buffer) < 0) {
        qDebug() << TIMEMS << "error in VIDIOC_DQBUF";
        return -1;
    }

    memcpy(yuyv_buff, (uchar *)img_buffers[buffer.index].start, img_buffers[buffer.index].length);

    //将取出的缓冲帧放回缓冲区
    if(ioctl(camera_fd, VIDIOC_QBUF, &buffer) < 0) {
        qDebug() << TIMEMS << "error in VIDIOC_QBUF";
        return -1;
    }

    return buffer.index;
}

QImage USBCameraThread::readImage()
{
    //错误次数计数
    static int errorCount = 0;
    QImage image;
    if(camera_ok) {
        if(readFrame() >= 0) {
            errorCount = 0;
            if(yuyvtorgb24(yuyv_buff, rgb_buff)) {
                image = QImage((uchar *)rgb_buff, cameraWidth, cameraHeight, QImage::Format_RGB888);
            }
        } else {
            qDebug() << TIMEMS << "error in ReadFrame";
            errorCount++;

            //错误次数连着超过5次则重新打开USB摄像头
            if (errorCount >= 5) {
                qDebug() << TIMEMS << "errorCount >=5  camera restart";
                errorCount = 0;
                emit restart();
            }
        }
    }

    return image;
}

QImage USBCameraThread::drawFace(QImage image)
{
#ifdef facesdk
    yuv422toyuv420(yuyv_out, yuyv_buff, cameraWidth, cameraHeight);
    int facebox[5 * 32];
    int result;

    if (mutex.tryLock()) {
        if (findFast) {
            result = FaceDetect_Fast_ColorReco(yuyv_out, cameraWidth, cameraHeight, facebox, true);
        } else {
            result = FaceDetect_Normal_ColorReco(yuyv_out, cameraWidth, cameraHeight, facebox, true, cameraWidth / 4);
        }
        mutex.unlock();
    }

    if (result == 1) {
        QPainter painter;
        painter.begin(&image);
        painter.setPen(QPen(Qt::red, 5));

        int x = facebox[1];
        int y = facebox[2];
        int width = facebox[3];
        int height = facebox[4];
#if 1
        painter.drawRect(x, y, width, height);
#else
        //绘制四个角
        int offset = 10;
        painter.drawLine(x, y, x + offset, y);
        painter.drawLine(x, y, x, y + offset);
        painter.drawLine(x + width - offset, y, x + width, y);
        painter.drawLine(x + width, y, x + width, y + offset);
        painter.drawLine(x, y + height - offset, x, y + height);
        painter.drawLine(x, y + height, x + offset, y + height);
        painter.drawLine(x + width - offset, y + height, x + width, y + height);
        painter.drawLine(x + width, y + height - offset, x + width, y + height);
#endif
        painter.end();
    }
#endif

    return image;
}

void USBCameraThread::findFace(const QImage &image)
{
#ifdef facesdk
    FaceLocal::Instance()->setOneImg("oneImg", image);
#endif
}

void USBCameraThread::yuv422toyuv420(unsigned char *out, const unsigned char *in, unsigned int width, unsigned int height)
{
    unsigned char *y = out;
    unsigned int i, j;
    unsigned int y_index = 0;
    unsigned long yuv422_length = 2 * width * height;

    for(i = 0; i < yuv422_length; i += 2) {
        *(y + y_index) = *(in + i);
        y_index++;
    }
}

bool USBCameraThread::yuyvtorgb24(uchar *yuv, uchar *rgb24)
{
    if (yuv == NULL || rgb24 == NULL) {
        return false;
    }

    AVPicture pFrameYUV, pFrameRGB;
    avpicture_fill(&pFrameYUV, yuv, AV_PIX_FMT_YUYV422, cameraWidth, cameraHeight);
    avpicture_fill(&pFrameRGB, rgb24, AV_PIX_FMT_BGR24, cameraWidth, cameraHeight);

    //U,V互换
    uint8_t *ptmp = pFrameYUV.data[1];
    pFrameYUV.data[1] = pFrameYUV.data[2];
    pFrameYUV.data[2] = ptmp;

    struct SwsContext *imgCtx = sws_getContext(cameraWidth, cameraHeight, AV_PIX_FMT_YUYV422, cameraWidth, cameraHeight, AV_PIX_FMT_RGB24, SWS_BILINEAR, 0, 0, 0);

    if(imgCtx != NULL) {
        sws_scale(imgCtx, pFrameYUV.data, pFrameYUV.linesize, 0, cameraHeight, pFrameRGB.data, pFrameRGB.linesize);
        if(imgCtx) {
            sws_freeContext(imgCtx);
            imgCtx = NULL;
        }
        return true;
    } else {
        sws_freeContext(imgCtx);
        imgCtx = NULL;
        return false;
    }
}
#endif

void USBCameraThread::displayImage(int, const QImage &image)
{
    emit snapImage(image);
}

void USBCameraThread::stateChanged()
{
#ifdef __arm__
#else
#if (QT_VERSION > QT_VERSION_CHECK(5,5,0))
    if (camera->state() == QCamera::ActiveState) {
        //输出当前设备支持的分辨率
        QList<QSize> sizes = camera->supportedViewfinderResolutions();
        foreach (QSize size, sizes) {
            qDebug() << "Resolutions size = " << size;
        }

        //重新设置分辨率
        QCameraViewfinderSettings set;
        set.setResolution(cameraWidth, cameraHeight);
        camera->setViewfinderSettings(set);
    }
#endif
#endif
}

void USBCameraThread::setCameraName(const QString &cameraName)
{
    this->cameraName = cameraName;
}

void USBCameraThread::setCameraWidth(int cameraWidth)
{
    this->cameraWidth = cameraWidth;
}

void USBCameraThread::setCameraHeight(int cameraHeight)
{
    this->cameraHeight = cameraHeight;
}

void USBCameraThread::setFindFaceRect(bool findFaceRect, bool findFast)
{
    this->findFaceRect = findFaceRect;
    this->findFast = findFast;
}

void USBCameraThread::setFindFaceOne(bool findFaceOne)
{
    this->findFaceOne = findFaceOne;
}

void USBCameraThread::setInterval(int interval)
{
    if (interval > 0) {
        this->interval = interval;
    }
}

void USBCameraThread::setBorderWidth(int borderWidth)
{
    this->borderWidth = borderWidth;
}

void USBCameraThread::setDrawImage(bool drawImage)
{
#ifdef __arm__
#else
#if (QT_VERSION > QT_VERSION_CHECK(5,5,0))
    if (viewfinder != NULL) {
        viewfinder->setVisible(drawImage);
    }
#endif
#endif
}

void USBCameraThread::setFillImage(bool fillImage)
{
    this->fillImage = fillImage;
}

bool USBCameraThread::init()
{
#ifdef __arm__
    camera_fd = -1;
    camera_ok = false;
    yuyv_buff = (uchar *)malloc(cameraWidth * cameraHeight * 16 / 8);
    yuyv_out = (uchar *)malloc(cameraWidth * cameraHeight * 16 / 8);
    rgb_buff = (uchar *)malloc(cameraWidth * cameraHeight * 24 / 8);

    findCamera();
    if(openCamera()) {
        if(initCamera()) {
            camera_ok = true;
        }
    }

    return camera_ok;
#else
#if (QT_VERSION > QT_VERSION_CHECK(5,5,0))
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    if (cameras.count() == 0) {
        qDebug() << TIMEMS << "not find USBCamera";
        return false;
    } else {
        QWidget *frm = (QWidget *)parent();
        camera = new QCamera(frm);
        connect(camera, SIGNAL(stateChanged(QCamera::State)), this, SLOT(stateChanged()));

        viewfinder = new QCameraViewfinder(frm);
        viewfinder->installEventFilter(this);
        camera->setViewfinder(viewfinder);

        QRect rect = frm->rect();
        viewfinder->setAspectRatioMode(fillImage ? Qt::IgnoreAspectRatio : Qt::KeepAspectRatio);
        int offset = borderWidth * 1 + 0;
        viewfinder->setGeometry(offset / 2, offset / 2, rect.width() - offset, rect.height() - offset);
        viewfinder->setVisible(true);

        imageCapture = new QCameraImageCapture(camera);
        connect(imageCapture, SIGNAL(imageCaptured(int, QImage)), this, SLOT(displayImage(int, QImage)));
        return true;
    }
#endif
#endif
}

void USBCameraThread::open()
{
    stopped = false;
    isPause = false;
    start();
#ifdef __arm__
#else
#if (QT_VERSION > QT_VERSION_CHECK(5,5,0))
    if (camera != NULL) {
        camera->start();
    }
#endif
#endif
}

void USBCameraThread::pause()
{
    isPause = true;
}

void USBCameraThread::next()
{
    isPause = false;
}

void USBCameraThread::stop()
{
    stopped = true;

#ifdef __arm__
    closeCamera();
#else
#if (QT_VERSION > QT_VERSION_CHECK(5,5,0))
    if (camera != NULL) {
        viewfinder->setVisible(false);
        camera->stop();
    }
#endif
#endif
}

void USBCameraThread::snap()
{
#ifdef __arm__
    isSnap = true;
#else
#if (QT_VERSION > QT_VERSION_CHECK(5,5,0))
    if (imageCapture != NULL) {
        imageCapture->capture(qApp->applicationDirPath() + "/snap.jpg");
    }
#endif
#endif
}

//USB摄像机图像显示窗体类
USBCameraWidget::USBCameraWidget(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    usbCamera = new USBCameraThread(this);
    connect(usbCamera, SIGNAL(snapImage(QImage)), this, SIGNAL(snapImage(QImage)));
    connect(usbCamera, SIGNAL(receiveImage(QImage)), this, SIGNAL(receiveImage(QImage)));
    connect(usbCamera, SIGNAL(receiveImage(QImage)), this, SLOT(updateImage(QImage)));
    connect(usbCamera, SIGNAL(restart()), this, SLOT(restart()));

    timerCheck = new QTimer(this);
    timerCheck->setInterval(5 * 1000);
    connect(timerCheck, SIGNAL(timeout()), this, SLOT(checkCamera()));

    image = QImage();

    cameraWidth = 640;
    cameraHeight = 480;

    copyImage = false;
    checkLive = true;
    drawImage = true;
    fillImage = true;
    timeout = 10;
    borderWidth = 5;
    borderColor = "#000000";
    focusColor = "#22A3A9";
    bgText = "实时图像";
    bgImage = QImage();

    osd1Visible = false;
    osd1FontSize = 12;
    osd1Text = "时间";
    osd1Color = "#FF0000";
    osd1Image = QImage();
    osd1Format = OSDFormat_DateTime;
    osd1Position = OSDPosition_Right_Top;

    osd2Visible = false;
    osd2FontSize = 12;
    osd2Text = "通道名称";
    osd2Color = "#FF0000";
    osd2Image = QImage();
    osd2Format = OSDFormat_Text;
    osd2Position = OSDPosition_Left_Bottom;
}

USBCameraWidget::~USBCameraWidget()
{
    this->close();
}

void USBCameraWidget::paintEvent(QPaintEvent *e)
{
    //如果不需要绘制
    if (!drawImage) {
        return;
    }

    //qDebug() << TIMEMS << "paintEvent" << objectName();
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    //绘制边框
    drawBorder(&painter);
    //绘制背景
    drawBg(&painter);

    QImage img = image;
    if (!img.isNull()) {
        //绘制背景图片
        drawImg(&painter, img);

        //绘制标签1
        if (osd1Visible) {
            drawOSD(&painter, osd1FontSize, osd1Text, osd1Color, osd1Image, osd1Format, osd1Position);
        }

        //绘制标签2
        if (osd2Visible) {
            drawOSD(&painter, osd2FontSize, osd2Text, osd2Color, osd2Image, osd2Format, osd2Position);
        }
    }
}

void USBCameraWidget::drawBorder(QPainter *painter)
{
    painter->save();
    QPen pen;
    pen.setWidth(borderWidth);
    pen.setColor(hasFocus() ? focusColor : borderColor);
    painter->setPen(pen);
    painter->drawRect(rect());
    painter->restore();
}

void USBCameraWidget::drawBg(QPainter *painter)
{
    painter->save();

    //背景图片为空则绘制文字,否则绘制背景图片
    if (bgImage.isNull()) {
        painter->setPen(palette().foreground().color());
        painter->drawText(rect(), Qt::AlignCenter, bgText);
    } else {
        //居中绘制
        int pixX = rect().center().x() - bgImage.width() / 2;
        int pixY = rect().center().y() - bgImage.height() / 2;
        QPoint point(pixX, pixY);
        painter->drawImage(point, bgImage);
    }

    painter->restore();
}

void USBCameraWidget::drawImg(QPainter *painter, QImage img)
{
    painter->save();

    int offset = borderWidth * 1 + 0;
    if (fillImage) {
        QRect rect(offset / 2, offset / 2, width() - offset, height() - offset);
        painter->drawImage(rect, img);
    } else {
        //按照比例自动居中绘制
        img = img.scaled(width() - offset, height() - offset, Qt::KeepAspectRatio);
        int pixX = rect().center().x() - img.width() / 2;
        int pixY = rect().center().y() - img.height() / 2;
        QPoint point(pixX, pixY);
        painter->drawImage(point, img);
    }

    painter->restore();
}

void USBCameraWidget::drawOSD(QPainter *painter,
                              int osdFontSize,
                              const QString &osdText,
                              const QColor &osdColor,
                              const QImage &osdImage,
                              const USBCameraWidget::OSDFormat &osdFormat,
                              const USBCameraWidget::OSDPosition &osdPosition)
{
    painter->save();

    //标签位置尽量偏移多一点避免遮挡
    QRect osdRect(rect().x() + (borderWidth * 2), rect().y() + (borderWidth * 2), width() - (borderWidth * 5), height() - (borderWidth * 5));
    int flag = Qt::AlignLeft | Qt::AlignTop;
    QPoint point = QPoint(osdRect.x(), osdRect.y());

    if (osdPosition == OSDPosition_Left_Top) {
        flag = Qt::AlignLeft | Qt::AlignTop;
        point = QPoint(osdRect.x(), osdRect.y());
    } else if (osdPosition == OSDPosition_Left_Bottom) {
        flag = Qt::AlignLeft | Qt::AlignBottom;
        point = QPoint(osdRect.x(), osdRect.height() - osdImage.height());
    } else if (osdPosition == OSDPosition_Right_Top) {
        flag = Qt::AlignRight | Qt::AlignTop;
        point = QPoint(osdRect.width() - osdImage.width(), osdRect.y());
    } else if (osdPosition == OSDPosition_Right_Bottom) {
        flag = Qt::AlignRight | Qt::AlignBottom;
        point = QPoint(osdRect.width() - osdImage.width(), osdRect.height() - osdImage.height());
    }

    if (osdFormat == OSDFormat_Image) {
        painter->drawImage(point, osdImage);
    } else {
        QDateTime now = QDateTime::currentDateTime();
        QString text = osdText;
        if (osdFormat == OSDFormat_Date) {
            text = now.toString("yyyy-MM-dd");
        } else if (osdFormat == OSDFormat_Time) {
            text = now.toString("HH:mm:ss");
        } else if (osdFormat == OSDFormat_DateTime) {
            text = now.toString("yyyy-MM-dd HH:mm:ss");
        }

        //设置颜色及字号
        QFont font;
        font.setPixelSize(osdFontSize);
        painter->setPen(osdColor);
        painter->setFont(font);

        painter->drawText(osdRect, flag, text);
    }

    painter->restore();
}

QImage USBCameraWidget::getImage() const
{
    usbCamera->snap();
    return this->image;
}

QImage USBCameraWidget::getImageOnly() const
{
    return this->image;
}

QDateTime USBCameraWidget::getLastTime() const
{
    return usbCamera->getLastTime();
}

QString USBCameraWidget::getCameraName() const
{
    return usbCamera->getCameraName();
}

int USBCameraWidget::getCameraWidth() const
{
    return this->cameraWidth;
}

int USBCameraWidget::getCameraHeight() const
{
    return this->cameraHeight;
}

bool USBCameraWidget::getCopyImage() const
{
    return this->copyImage;
}

bool USBCameraWidget::getCheckLive() const
{
    return this->checkLive;
}

bool USBCameraWidget::getDrawImage() const
{
    return this->drawImage;
}

bool USBCameraWidget::getFillImage() const
{
    return this->fillImage;
}

int USBCameraWidget::getTimeout() const
{
    return this->timeout;
}

int USBCameraWidget::getBorderWidth() const
{
    return this->borderWidth;
}

QColor USBCameraWidget::getBorderColor() const
{
    return this->borderColor;
}

QColor USBCameraWidget::getFocusColor() const
{
    return this->focusColor;
}

QString USBCameraWidget::getBgText() const
{
    return this->bgText;
}

QImage USBCameraWidget::getBgImage() const
{
    return this->bgImage;
}

bool USBCameraWidget::getOSD1Visible() const
{
    return this->osd1Visible;
}

int USBCameraWidget::getOSD1FontSize() const
{
    return this->osd1FontSize;
}

QString USBCameraWidget::getOSD1Text() const
{
    return this->osd1Text;
}

QColor USBCameraWidget::getOSD1Color() const
{
    return this->osd1Color;
}

QImage USBCameraWidget::getOSD1Image() const
{
    return this->osd1Image;
}

USBCameraWidget::OSDFormat USBCameraWidget::getOSD1Format() const
{
    return this->osd1Format;
}

USBCameraWidget::OSDPosition USBCameraWidget::getOSD1Position() const
{
    return this->osd1Position;
}

bool USBCameraWidget::getOSD2Visible() const
{
    return this->osd2Visible;
}

int USBCameraWidget::getOSD2FontSize() const
{
    return this->osd2FontSize;
}

QString USBCameraWidget::getOSD2Text() const
{
    return this->osd2Text;
}

QColor USBCameraWidget::getOSD2Color() const
{
    return this->osd2Color;
}

QImage USBCameraWidget::getOSD2Image() const
{
    return this->osd2Image;
}

USBCameraWidget::OSDFormat USBCameraWidget::getOSD2Format() const
{
    return this->osd2Format;
}

USBCameraWidget::OSDPosition USBCameraWidget::getOSD2Position() const
{
    return this->osd2Position;
}

void USBCameraWidget::updateImage(const QImage &image)
{
    //拷贝图片有个好处,当处理器比较差的时候,图片不会产生断层,缺点是占用时间
    //默认QImage类型是浅拷贝,可能正在绘制的时候,那边已经更改了图片的上部分数据
    this->image = copyImage ? image.copy() : image;
    this->update();
}

void USBCameraWidget::checkCamera()
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime lastTime = usbCamera->getLastTime();
    int sec = lastTime.secsTo(now);
    if (sec >= timeout) {
        restart();
    }
}

void USBCameraWidget::setFindFaceRect(bool findFaceRect, bool findFast)
{
    usbCamera->setFindFaceRect(findFaceRect, findFast);
}

void USBCameraWidget::setFindFaceOne(bool findFaceOne)
{
    usbCamera->setFindFaceOne(findFaceOne);
}

void USBCameraWidget::setInterval(int interval)
{
    usbCamera->setInterval(interval);
}

void USBCameraWidget::setCameraName(const QString &cameraName)
{
    usbCamera->setCameraName(cameraName);
}

void USBCameraWidget::setCameraWidth(int cameraWidth)
{
    usbCamera->setCameraWidth(cameraWidth);
}

void USBCameraWidget::setCameraHeight(int cameraHeight)
{
    usbCamera->setCameraHeight(cameraHeight);
}

void USBCameraWidget::setCopyImage(bool copyImage)
{
    this->copyImage = copyImage;
}

void USBCameraWidget::setCheckLive(bool checkLive)
{
    this->checkLive = checkLive;
}

void USBCameraWidget::setDrawImage(bool drawImage)
{
    this->drawImage = drawImage;
    usbCamera->setDrawImage(drawImage);
}

void USBCameraWidget::setFillImage(bool fillImage)
{
    this->fillImage = fillImage;
    usbCamera->setFillImage(fillImage);
}

void USBCameraWidget::setTimeout(int timeout)
{
    this->timeout = timeout;
}

void USBCameraWidget::setBorderWidth(int borderWidth)
{
    this->borderWidth = borderWidth;
    usbCamera->setBorderWidth(borderWidth);
}

void USBCameraWidget::setBorderColor(const QColor &borderColor)
{
    this->borderColor = borderColor;
}

void USBCameraWidget::setFocusColor(const QColor &focusColor)
{
    this->focusColor = focusColor;
}

void USBCameraWidget::setBgText(const QString &bgText)
{
    this->bgText = bgText;
}

void USBCameraWidget::setBgImage(const QImage &bgImage)
{
    this->bgImage = bgImage;
}

void USBCameraWidget::setOSD1Visible(bool visible)
{
    this->osd1Visible = visible;
}

void USBCameraWidget::setOSD1FontSize(int fontSize)
{
    this->osd1FontSize = fontSize;
}

void USBCameraWidget::setOSD1Text(const QString &text)
{
    this->osd1Text = text;
}

void USBCameraWidget::setOSD1Color(const QColor &color)
{
    this->osd1Color = color;
}

void USBCameraWidget::setOSD1Image(const QImage &osdImage)
{
    this->osd1Image = osdImage;
}

void USBCameraWidget::setOSD1Format(const USBCameraWidget::OSDFormat &osdFormat)
{
    this->osd1Format = osdFormat;
}

void USBCameraWidget::setOSD1Position(const USBCameraWidget::OSDPosition &osdPosition)
{
    this->osd1Position = osdPosition;
}

void USBCameraWidget::setOSD2Visible(bool visible)
{
    this->osd2Visible = visible;
}

void USBCameraWidget::setOSD2FontSize(int fontSize)
{
    this->osd2FontSize = fontSize;
}

void USBCameraWidget::setOSD2Text(const QString &text)
{
    this->osd2Text = text;
}

void USBCameraWidget::setOSD2Color(const QColor &color)
{
    this->osd2Color = color;
}

void USBCameraWidget::setOSD2Image(const QImage &osdImage)
{
    this->osd2Image = osdImage;
}

void USBCameraWidget::setOSD2Format(const USBCameraWidget::OSDFormat &osdFormat)
{
    this->osd2Format = osdFormat;
}

void USBCameraWidget::setOSD2Position(const USBCameraWidget::OSDPosition &osdPosition)
{
    this->osd2Position = osdPosition;
}

bool USBCameraWidget::open()
{
    bool ok = usbCamera->init();
    if (ok) {
        usbCamera->open();
    }

    if (checkLive) {
        timerCheck->start();
    }

    return ok;
}

void USBCameraWidget::pause()
{
    usbCamera->pause();
}

void USBCameraWidget::next()
{
    usbCamera->next();
}

void USBCameraWidget::close()
{
    if (usbCamera->isRunning()) {
        usbCamera->stop();
        usbCamera->quit();
        usbCamera->wait(500);
    }

    if (checkLive) {
        timerCheck->stop();
    }

    QTimer::singleShot(1, this, SLOT(clear()));
}

void USBCameraWidget::restart()
{
    qDebug() << TIMEMS << "restart camera";
    close();
    QTimer::singleShot(10, this, SLOT(open()));
}

void USBCameraWidget::clear()
{
    image = QImage();
    update();
}
