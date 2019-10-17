#include "ffmpeg.h"

#define TIMEMS      qPrintable(QTime::currentTime().toString("HH:mm:ss zzz"))
#define STRDATETIME qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss"))

FFmpegThread::FFmpegThread(QObject *parent) : QThread(parent)
{
    setObjectName("FFmpegThread");
    stopped = false;
    isPlay = false;
    isPause = false;
    isRtsp = false;

    lastTime = QDateTime::currentDateTime();
    frameCount = 0;
    frameFinish = 0;
    videoWidth = 0;
    videoHeight = 0;
    oldWidth = 0;
    oldHeight = 0;
    videoStreamIndex = -1;
    audioStreamIndex = -1;
    videoFps = 0;

    interval = 1;
    sleepTime = 0;
    checkTime = 1000;
    checkConn = false;
    url = "rtsp://192.168.1.200:554/1";
    hardware = "none";

    saveFile = false;
    saveInterval = 0;
    savePath = qApp->applicationDirPath();
    fileName = QString("%1/%2.mp4").arg(savePath).arg(STRDATETIME);

    //定时器用于隔一段时间保存文件,文件名按照 yyyy-MM-dd-HH-mm-ss 格式
    timerSave = new QTimer(this);
    timerSave->setInterval(60 * 1000);
    connect(timerSave, SIGNAL(timeout()), this, SLOT(save()));
    connect(this, SIGNAL(sig_startSave()), this, SLOT(startSave()));
    connect(this, SIGNAL(sig_stopSave()), this, SLOT(stopSave()));

    buffer = NULL;
    avPacket = NULL;
    avFrame = NULL;
    avFrame2 = NULL;
    avFrame3 = NULL;
    avFormatContext = NULL;
    videoCodec = NULL;
    audioCodec = NULL;
    swsContext = NULL;

    //音频dts头部数据
    int profile = 2;
    int freqIdx = 4;
    int chanCfg = 2;
    dtsData = (char *)malloc(sizeof(char) * 7);
    dtsData[0] = (char)0xFF;
    dtsData[1] = (char)0xF1;
    dtsData[2] = (char)(((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
    dtsData[6] = (char)0xFC;

#ifndef gcc45
    filter = av_bsf_get_by_name("h264_mp4toannexb");
#endif

    //初始化注册,一个软件中只注册一次即可
    FFmpegThread::initlib();
}

//一个软件中只需要初始化一次就行
void FFmpegThread::initlib()
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    static bool isInit = false;
    if (!isInit) {
        //注册库中所有可用的文件格式和解码器
        av_register_all();
        //初始化网络流格式,使用RTSP网络流时必须先执行
        avformat_network_init();

        isInit = true;
        qDebug() << TIMEMS << "init ffmpeg lib ok" << " version:" << FFMPEG_VERSION;
#if 0
        //输出所有支持的解码器名称
        QStringList listCodeName;
        AVCodec *code = av_codec_next(NULL);
        while (code != NULL) {
            listCodeName << code->name;
            code = code->next;
        }

        qDebug() << TIMEMS << listCodeName;
#endif
    }
}

int FFmpegThread::av_bsf_filter(const AVBitStreamFilter *filter, AVPacket *pPacket, const AVCodecParameters *src)
{
#ifndef gcc45
    int ret;
    AVBSFContext *ctx = NULL;
    if (!filter) {
        return 0;
    }

    ret = av_bsf_alloc(filter, &ctx);
    if (ret < 0) {
        return ret;
    }

    ret = avcodec_parameters_copy(ctx->par_in, src);
    if (ret < 0) {
        return ret;
    }

    ret = av_bsf_init(ctx);
    if (ret < 0) {
        return ret;
    }

    AVPacket pkt = {0};
    pkt.data = pPacket->data;
    pkt.size = pPacket->size;

    ret = av_bsf_send_packet(ctx, &pkt);
    if (ret < 0) {
        return ret;
    }

    ret = av_bsf_receive_packet(ctx, &pkt);
    if (pkt.data == pPacket->data) {
        uint8_t *poutbuf = (uint8_t *)av_malloc(pkt.size);
        if (!poutbuf) {
            av_packet_unref(&pkt);
            av_free(poutbuf);
            return -1;
        }

        memcpy(poutbuf, pkt.data, pkt.size);
        av_packet_unref(pPacket);
        pPacket->data = poutbuf;
        pPacket->size = pkt.size;
        av_packet_unref(&pkt);
        av_bsf_free(&ctx);
        av_free(poutbuf);
        return 1;
    }

    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return 0;
    } else if (ret < 0) {
        return ret;
    }

    uint8_t *poutbuf = (uint8_t *)av_malloc(pkt.size + AV_INPUT_BUFFER_PADDING_SIZE);
    if (!poutbuf) {
        av_packet_unref(&pkt);
        av_free(poutbuf);
        return AVERROR(ENOMEM);
    }

    int poutbuf_size = pkt.size;
    memcpy(poutbuf, pkt.data, pkt.size);
    pPacket->data = poutbuf;
    pPacket->size = poutbuf_size;
    av_packet_unref(&pkt);

    while (ret >= 0) {
        ret = av_bsf_receive_packet(ctx, &pkt);
        av_packet_unref(&pkt);
    }

    av_packet_unref(&pkt);
    av_bsf_free(&ctx);
    av_free(poutbuf);
#endif
    return 1;
}

int FFmpegThread::decode_packet(AVCodecContext *avctx, AVPacket *packet)
{
#ifndef gcc45
    int ret = 0;
    ret = avcodec_send_packet(avctx, packet);
    if (ret < 0) {
        qDebug() << TIMEMS << "Error during decoding";
        return ret;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(avctx, avFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            qDebug() << TIMEMS << "Error during decoding";
            break;
        }

        ret = av_hwframe_transfer_data(avFrame2, avFrame, 0);
        if (ret < 0) {
            qDebug() << TIMEMS << "Error transferring the data to system memory";
            av_frame_unref(avFrame2);
            av_frame_unref(avFrame);
            return ret;
        }
    }
#endif

    return 1;
}

static AVPixelFormat get_qsv_format(AVCodecContext *avctx, const enum AVPixelFormat *pix_fmts)
{
#ifndef gcc45
    while (*pix_fmts != AV_PIX_FMT_NONE) {
        if (*pix_fmts == AV_PIX_FMT_QSV) {
            DecodeContext *decode = (DecodeContext *)avctx->opaque;
            avctx->hw_frames_ctx = av_hwframe_ctx_alloc(decode->hw_device_ref);
            if (!avctx->hw_frames_ctx) {
                return AV_PIX_FMT_NONE;
            }

            AVHWFramesContext *frames_ctx = (AVHWFramesContext *)avctx->hw_frames_ctx->data;
            AVQSVFramesContext *frames_hwctx = (AVQSVFramesContext *)frames_ctx->hwctx;

            frames_ctx->format = AV_PIX_FMT_QSV;
            frames_ctx->sw_format = avctx->sw_pix_fmt;
            frames_ctx->width = FFALIGN(avctx->coded_width, 32);
            frames_ctx->height = FFALIGN(avctx->coded_height, 32);
            frames_ctx->initial_pool_size = 32;
            frames_hwctx->frame_type = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;

            int ret = av_hwframe_ctx_init(avctx->hw_frames_ctx);
            if (ret < 0) {
                return AV_PIX_FMT_NONE;
            }

            return AV_PIX_FMT_QSV;
        }

        pix_fmts++;
    }
#endif

    qDebug() << TIMEMS << "The QSV pixel format not offered in get_format()";
    return AV_PIX_FMT_NONE;
}

enum AVPixelFormat hw_pix_fmt;
static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;
    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt) {
            return *p;
        }
    }

    qDebug() << TIMEMS << "Failed to get HW surface format";
    return AV_PIX_FMT_NONE;
}

