#include "allocatevideothread.h"

#include <QDebug>

AllocateVideoThread::AllocateVideoThread(Cache<VideoFrame> *vc, QObject *parent)
    : QThread(parent)
    , mVideoCache(vc)
    , mTimestamp(0)
    , mTimestampUpdated(false)
{

}

AllocateVideoThread::~AllocateVideoThread()
{
    mVideoCache->close();
    requestInterruption();
    quit();
    wait();
}

void AllocateVideoThread::timestamp(quint64 timestamp)
{
  //  mMutex.lock();
    mTimestamp = timestamp;
  //  mTimestampUpdated = true;
  //  mMutex.unlock();
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
            //qDebug() << "  -> pop video timestamp=" << vf.timestamp;
            if(first)
            {
                first = false;
                firstTimestamp = vf.timestamp;
                mTimer.start();
            }

            qint64 sleep = 0;

           // mMutex.lock();
            if(mTimestampUpdated)
            {
                sleep = vf.timestamp - mTimestamp;
                mTimestampUpdated = false;
            }
            else
            {
                sleep =  vf.timestamp - firstTimestamp - mTimer.elapsed();
            }

          //  mMutex.unlock();

            //qDebug() << sleep;
            if(sleep > 0)
            {
                //msleep(sleep);
                usleep(sleep * 1000);
                emit video(vf.image);
            }
        }
    }
}
