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

    void load(const QString &filename);

signals:
    void loaded();
    void frame(QImage image);
    void audio(unsigned char *data, int length);

protected:
    void run() override;

private:
    bool openInput(Context *ctx, const char *filename);
    void decodePacket(Context *ctx, AVPacket *pkt, Param *param);

signals:

private:
    Context mCtx;
    Param mParam;

    QString mFilename;

    bool mAbort;
    QMutex mMutex;

    FILE *fAudio;

    QElapsedTimer mTimestampTimer;
};

#endif // DECODER_H
