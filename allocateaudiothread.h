#ifndef ALLOCATEAUDIOTHREAD_H
#define ALLOCATEAUDIOTHREAD_H

#include <QThread>
#include <QElapsedTimer>

#include "cache.h"
#include "datastructure.h"

class AllocateAudioThread : public QThread
{
    Q_OBJECT
public:
    AllocateAudioThread(Cache<AudioFrame> *ac, QObject *parent = nullptr);
    ~AllocateAudioThread();

signals:
    void audio(const char *data, int length);

    void video(QImage image);

protected:
    void run() override;

private:
    Cache<AudioFrame> *mAudioCache;
    QElapsedTimer mTimer;

    bool first;
    quint64 firstTimestamp;
};

#endif // ALLOCATEAUDIOTHREAD_H
