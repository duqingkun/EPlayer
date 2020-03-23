#include "decodethread.h"

#include <QDebug>
#include <QMutexLocker>
#include <QElapsedTimer>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

DecodeThread::DecodeThread(Cache<AudioFrame> *ac, Cache<VideoFrame> *vc, QObject *parent)
    : QThread(parent)
    , mAudioCache(ac)
    , mVideoCache(vc)
{
}

DecodeThread::~DecodeThread()
{
    mAudioCache->close();
    mVideoCache->close();
    requestInterruption();
    quit();
    wait();
}

void DecodeThread::load(const QString &filename)
{
    mFilename = filename;
    if(isRunning())
    {
        requestInterruption();
        quit();
        wait();
    }

    init(mFilename);
    emit loaded();
}

bool DecodeThread::init(const QString &filename)
{
    //初始化
    memset(&mCtx, 0, sizeof(Context));
    memset(&mParam, 0, sizeof(Param));

    //打开编码器
    if(!openInput(&mCtx, mFilename.toStdString().c_str()))
    {
        freeAll();
        return false;
    }

    //av_dump_format(mCtx.fmt_ctx, 0, filename, 0);

    //获取参数
    if(mCtx.hasVideo)
    {
        mParam.video.width = mCtx.v_codec_ctx->width;
        mParam.video.height = mCtx.v_codec_ctx->height;
        mParam.video.fmt = AV_PIX_FMT_RGB24;//mCtx.v_codec_ctx->pix_fmt;

        mCtx.sws_ctx = sws_getContext(mCtx.v_codec_ctx->width, mCtx.v_codec_ctx->height,
            mCtx.v_codec_ctx->pix_fmt, mParam.video.width, mParam.video.height, mParam.video.fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
        mCtx.v_frm = av_frame_alloc();

        emit videoInfo(mParam.video.width, mParam.video.height);
    }

    if (mCtx.hasAudio)
    {
        mParam.audio.ch_layout = AV_CH_LAYOUT_STEREO;//mCtx.a_codec_ctx->channel_layout;
        mParam.audio.rate = 44100; //mCtx.a_codec_ctx->sample_rate;
        mParam.audio.fmt = AV_SAMPLE_FMT_S16P;//mCtx.a_codec_ctx->sample_fmt;
        mParam.audio.__nb_channels = av_get_channel_layout_nb_channels(mParam.audio.ch_layout);
        mParam.audio.__nb_samples = 0;
        mParam.audio.__src_nb_samples = 0;

        mCtx.swr_ctx = swr_alloc();
        av_opt_set_int(mCtx.swr_ctx, "in_channel_layout", mCtx.a_codec_ctx->channel_layout, 0);
        av_opt_set_int(mCtx.swr_ctx, "in_sample_rate", mCtx.a_codec_ctx->sample_rate, 0);
        av_opt_set_sample_fmt(mCtx.swr_ctx, "in_sample_fmt", mCtx.a_codec_ctx->sample_fmt, 0);

        av_opt_set_int(mCtx.swr_ctx, "out_channel_layout", mParam.audio.ch_layout, 0);
        av_opt_set_int(mCtx.swr_ctx, "out_sample_rate", mParam.audio.rate, 0);
        av_opt_set_sample_fmt(mCtx.swr_ctx, "out_sample_fmt", mParam.audio.fmt, 0);

        swr_init(mCtx.swr_ctx);
        mCtx.a_frm = av_frame_alloc();

        emit audioInfo(mParam.audio.rate, mParam.audio.__nb_channels, (int)mParam.audio.fmt);
    }

    return true;
}

void DecodeThread::run()
{
//    if(!init(mFilename))
//    {
//        goto exit;
//    }

    AVPacket pkt;
    av_init_packet(&pkt);

    //解码
    while (!isInterruptionRequested() && av_read_frame(mCtx.fmt_ctx, &pkt) >= 0)
    {
        decodePacket(&mCtx, &pkt, &mParam);
    }

    //flush decoder
    pkt.data = nullptr;
    pkt.size = 0;
    decodePacket(&mCtx, &pkt, &mParam);

    freeAll();

    //qDebug() << "====DecodeThread exit.====";
}

void DecodeThread::freeAll()
{
    if(mCtx.v_frm != nullptr) av_frame_free(&mCtx.v_frm);
    if (mCtx.a_frm != nullptr) av_frame_free(&mCtx.a_frm);
    if (mCtx.a_codec_ctx != nullptr) avcodec_free_context(&mCtx.a_codec_ctx);
    if (mCtx.v_codec_ctx != nullptr)avcodec_free_context(&mCtx.v_codec_ctx);
    avformat_close_input(&mCtx.fmt_ctx);
}

bool DecodeThread::openInput(Context *ctx, const char *filename)
{
    ctx->hasAudio = false;
    ctx->hasVideo = false;
    int ret = avformat_open_input(&ctx->fmt_ctx, filename, nullptr, nullptr);
    if (ret < 0)
    {
        return false;
    }

    ret = avformat_find_stream_info(ctx->fmt_ctx, nullptr);
    if (ret < 0)
    {
        return false;
    }

    ctx->a_index = av_find_best_stream(ctx->fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (ctx->a_index >= 0)
    {
        ctx->a_codec = avcodec_find_decoder(ctx->fmt_ctx->streams[ctx->a_index]->codecpar->codec_id);
        if (ctx->a_codec != nullptr)
        {
            ctx->a_codec_ctx = avcodec_alloc_context3(ctx->a_codec);
            if (ctx->a_codec_ctx != nullptr)
            {
                ret = avcodec_parameters_to_context(ctx->a_codec_ctx, ctx->fmt_ctx->streams[ctx->a_index]->codecpar);
                if (ret >= 0)
                {
                    ret = avcodec_open2(ctx->a_codec_ctx, ctx->a_codec, nullptr);
                    ctx->hasAudio = (ret >= 0);
                }
            }
        }
    }

    ctx->v_index = av_find_best_stream(ctx->fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (ctx->v_index >= 0)
    {
        ctx->v_codec = avcodec_find_decoder(ctx->fmt_ctx->streams[ctx->v_index]->codecpar->codec_id);
        if (ctx->v_codec != nullptr)
        {
            ctx->v_codec_ctx = avcodec_alloc_context3(ctx->v_codec);
            if (ctx->v_codec_ctx != nullptr)
            {
                ret = avcodec_parameters_to_context(ctx->v_codec_ctx, ctx->fmt_ctx->streams[ctx->v_index]->codecpar);
                if (ret >= 0)
                {
                    ret = avcodec_open2(ctx->v_codec_ctx, ctx->v_codec, nullptr);
                    ctx->hasVideo = (ret >= 0);
                }
            }
        }
    }

    return true;
}

void DecodeThread::decodePacket(Context *ctx, AVPacket *pkt, Param *param)
{
    AVFrame *frm = av_frame_alloc();
    if(pkt->stream_index == ctx->a_index) //音频
    {
        decodeAudioPacket(ctx, pkt, frm, param);
    }
    else if(pkt->stream_index == ctx->v_index) //视频
    {
        decodeVideoPacket(ctx, pkt, frm, param);
    }

    av_frame_free(&frm);
}

void DecodeThread::decodeAudioPacket(Context *ctx, AVPacket *pkt, AVFrame *frm, Param *param)
{
    int ret = avcodec_send_packet(ctx->a_codec_ctx, pkt);
    if(ret < 0)
    {
        return;
    }
    while (ret >= 0)
    {
        if(isInterruptionRequested())
        {
            return;
        }

        ret = avcodec_receive_frame(ctx->a_codec_ctx, frm);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0)
        {
            break;
        }

        qint64 timestamp = (qint64)(frm->pts * av_q2d(ctx->fmt_ctx->streams[ctx->a_index]->time_base) * 1000); //ms
        //qDebug() << "Audio timestamp: " << timestamp << "ms";
        int data_size = av_get_bytes_per_sample(param->audio.fmt);
        if(data_size < 0)
        {
            break;
        }

        if(ctx->swr_ctx != nullptr)     //需要转换格式
        {
            if (param->audio.__src_nb_samples != frm->nb_samples)//改变时重新计算
            {
                param->audio.__src_nb_samples = frm->nb_samples;
                int dst_nb_samples = av_rescale_rnd(swr_get_delay(ctx->swr_ctx, ctx->a_codec_ctx->sample_rate) + frm->nb_samples,
                    param->audio.rate, ctx->a_codec_ctx->sample_rate, AV_ROUND_UP);
                if(dst_nb_samples > param->audio.__nb_samples)
                {
                    if(ctx->a_frm->data[0] != nullptr)
                    {
                        av_freep(&ctx->a_frm->data[0]);
                    }

                    ret = av_samples_alloc(ctx->a_frm->data, ctx->a_frm->linesize, param->audio.__nb_channels, dst_nb_samples, param->audio.fmt, 1);
                    param->audio.__nb_samples = dst_nb_samples;
                }
            }

            //转换，返回转换后的nb_samples, 转换前的数据在frm中，转换后的数据在ctx->a_frm->data中
            ret = swr_convert(ctx->swr_ctx, ctx->a_frm->data, param->audio.__nb_samples, (const uint8_t**)frm->data, frm->nb_samples);
            if(ret < 0)
            {
                break;
            }

            QByteArray datagram;
            for (int i = 0; i < ret; i++) //nb_samples
            {
                for (int ch = 0; ch < param->audio.__nb_channels; ch++)
                {
                    datagram.append((const char *)(ctx->a_frm->data[ch] + data_size * i), data_size);
                }
            }

            AudioFrame af;
            af.timestamp = timestamp;
            af.length = datagram.length();
            af.data = (char *)malloc(af.length);
            memcpy(af.data, datagram.constData(), af.length);
            //mAudioCache->push(af);

            //qDebug() << "<- push audio timestamp=" << timestamp;
        }
        else    //不需要转换格式
        {
            QByteArray datagram;
            for (int i = 0; i < frm->nb_samples; i++)
            {
                for (int ch = 0; ch < ctx->a_codec_ctx->channels; ch++)
                {
                    datagram.append((const char *)(ctx->a_frm->data[ch] + data_size * i), data_size);
                }
            }

            AudioFrame af;
            af.timestamp = timestamp;
            af.length = datagram.length();
            af.data = (char *)malloc(af.length);
            memcpy(af.data, datagram.constData(), af.length);
            mAudioCache->push(af);
        }
    }
}

void DecodeThread::decodeVideoPacket(Context *ctx, AVPacket *pkt, AVFrame *frm, Param *param)
{
    int ret = avcodec_send_packet(ctx->v_codec_ctx, pkt);
    if (ret < 0)
    {
        return;
    }

    while (ret >= 0)
    {
        if(isInterruptionRequested())
        {
            return;
        }

        ret = avcodec_receive_frame(ctx->v_codec_ctx, frm);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0)
        {
            break;
        }

        qint64 timestamp = (qint64)(1000 * frm->pts * av_q2d(ctx->fmt_ctx->streams[ctx->v_index]->time_base));

        if(ctx->sws_ctx != nullptr)
        {
            //为yuv420p_frm中的data,linesize指针分配空间
            if(ctx->v_frm->data[0] == nullptr)
            {
                ret = av_image_alloc(ctx->v_frm->data, ctx->v_frm->linesize,
                    param->video.width, param->video.height, param->video.fmt, 1);
            }

            //转换
            ret = sws_scale(ctx->sws_ctx, frm->data, frm->linesize, 0, ctx->v_codec_ctx->height,
                ctx->v_frm->data, ctx->v_frm->linesize);

            if(ret == param->video.height)
            {
                QImage image(ctx->v_frm->data[0], param->video.width, param->video.height,
                        ctx->v_frm->linesize[0], QImage::Format_RGB888, nullptr, nullptr);

                VideoFrame vf;
                vf.timestamp = timestamp;
                vf.image = image;
                mVideoCache->push(vf);
                //qDebug() << "<- push video timestamp=" << timestamp;
            }
        }
        else
        {
            //不做转换，则可能会有填充得空白数据，比如688*384的视频会填充成768*384，data中每行结尾都有无效数据
            int ySize = frm->linesize[0] * ctx->v_codec_ctx->height;// ctx->v_codec_ctx->width * ctx->v_codec_ctx->height;
            int uSize = ySize / 4;
            int vSize = ySize / 4;
//                fwrite(frm->data[0], 1, ySize, fVideo);
//                fwrite(frm->data[1], 1, uSize, fVideo);
//                fwrite(frm->data[2], 1, vSize, fVideo);
        }
    }
}
