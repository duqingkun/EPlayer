#include "allocatevideothread.h"

#include <QDebug>

AllocateVideoThread::AllocateVideoThread(Cache<VideoFrame> *vc, QObject *parent)
    : QThread(parent)
    , mVideoCache(vc)
{

}

AllocateVideoThread::~AllocateVideoThread()
{
    mVideoCache->close();
    requestInterruption();
    quit();
    wait();
}

void AllocateVideoThread::run()
{
    first = true;
    //分配视频、音频数据
    while (!isInterruptionRequested())
    {
        VideoFrame vf = mVideoCache->pop();
        if(!vf.image.isNull())
        {
            qDebug() << "  -> pop video timestamp=" << vf.timestamp;
            if(first)
            {
                first = false;
                firstTimestamp = vf.timestamp;
                mTimer.start();
            }

            qint64 sleep =  vf.timestamp - firstTimestamp - mTimer.elapsed();
            if(sleep > 0)
            {
                msleep(sleep);
            }

            emit video(vf.image);
        }
    }
}