bool FFmpegThread::init()
{
    if (!checkUrl()) {
        return false;
    }

    decode = {NULL};
    AVDictionary *options = NULL;
    AVCodec *videoDecoder = NULL;
    AVCodec *audioDecoder = NULL;

    //设置缓存大小,1080p可将值调大
    av_dict_set(&options, "buffer_size", "8192000", 0);
    //以udp方式打开,如果以tcp方式打开将udp替换为tcp
    av_dict_set(&options, "rtsp_transport", "udp", 0);
    //设置超时断开连接时间,单位微秒,3000000表示3秒
    av_dict_set(&options, "stimeout", "3000000", 0);
    //设置最大时延,单位微秒,1000000表示1秒
    av_dict_set(&options, "max_delay", "1000000", 0);
    //自动开启线程数
    av_dict_set(&options, "threads", "auto", 0);

    //打开视频流
    avFormatContext = avformat_alloc_context();
    int result = avformat_open_input(&avFormatContext, url.toStdString().data(), NULL, &options);
    if (result < 0) {
        qDebug() << TIMEMS << "open input error" << url;
        return false;
    }

    //释放设置参数
    if(options != NULL) {
        av_dict_free(&options);
    }

    //获取流信息
    result = avformat_find_stream_info(avFormatContext, NULL);
    if (result < 0) {
        qDebug() << TIMEMS << "find stream info error";
        return false;
    }

    //----------视频流部分开始,打个标记方便折叠代码----------
    if (1) {
        //循环查找视频流索引,以下两种方法都可以
        videoStreamIndex = -1;
#if 1
        videoStreamIndex = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &videoDecoder, 0);
#else
        for (uint i = 0; i < avFormatContext->nb_streams; i++) {
            if (avFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStreamIndex = i;
                break;
            }
        }
#endif

        if (videoStreamIndex < 0) {
            qDebug() << TIMEMS << "find video stream index error";
            return false;
        }

        //获取视频流
        AVStream *videoStream = avFormatContext->streams[videoStreamIndex];

        //如果选择了硬解码则根据硬解码的类型处理
        if (hardware == "none") {
            //获取视频流解码器,或者指定解码器
            videoCodec = videoStream->codec;
            videoDecoder = avcodec_find_decoder(videoCodec->codec_id);
            //videoDecoder = avcodec_find_decoder_by_name("h264_qsv");
            if (videoDecoder == NULL) {
                qDebug() << TIMEMS << "video decoder not found";
                return false;
            }
        } else if (hardware == "qsv") {
#ifndef gcc45
            //创建硬解码设备
            result = av_hwdevice_ctx_create(&decode.hw_device_ref, AV_HWDEVICE_TYPE_QSV, "auto", NULL, 0);
            if (result < 0) {
                qDebug() << TIMEMS << "open the hardware device error";
                return false;
            }

            //英特尔处理器是h264_qsv,英伟达处理器是h264_cuvid
            videoDecoder = avcodec_find_decoder_by_name("h264_qsv");
            if (videoDecoder == NULL) {
                qDebug() << TIMEMS << "video decoder not found";
                return false;
            }

            videoCodec = avcodec_alloc_context3(videoDecoder);
            if (!videoCodec) {
                qDebug() << TIMEMS << "avcodec_alloc_context3 error";
                return false;
            }

            videoCodec->codec_id = AV_CODEC_ID_H264;
            if (videoStream->codecpar->extradata_size) {
                videoCodec->extradata = (uint8_t *)av_mallocz(videoStream->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
                if (!videoCodec->extradata) {
                    return false;
                }

                memcpy(videoCodec->extradata, videoStream->codecpar->extradata, videoStream->codecpar->extradata_size);
                videoCodec->extradata_size = videoStream->codecpar->extradata_size;
            }

            videoCodec->refcounted_frames = 1;
            videoCodec->opaque = &decode;
            videoCodec->get_format = get_qsv_format;
#endif
        } else {
#ifndef gcc45
            enum AVHWDeviceType type = av_hwdevice_find_type_by_name(hardware.toStdString().data());
            qDebug() << TIMEMS << "AVHWDeviceType" << type;

            //找到对应的硬解码格式
            switch (type) {
            case AV_HWDEVICE_TYPE_QSV:
                hw_pix_fmt = AV_PIX_FMT_QSV;
                break;
            case AV_HWDEVICE_TYPE_VAAPI:
                hw_pix_fmt = AV_PIX_FMT_VAAPI;
                break;
            case AV_HWDEVICE_TYPE_DXVA2:
                hw_pix_fmt = AV_PIX_FMT_DXVA2_VLD;
                break;
            case AV_HWDEVICE_TYPE_D3D11VA:
                hw_pix_fmt = AV_PIX_FMT_D3D11;
                break;
            case AV_HWDEVICE_TYPE_VDPAU:
                hw_pix_fmt = AV_PIX_FMT_VDPAU;
                break;
            case AV_HWDEVICE_TYPE_VIDEOTOOLBOX:
                hw_pix_fmt = AV_PIX_FMT_VIDEOTOOLBOX;
                break;
            case AV_HWDEVICE_TYPE_DRM:
                hw_pix_fmt = AV_PIX_FMT_DRM_PRIME;
                break;
            default:
                hw_pix_fmt = AV_PIX_FMT_NONE;
                break;
            }

            if (hw_pix_fmt == -1) {
                qDebug() << TIMEMS << "cannot support hardware";
                return false;
            }

            videoCodec = avcodec_alloc_context3(videoDecoder);
            if (!videoCodec) {
                qDebug() << TIMEMS << "avcodec_alloc_context3 error";
                return false;
            }

            result = avcodec_parameters_to_context(videoCodec, videoStream->codecpar);
            if (result < 0) {
                qDebug() << TIMEMS << "avcodec_parameters_to_context error";
                return false;
            }

            videoCodec->get_format = get_hw_format;
            //av_opt_set_int(videoCodec, "refcounted_frames", 1, 0);

            //创建硬解码设备
            result = av_hwdevice_ctx_create(&decode.hw_device_ref, type, NULL, NULL, 0);
            if (result < 0) {
                qDebug() << TIMEMS << "open the hardware device error";
                return false;
            }

            videoCodec->hw_device_ctx = av_buffer_ref(decode.hw_device_ref);
#endif
        }

        //设置加速解码
        videoCodec->lowres = videoDecoder->max_lowres;
#ifndef gcc45
        videoCodec->flags2 |= AV_CODEC_FLAG2_FAST;
#endif

        //打开视频解码器
        result = avcodec_open2(videoCodec, videoDecoder, NULL);
        if (result < 0) {
            qDebug() << TIMEMS << "open video codec error";
            return false;
        }

        //获取分辨率大小
        videoWidth = videoStream->codec->width;
        videoHeight = videoStream->codec->height;

        //如果没有获取到宽高则返回
        if (videoWidth == 0 || videoHeight == 0) {
            qDebug() << TIMEMS << "find width height error";
            return false;
        }

        //获取视频流的帧率 fps,要对0进行过滤,除数不能为0,有些时候获取到的是0
        int num = videoStream->avg_frame_rate.num;
        int den = videoStream->avg_frame_rate.den;
        if (num != 0 && den != 0) {
            videoFps = num / den;
        }

        QString videoInfo = QString("视频流信息 -> 索引: %1  解码: %2  格式: %3  时长: %4 秒  fps: %5  分辨率: %6*%7")
                            .arg(videoStreamIndex).arg(videoDecoder->name).arg(avFormatContext->iformat->name)
                            .arg((avFormatContext->duration) / 1000000).arg(videoFps).arg(videoWidth).arg(videoHeight);
        qDebug() << TIMEMS << videoInfo;
    }
    //----------视频流部分开始----------

    //----------音频流部分开始,打个标记方便折叠代码----------
    if (1) {
        //循环查找音频流索引
        audioStreamIndex = -1;
        for (uint i = 0; i < avFormatContext->nb_streams; i++) {
            if (avFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
                audioStreamIndex = i;
                break;
            }
        }

        //有些没有音频流,所以这里不用返回
        if (audioStreamIndex == -1) {
            qDebug() << TIMEMS << "find audio stream index error";
        } else {
            //获取音频流
            AVStream *audioStream = avFormatContext->streams[audioStreamIndex];
            audioCodec = audioStream->codec;

            //获取音频流解码器,或者指定解码器
            audioDecoder = avcodec_find_decoder(audioCodec->codec_id);
            //audioDecoder = avcodec_find_decoder_by_name("aac");
            if (audioDecoder == NULL) {
                qDebug() << TIMEMS << "audio codec not found";
                return false;
            }

            //打开音频解码器
            result = avcodec_open2(audioCodec, audioDecoder, NULL);
            if (result < 0) {
                qDebug() << TIMEMS << "open audio codec error";
                return false;
            }

            QString audioInfo = QString("音频流信息 -> 索引: %1  解码: %2  比特率: %3  声道数: %4  采样: %5")
                                .arg(audioStreamIndex).arg(audioDecoder->name).arg(avFormatContext->bit_rate)
                                .arg(audioCodec->channels).arg(audioCodec->sample_rate);
            qDebug() << TIMEMS << audioInfo;
        }
    }
    //----------音频流部分结束----------

    //预分配好内存
#ifndef gcc45
    avPacket = av_packet_alloc();
#else
    avPacket = new AVPacket;
#endif

    avFrame = av_frame_alloc();
    avFrame2 = av_frame_alloc();
    avFrame3 = av_frame_alloc();

    //比较上一次文件的宽度高度,当改变时,需要重新分配内存
    if (oldWidth != videoWidth || oldHeight != videoHeight) {
        int byte = avpicture_get_size(AV_PIX_FMT_RGB32, videoWidth, videoHeight);
        buffer = (uint8_t *)av_malloc(byte * sizeof(uint8_t));
        oldWidth = videoWidth;
        oldHeight = videoHeight;
    }

    //以下两种方法都可以
    //avpicture_fill((AVPicture *)avFrame3, buffer, AV_PIX_FMT_RGB32, videoWidth, videoHeight);
    av_image_fill_arrays(avFrame3->data, avFrame3->linesize, buffer, AV_PIX_FMT_RGB32, videoWidth, videoHeight, 1);

    if (hardware == "none") {
        swsContext = sws_getContext(videoWidth, videoHeight, AV_PIX_FMT_YUV420P, videoWidth, videoHeight, AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    } else {
        swsContext = sws_getContext(videoWidth, videoHeight, AV_PIX_FMT_NV12, videoWidth, videoHeight, AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    }

    //输出视频信息
    //av_dump_format(avFormatContext, 0, url.toStdString().data(), 0);

    //qDebug() << TIMEMS << "init ffmpeg finsh";
    return true;
}

void FFmpegThread::run()
{
    //计时
    QTime time;
    while (!stopped) {
        //根据标志位执行初始化操作
        if (isPlay) {
            if (init()) {
                //启用保存文件,先关闭文件
                if (saveFile) {
                    if (fileVideo.isOpen()) {
                        fileVideo.close();
                    }

                    if (fileAudio.isOpen()) {
                        fileAudio.close();
                    }

                    //如果存储间隔大于0说明需要定时存储
                    if (saveInterval > 0) {
                        fileName = QString("%1/%2.mp4").arg(savePath).arg(STRDATETIME);
                        emit sig_startSave();
                    }

                    if (videoStreamIndex >= 0) {
                        fileVideo.setFileName(fileName);
                        fileVideo.open(QFile::WriteOnly);
                    }

                    if (audioStreamIndex >= 0) {
                        fileAudio.setFileName(fileName.replace(QFileInfo(fileName).suffix(), "aac"));
                        fileAudio.open(QFile::WriteOnly);
                    }
                }

                emit receivePlayOk();
            } else {
                break;
                emit receivePlayError();
            }

            isPlay = false;
            continue;
        }

        if (isPause) {
            //这里需要假设正常,暂停期间继续更新时间
            lastTime = QDateTime::currentDateTime();
            msleep(1);
            continue;
        }

        time.restart();
        if (av_read_frame(avFormatContext, avPacket) >= 0) {
            //判断当前包是视频还是音频
            int packetSize = avPacket->size;
            int index = avPacket->stream_index;
            if (index == videoStreamIndex) {
                //解码视频流
                if (hardware == "none") {
                    avcodec_decode_video2(videoCodec, avFrame2, &frameFinish, avPacket);
                } else {
                    frameFinish = decode_packet(videoCodec, avPacket);
                }

                if (frameFinish) {
                    //计数,只有到了设定的帧率才刷新图片
                    frameCount++;
                    if (frameCount != interval) {
                        av_packet_unref(avPacket);
                        msleep(1);
                        continue;
                    } else {
                        frameCount = 0;
                    }

                    //保存视频流数据到文件
                    QMutexLocker lock(&mutex);
                    if (fileVideo.isOpen()) {
                        //rtmp视频流需要添加pps sps
#ifndef gcc45
                        av_bsf_filter(filter, avPacket, avFormatContext->streams[videoStreamIndex]->codecpar);
#endif
                        fileVideo.write((const char *)avPacket->data, packetSize);
                    }

                    //将数据转成一张图片
                    sws_scale(swsContext, (const uint8_t *const *)avFrame2->data, avFrame2->linesize, 0, videoHeight, avFrame3->data, avFrame3->linesize);
                    //以下两种方法都可以
                    //QImage image(avFrame3->data[0], videoWidth, videoHeight, QImage::Format_RGB32);
                    QImage image((uchar *)buffer, videoWidth, videoHeight, QImage::Format_RGB32);

                    if (!image.isNull()) {
                        lastTime = QDateTime::currentDateTime();
                        emit receiveImage(image);

                        int useTime = time.elapsed();
                        if (!isRtsp && videoFps > 0) {
                            //一帧解码用时+固定休眠1毫秒+其他用时1毫秒
                            int frameTime = useTime + 1 + 1;
                            //等待时间=1秒钟即1000毫秒-所有帧解码完成用的毫秒数/帧数
                            sleepTime = (1000 - (videoFps * frameTime)) / videoFps;
                            //有时候如果图片很大或者解码很难比如h265造成解码一张图片耗时很大可能出现负数
                            sleepTime = sleepTime < 0 ? 0 : sleepTime;
                        }

                        //qDebug() << TIMEMS << image.size() << "use time" << time.elapsed() << "sleep time" << sleepTime;
                    }

                    msleep(sleepTime);
                }
            } else if (index == audioStreamIndex) {
                //解码音频流,这里暂不处理,以后交给sdl播放

                //保存音频流数据到文件
                QMutexLocker lock(&mutex);
                if (fileAudio.isOpen()) {
                    //先写入dts头,再写入音频流数据
                    dtsData[3] = (char)(((2 & 3) << 6) + ((7 + packetSize) >> 11));
                    dtsData[4] = (char)(((7 + packetSize) & 0x7FF) >> 3);
                    dtsData[5] = (char)((((7 + packetSize) & 7) << 5) + 0x1F);
                    fileAudio.write((const char *)dtsData, 7);
                    fileAudio.write((const char *)avPacket->data, packetSize);
                }
            }
        } else if (!isRtsp) {
            //如果不是视频流则说明是视频文件播放完毕
            break;
        }

        av_packet_unref(avPacket);
        msleep(1);
    }

    emit sig_stopSave();

    //线程结束后释放资源
    free();
    stopped = false;
    isPlay = false;
    isPause = false;

    emit receivePlayFinsh();
    //qDebug() << TIMEMS << "stop ffmpeg thread";
}

void FFmpegThread::startSave()
{
    timerSave->start(saveInterval * 1000);
}

void FFmpegThread::stopSave()
{
    //停止存储定时器
    if (timerSave->isActive()) {
        timerSave->stop();
    }
}

void FFmpegThread::save()
{
    QMutexLocker lock(&mutex);

    //只有启用了保存文件才保存,先关闭文件
    if (saveFile) {
        if (fileVideo.isOpen()) {
            fileVideo.close();
        }

        if (fileAudio.isOpen()) {
            fileAudio.close();
        }

        //重新设置文件名称
        fileName = QString("%1/%2.mp4").arg(savePath).arg(STRDATETIME);

        if (videoStreamIndex >= 0) {
            fileVideo.setFileName(fileName);
            fileVideo.open(QFile::WriteOnly);
        }

        if (audioStreamIndex >= 0) {
            fileAudio.setFileName(fileName.replace(QFileInfo(fileName).suffix(), "aac"));
            fileAudio.open(QFile::WriteOnly);
        }
    }
}

QDateTime FFmpegThread::getLastTime()
{
    return this->lastTime;
}

QString FFmpegThread::getUrl()
{
    return this->url;
}

int FFmpegThread::getVideoWidth()
{
    return this->videoWidth;
}

int FFmpegThread::getVideoHeight()
{
    return this->videoHeight;
}

void FFmpegThread::setInterval(int interval)
{
    QMutexLocker lock(&mutex);
    if (interval > 0) {
        this->interval = interval;
        this->frameCount = 0;
    }
}

void FFmpegThread::setSleepTime(int sleepTime)
{
    if (sleepTime > 0) {
        this->sleepTime = sleepTime;
    }
}

void FFmpegThread::setCheckTime(int checkTime)
{
    this->checkTime = checkTime;
}

void FFmpegThread::setCheckConn(bool checkConn)
{
    this->checkConn = checkConn;
}

void FFmpegThread::setUrl(const QString &url)
{
    this->url = url;
    isRtsp = (url.startsWith("rtsp") || url.startsWith("rtmp") || url.startsWith("http"));
}

void FFmpegThread::setHardware(const QString &hardware)
{
    this->hardware = hardware;
}

void FFmpegThread::setSaveFile(bool saveFile)
{
    this->saveFile = saveFile;
}

void FFmpegThread::setSaveInterval(int saveInterval)
{
    this->saveInterval = saveInterval;
    timerSave->setInterval(saveInterval * 1000);
}

void FFmpegThread::setSavePath(const QString &savePath)
{
    this->savePath = savePath;
}

void FFmpegThread::setFileName(const QString &fileName)
{
    this->fileName = fileName;
}

bool FFmpegThread::checkUrl()
{
    //首先判断该摄像机是否能联通
    //从字符串中找出IP地址和端口,默认端口554
    if (checkConn && isRtsp) {
        QRegExp reg("((?:(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d))");
        reg.indexIn(url);
        QString ip = url.mid(url.indexOf(reg), reg.matchedLength());
        int port = 554;
        int index = url.indexOf(ip);

        //取出端口号
        if (index >= 0) {
            //判断该IP地址后面是不是:,是则说明有端口号
            index = index + ip.length();
            QString flag = url.mid(index, 1);
            if (flag == ":") {
                //取出端口号后面的斜杠位置
                bool end = false;
                int start = 1;
                while(!end) {
                    flag = url.mid(index + start, 1);
                    if (flag >= "0" && flag <= "9") {
                        start++;
                    } else {
                        port = url.mid(index + 1, start - 1).toInt();
                        end = true;
                    }
                }
            }
        }

        QTcpSocket tcpClient;
        tcpClient.connectToHost(ip, port);

        //超时没有连接上则判断该摄像机不在线
        bool ok = tcpClient.waitForConnected(checkTime);
        tcpClient.abort();
        if (!ok) {
            qDebug() << TIMEMS << "rtsp connect error";
            return false;
        }
    }

    return true;
}

void FFmpegThread::free()
{
    //关闭文件
    if (fileVideo.isOpen()) {
        fileVideo.close();
    }

    if (fileAudio.isOpen()) {
        fileAudio.close();
    }

    if (swsContext != NULL) {
        sws_freeContext(swsContext);
        swsContext = NULL;
    }

    if (avPacket != NULL) {
        av_packet_unref(avPacket);
        avPacket = NULL;
    }

    if (avFrame != NULL) {
        av_frame_free(&avFrame);
        avFrame = NULL;
    }

    if (avFrame2 != NULL) {
        av_frame_free(&avFrame2);
        avFrame2 = NULL;
    }

    if (avFrame3 != NULL) {
        av_frame_free(&avFrame3);
        avFrame3 = NULL;
    }

    if (videoCodec != NULL) {
        avcodec_close(videoCodec);
        videoCodec = NULL;
    }

    if (audioCodec != NULL) {
        avcodec_close(audioCodec);
        audioCodec = NULL;
    }

    if (avFormatContext != NULL) {
        avformat_close_input(&avFormatContext);
        avFormatContext = NULL;
    }

    av_buffer_unref(&decode.hw_device_ref);
    //qDebug() << TIMEMS << "close ffmpeg ok";
}

void FFmpegThread::play()
{
    //通过标志位让线程执行初始化
    isPlay = true;
    isPause = false;
}

void FFmpegThread::pause()
{
    //只对非视频流起作用
    if (!isRtsp) {
        isPause = true;
    }
}

void FFmpegThread::next()
{
    //只对非视频流起作用
    if (!isRtsp) {
        isPause = false;
    }
}

void FFmpegThread::stop()
{
    //通过标志位让线程停止
    stopped = true;
}

//实时视频显示窗体类
#ifdef opengl
FFmpegWidget::FFmpegWidget(QWidget *parent) : QOpenGLWidget(parent)
#else
FFmpegWidget::FFmpegWidget(QWidget * parent) : QWidget(parent)
#endif
{
    //设置强焦点
    setFocusPolicy(Qt::StrongFocus);
    //设置支持拖放
    setAcceptDrops(true);

    ffmpeg = new FFmpegThread(this);
    connect(ffmpeg, SIGNAL(receivePlayOk()), this, SIGNAL(receivePlayOk()));
    connect(ffmpeg, SIGNAL(receivePlayError()), this, SIGNAL(receivePlayError()));
    connect(ffmpeg, SIGNAL(receivePlayFinsh()), this, SIGNAL(receivePlayFinsh()));
    connect(ffmpeg, SIGNAL(receiveImage(QImage)), this, SIGNAL(receiveImage(QImage)));
    connect(ffmpeg, SIGNAL(receiveImage(QImage)), this, SLOT(updateImage(QImage)));

    timerCheck = new QTimer(this);
    timerCheck->setInterval(10 * 1000);
    connect(timerCheck, SIGNAL(timeout()), this, SLOT(checkVideo()));

    image = QImage();

    //顶部工具栏,默认隐藏,鼠标移入显示移除隐藏
    flowPanel = new QWidget(this);
    flowPanel->setObjectName("flowPanel");
    flowPanel->setVisible(false);

    //设置样式以便区分,可以自行更改样式,也可以不用样式
    QStringList qss;
    qss.append("#flowPanel{background:rgba(0,0,0,50);border:none;}");
    qss.append("QPushButton{border:none;padding:0px;background:rgba(0,0,0,0);}");
    qss.append("QPushButton:pressed{color:rgb(32,152,158);}");
    flowPanel->setStyleSheet(qss.join(""));

    //用布局顶住,左侧弹簧
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(2);
    layout->setMargin(0);
    layout->addStretch();
    flowPanel->setLayout(layout);

    //按钮集合名称,如果需要新增按钮则在这里增加即可
    QList<QString> btns;
    btns << "btnFlowVideo" << "btnFlowSnap" << "btnFlowSound" << "btnFlowAlarm" << "btnFlowClose";

    //有多种办法来设置图片,qt内置的图标+自定义的图标+图形字体
    //既可以设置图标形式,也可以直接图形字体设置文本
#if 0
    QList<QIcon> icons;
    icons << QApplication::style()->standardIcon(QStyle::SP_ComputerIcon);
    icons << QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    icons << QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    icons << QApplication::style()->standardIcon(QStyle::SP_DialogOkButton);
    icons << QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton);
#else
    QList<QChar> chars;
    chars << 0xe68d << 0xe672 << 0xe674 << 0xea36 << 0xe74c;

    //判断图形字体是否存在,不存在则加入
    QFont iconFont;
    QFontDatabase fontDb;
    if (!fontDb.families().contains("iconfont")) {
        int fontId = fontDb.addApplicationFont(":/image/iconfont.ttf");
        QStringList fontName = fontDb.applicationFontFamilies(fontId);
        if (fontName.count() == 0) {
            qDebug() << "load iconfont.ttf error";
        }
    }

    if (fontDb.families().contains("iconfont")) {
        iconFont = QFont("iconfont");
        iconFont.setPixelSize(17);
#if (QT_VERSION >= QT_VERSION_CHECK(4,8,0))
        iconFont.setHintingPreference(QFont::PreferNoHinting);
#endif
    }
#endif

    //循环添加顶部按钮
    for (int i = 0; i < btns.count(); i++) {
        QPushButton *btn = new QPushButton;
        //绑定按钮单击事件,用来发出信号通知
        connect(btn, SIGNAL(clicked(bool)), this, SLOT(btnClicked()));
        //设置标识,用来区别按钮
        btn->setObjectName(btns.at(i));
        //设置固定宽度
        btn->setFixedWidth(20);
        //设置拉伸策略使得填充
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        //设置焦点策略为无焦点,避免单击后焦点跑到按钮上
        btn->setFocusPolicy(Qt::NoFocus);

#if 0
        //设置图标大小和图标
        btn->setIconSize(QSize(16, 16));
        btn->setIcon(icons.at(i));
#else
        btn->setFont(iconFont);
        btn->setText(chars.at(i));
#endif

        //将按钮加到布局中
        layout->addWidget(btn);
    }

    copyImage = false;
    checkLive = true;
    drawImage = true;
    fillImage = true;
    flowEnable = false;

    timeout = 20;
    borderWidth = 5;
    borderColor = "#000000";
    focusColor = "#22A3A9";
    bgText = "实时视频";
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

FFmpegWidget::~FFmpegWidget()
{
    if (timerCheck->isActive()) {
        timerCheck->stop();
    }

    close();
}

void FFmpegWidget::resizeEvent(QResizeEvent *)
{
    //重新设置顶部工具栏的位置和宽高,可以自行设置顶部显示或者底部显示
    int height = 20;
    flowPanel->setGeometry(borderWidth, borderWidth, this->width() - (borderWidth * 2), height);
    //flowPanel->setGeometry(borderWidth, this->height() - height - borderWidth, this->width() - (borderWidth * 2), height);
}

void FFmpegWidget::enterEvent(QEvent *)
{
    //这里还可以增加一个判断,是否获取了焦点的才需要显示
    //if (this->hasFocus()) {}
    if (flowEnable) {
        flowPanel->setVisible(true);
    }
}

void FFmpegWidget::leaveEvent(QEvent *)
{
    if (flowEnable) {
        flowPanel->setVisible(false);
    }
}

void FFmpegWidget::dropEvent(QDropEvent *event)
{
    //拖放完毕鼠标松开的时候执行
    //判断拖放进来的类型,取出文件,进行播放
    if(event->mimeData()->hasUrls()) {
        QString url = event->mimeData()->urls().first().toLocalFile();
        this->close();
        this->setUrl(url);
        this->open();
        emit fileDrag(url);
    } else if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        QTreeWidget *treeWidget = (QTreeWidget *)event->source();
        if (treeWidget != 0) {
            QString url = treeWidget->currentItem()->data(0, Qt::UserRole).toString();
            this->close();
            this->setUrl(url);
            this->open();
            emit fileDrag(url);
        }
    }
}

void FFmpegWidget::dragEnterEvent(QDragEnterEvent *event)
{
    //拖曳进来的时候先判断下类型,非法类型则不处理
    if(event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else if(event->mimeData()->hasFormat("text/uri-list")) {
        event->setDropAction(Qt::LinkAction);
        event->accept();
    } else {
        event->ignore();
    }
}

void FFmpegWidget::paintEvent(QPaintEvent *)
{
    //如果不需要绘制
    if (!drawImage) {
        return;
    }

    //qDebug() << TIMEMS << "paintEvent" << objectName();
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    //绘制边框
    if (borderWidth > 0) {
        drawBorder(&painter);
    }

    if (!image.isNull()) {
        //绘制背景图片
        drawImg(&painter, image);

        //绘制标签1
        if (osd1Visible) {
            drawOSD(&painter, osd1FontSize, osd1Text, osd1Color, osd1Image, osd1Format, osd1Position);
        }

        //绘制标签2
        if (osd2Visible) {
            drawOSD(&painter, osd2FontSize, osd2Text, osd2Color, osd2Image, osd2Format, osd2Position);
        }
    } else {
        //绘制背景
        drawBg(&painter);
    }
}

void FFmpegWidget::drawBorder(QPainter *painter)
{
    painter->save();
    QPen pen;
    pen.setWidth(borderWidth);
    pen.setColor(hasFocus() ? focusColor : borderColor);
    painter->setPen(pen);
    painter->drawRect(rect());
    painter->restore();
}

void FFmpegWidget::drawBg(QPainter *painter)
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

void FFmpegWidget::drawImg(QPainter *painter, QImage img)
{
    painter->save();

    int offset = borderWidth * 1 + 0;
    img = img.scaled(width() - offset, height() - offset, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (fillImage) {
        QRect rect(offset / 2, offset / 2, width() - offset, height() - offset);
        painter->drawImage(rect, img);
    } else {
        //按照比例自动居中绘制
        int pixX = rect().center().x() - img.width() / 2;
        int pixY = rect().center().y() - img.height() / 2;
        QPoint point(pixX, pixY);
        painter->drawImage(point, img);
    }

    painter->restore();
}

void FFmpegWidget::drawOSD(QPainter *painter,
                           int osdFontSize,
                           const QString &osdText,
                           const QColor &osdColor,
                           const QImage &osdImage,
                           const FFmpegWidget::OSDFormat &osdFormat,
                           const FFmpegWidget::OSDPosition &osdPosition)
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

QImage FFmpegWidget::getImage() const
{
    return this->image;
}

QDateTime FFmpegWidget::getLastTime() const
{
    return ffmpeg->getLastTime();
}

QString FFmpegWidget::getUrl() const
{
    return ffmpeg->getUrl();
}

bool FFmpegWidget::getCopyImage() const
{
    return this->copyImage;
}

bool FFmpegWidget::getCheckLive() const
{
    return this->checkLive;
}

bool FFmpegWidget::getDrawImage() const
{
    return this->drawImage;
}

bool FFmpegWidget::getFillImage() const
{
    return this->fillImage;
}

bool FFmpegWidget::getFlowEnable() const
{
    return this->flowEnable;
}

int FFmpegWidget::getTimeout() const
{
    return this->timeout;
}

int FFmpegWidget::getBorderWidth() const
{
    return this->borderWidth;
}

QColor FFmpegWidget::getBorderColor() const
{
    return this->borderColor;
}

QColor FFmpegWidget::getFocusColor() const
{
    return this->focusColor;
}

QString FFmpegWidget::getBgText() const
{
    return this->bgText;
}

QImage FFmpegWidget::getBgImage() const
{
    return this->bgImage;
}

bool FFmpegWidget::getOSD1Visible() const
{
    return this->osd1Visible;
}

int FFmpegWidget::getOSD1FontSize() const
{
    return this->osd1FontSize;
}

QString FFmpegWidget::getOSD1Text() const
{
    return this->osd1Text;
}

QColor FFmpegWidget::getOSD1Color() const
{
    return this->osd1Color;
}

QImage FFmpegWidget::getOSD1Image() const
{
    return this->osd1Image;
}

FFmpegWidget::OSDFormat FFmpegWidget::getOSD1Format() const
{
    return this->osd1Format;
}

FFmpegWidget::OSDPosition FFmpegWidget::getOSD1Position() const
{
    return this->osd1Position;
}

bool FFmpegWidget::getOSD2Visible() const
{
    return this->osd2Visible;
}

int FFmpegWidget::getOSD2FontSize() const
{
    return this->osd2FontSize;
}

QString FFmpegWidget::getOSD2Text() const
{
    return this->osd2Text;
}

QColor FFmpegWidget::getOSD2Color() const
{
    return this->osd2Color;
}

QImage FFmpegWidget::getOSD2Image() const
{
    return this->osd2Image;
}

FFmpegWidget::OSDFormat FFmpegWidget::getOSD2Format() const
{
    return this->osd2Format;
}

FFmpegWidget::OSDPosition FFmpegWidget::getOSD2Position() const
{
    return this->osd2Position;
}

void FFmpegWidget::updateImage(const QImage &image)
{
    //拷贝图片有个好处,当处理器比较差的时候,图片不会产生断层,缺点是占用时间
    //默认QImage类型是浅拷贝,可能正在绘制的时候,那边已经更改了图片的上部分数据
    this->image = copyImage ? image.copy() : image;
    this->update();
}

void FFmpegWidget::checkVideo()
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime lastTime = ffmpeg->getLastTime();
    int sec = lastTime.secsTo(now);
    if (sec >= timeout) {
        restart();
    }
}

void FFmpegWidget::btnClicked()
{
    QPushButton *btn = (QPushButton *)sender();
    emit btnClicked(btn->objectName());
}

void FFmpegWidget::setInterval(int interval)
{
    ffmpeg->setInterval(interval);
}

void FFmpegWidget::setSleepTime(int sleepTime)
{
    ffmpeg->setSleepTime(sleepTime);
}

void FFmpegWidget::setCheckTime(int checkTime)
{
    ffmpeg->setCheckTime(checkTime);
}

void FFmpegWidget::setCheckConn(bool checkConn)
{
    ffmpeg->setCheckConn(checkConn);
}

void FFmpegWidget::setUrl(const QString &url)
{
    ffmpeg->setUrl(url);
}

void FFmpegWidget::setHardware(const QString &hardware)
{
    ffmpeg->setHardware(hardware);
}

void FFmpegWidget::setSaveFile(bool saveFile)
{
    ffmpeg->setSaveFile(saveFile);
}

void FFmpegWidget::setSaveInterval(int saveInterval)
{
    ffmpeg->setSaveInterval(saveInterval);
}

void FFmpegWidget::setSavePath(const QString &savePath)
{
    //如果目录不存在则新建
    QDir dir(savePath);
    if (!dir.exists()) {
        dir.mkdir(savePath);
    }

    ffmpeg->setSavePath(savePath);
}

void FFmpegWidget::setFileName(const QString &fileName)
{
    ffmpeg->setFileName(fileName);
}

void FFmpegWidget::setCopyImage(bool copyImage)
{
    this->copyImage = copyImage;
}

void FFmpegWidget::setCheckLive(bool checkLive)
{
    this->checkLive = checkLive;
}

void FFmpegWidget::setDrawImage(bool drawImage)
{
    this->drawImage = drawImage;
}

void FFmpegWidget::setFillImage(bool fillImage)
{
    this->fillImage = fillImage;
}

void FFmpegWidget::setFlowEnable(bool flowEnable)
{
    this->flowEnable = flowEnable;
}

void FFmpegWidget::setTimeout(int timeout)
{
    this->timeout = timeout;
}

void FFmpegWidget::setBorderWidth(int borderWidth)
{
    this->borderWidth = borderWidth;
}

void FFmpegWidget::setBorderColor(const QColor &borderColor)
{
    this->borderColor = borderColor;
}

void FFmpegWidget::setFocusColor(const QColor &focusColor)
{
    this->focusColor = focusColor;
}

void FFmpegWidget::setBgText(const QString &bgText)
{
    this->bgText = bgText;
}

void FFmpegWidget::setBgImage(const QImage &bgImage)
{
    this->bgImage = bgImage;
}

void FFmpegWidget::setOSD1Visible(bool osdVisible)
{
    this->osd1Visible = osdVisible;
}

void FFmpegWidget::setOSD1FontSize(int osdFontSize)
{
    this->osd1FontSize = osdFontSize;
}

void FFmpegWidget::setOSD1Text(const QString &osdText)
{
    this->osd1Text = osdText;
}

void FFmpegWidget::setOSD1Color(const QColor &osdColor)
{
    this->osd1Color = osdColor;
}

void FFmpegWidget::setOSD1Image(const QImage &osdImage)
{
    this->osd1Image = osdImage;
}

void FFmpegWidget::setOSD1Format(const FFmpegWidget::OSDFormat &osdFormat)
{
    this->osd1Format = osdFormat;
}

void FFmpegWidget::setOSD1Position(const FFmpegWidget::OSDPosition &osdPosition)
{
    this->osd1Position = osdPosition;
}

void FFmpegWidget::setOSD2Visible(bool osdVisible)
{
    this->osd2Visible = osdVisible;
}

void FFmpegWidget::setOSD2FontSize(int osdFontSize)
{
    this->osd2FontSize = osdFontSize;
}

void FFmpegWidget::setOSD2Text(const QString &osdText)
{
    this->osd2Text = osdText;
}

void FFmpegWidget::setOSD2Color(const QColor &osdColor)
{
    this->osd2Color = osdColor;
}

void FFmpegWidget::setOSD2Image(const QImage &osdImage)
{
    this->osd2Image = osdImage;
}

void FFmpegWidget::setOSD2Format(const FFmpegWidget::OSDFormat &osdFormat)
{
    this->osd2Format = osdFormat;
}

void FFmpegWidget::setOSD2Position(const FFmpegWidget::OSDPosition &osdPosition)
{
    this->osd2Position = osdPosition;
}

void FFmpegWidget::open()
{
    //qDebug() << TIMEMS << "open video" << objectName();
    clear();

    //如果是图片则只显示图片就行
    image = QImage(ffmpeg->getUrl());
    if (!image.isNull()) {
        update();
        return;
    }

    ffmpeg->play();
    ffmpeg->start();

    if (checkLive) {
        timerCheck->start();
    }
}

void FFmpegWidget::pause()
{
    ffmpeg->pause();
}

void FFmpegWidget::next()
{
    ffmpeg->next();
}

void FFmpegWidget::close()
{
    //qDebug() << TIMEMS << "close video" << objectName();
    if (ffmpeg->isRunning()) {
        ffmpeg->stop();
        ffmpeg->quit();
        ffmpeg->wait(500);
    }

    if (checkLive) {
        timerCheck->stop();
    }

    QTimer::singleShot(1, this, SLOT(clear()));
}

void FFmpegWidget::restart()
{
    //qDebug() << TIMEMS << "restart video" << objectName();
    close();
    QTimer::singleShot(10, this, SLOT(open()));
}

void FFmpegWidget::clear()
{
    image = QImage();
    update();
}
