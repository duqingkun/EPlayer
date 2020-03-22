#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

#include <QImage>

typedef struct _audioframe
{
    quint64 timestamp;
    char *data;
    int length = 0;
    _audioframe()
        : data(0)
        , length(0)
    {}
}AudioFrame;

typedef struct _videoframe
{
    quint64 timestamp;
    QImage image;
}VideoFrame;

#endif // DATASTRUCTURE_H
