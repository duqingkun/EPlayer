#ifndef DECODER_H
#define DECODER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QImage>
#include <QElapsedTimer>

extern "C"
{
#include <libavutil/pixfmt.h>
#include <libavutil/samplefmt.h>
}

class AVFormatContext;
class AVCodec;
class AVCodecContext;
class AVPacket;
class SwsContext;
class SwrContext;
class AVFrame;


typedef struct _AudioParam
{
    int rate;
    int ch_layout;
    AVSampleFormat fmt;

    //private
    int __nb_channels;
    int __nb_samples;
}AudioParam;

typedef struct _VideoParam
{
    int width;
    int height;
    AVPixelFormat fmt;
}VideoParam;

typedef struct _Param
{
    AudioParam audio;
    VideoParam video;
}Param;

typedef struct _Context
{
    AVFormatContext *fmt_ctx;
    AVCodec *a_codec;
    AVCodec *v_codec;
    AVCodecContext *a_codec_ctx;
    AVCodecContext *v_codec_ctx;
    SwsContext *sws_ctx;
    SwrContext *swr_ctx;
    AVFrame *v_frm;
    AVFrame *a_frm;
    int a_index;
    int v_index;
    bool hasAudio;
    bool hasVideo;
}Context;

class DecodeThread : public QThread
{
    Q_OBJECT
public:
    DecodeThread(QObject *parent = nullptr);
    ~DecodeThread();

    ///
    /// \brief 加载音视频文件
    /// \param filename: 文件路径
    ///
    void load(const QString &filename);

signals:
    ///
    /// \brief 加载完成，可以开始播放
    ///
    void loaded();

    ///
    /// \brief 视频解码信息
    /// \param w: 视频宽度
    /// \param h: 视频高度
    ///
    void videoInfo(int w, int h);

    ///
    /// \brief 音频解码信息
    /// \param sampleRate: 采样率
    /// \param channels: 通道数
    /// \param fmt: AVSampleFormat格式
    ///
    void audioInfo(int sampleRate, int channels, int fmt);

    ///
    /// \brief 发送一帧视频图像数据
    /// \param image
    ///
    void frame(QImage image);

    ///
    /// \brief 发送一帧音频数据
    /// \param data
    /// \param length
    ///
    void audio(const char *data, int length);

protected:
    ///
    /// \brief 处理解码流程
    ///
    void run() override;

private:
    void freeAll();

    ///
    /// \brief 打开输入文件，并打开相应的解码器
    /// \param ctx: 上下文指针
    /// \param filename: 输入文件路径
    /// \return
    ///
    bool openInput(Context *ctx, const char *filename);

    ///
    /// \brief 解码一个packet
    /// \param ctx
    /// \param pkt: 解码前的一包数据
    /// \param param: 参数
    ///
    void decodePacket(Context *ctx, AVPacket *pkt, Param *param);

    ///
    /// \brief 初始化，打开解码器并获取参数
    /// \param filename
    /// \return
    ///
    bool init(const QString &filename);

    ///
    /// \brief 解码一个音频包
    /// \param ctx
    /// \param pkt
    /// \param frm: 用于存放解码后的数据
    /// \param param
    ///
    void decodeAudioPacket(Context *ctx, AVPacket *pkt, AVFrame *frm, Param *param);

    ///
    /// \brief 解码一个视频包
    /// \param ctx
    /// \param pkt
    /// \param frm: 用于存放解码后的数据
    /// \param param
    ///
    void decodeVideoPacket(Context *ctx, AVPacket *pkt, AVFrame *frm, Param *param);

signals:

private:
    Context mCtx;
    Param mParam;

    QString mFilename;

    bool mAbort;
    QMutex mMutex;

    QElapsedTimer mTimestampTimer;
};

#endif // DECODER_H
