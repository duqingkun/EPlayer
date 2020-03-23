#include "allocateaudiothread.h"

#include <QDebug>

AllocateAudioThread::AllocateAudioThread(Cache<AudioFrame> *ac, QObject *parent)
    : QThread(parent)
    , mAudioCache(ac)
{

}

AllocateAudioThread::~AllocateAudioThread()
{
    mAudioCache->close();
    requestInterruption();
    quit();
    wait();
}

void AllocateAudioThread::run()
{
    first = true;
    //分配视频、音频数据
    while (!isInterruptionRequested())
    {
        AudioFrame af = mAudioCache->pop();
        if(af.length > 0)
        {
            if(first)
            {
                first = false;
                firstTimestamp = af.timestamp;
                mTimer.start();
            }
            qint64 sleep =  af.timestamp - firstTimestamp - mTimer.elapsed();
            if(sleep > 0)
            {
                msleep(sleep);
            }
            timestamp(af.timestamp);
            audio(af.data, af.length);
        }
    }
}
