#ifndef ALLOCATEVIDEOTHREAD_H
#define ALLOCATEVIDEOTHREAD_H

#include <QThread>
#include <QElapsedTimer>

#include "cache.h"
#include "datastructure.h"

class AllocateVideoThread : public QThread
{
    Q_OBJECT
public:
    AllocateVideoThread(Cache<VideoFrame> *vc, QObject *parent = nullptr);
    ~AllocateVideoThread();

signals:
    void audio(const char *data, int length);

    void video(QImage image);

protected:
    void run() override;

private:
    Cache<VideoFrame> *mVideoCache;

    QElapsedTimer mTimer;

    bool first;
    quint64 firstTimestamp;
};

#endif // ALLOCATEVIDEOTHREAD_H
