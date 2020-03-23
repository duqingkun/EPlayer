#ifndef ALLOCATEVIDEOTHREAD_H
#define ALLOCATEVIDEOTHREAD_H

#include <QThread>
#include <QElapsedTimer>
#include <QMutex>

#include "cache.h"
#include "datastructure.h"

class AllocateVideoThread : public QThread
{
    Q_OBJECT
public:
    AllocateVideoThread(Cache<VideoFrame> *vc, QObject *parent = nullptr);
    ~AllocateVideoThread();

public slots:
    void timestamp(quint64 timestamp);

signals:
    void video(QImage image);

protected:
    void run() override;

private:
    Cache<VideoFrame> *mVideoCache;

    QElapsedTimer mTimer;
    QMutex mMutex;

    bool first;
    quint64 firstTimestamp;
    quint64 mTimestamp;
    bool mTimestampUpdated;
};

#endif // ALLOCATEVIDEOTHREAD_H
