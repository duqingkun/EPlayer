#ifndef CONTAINER_H
#define CONTAINER_H

#include <QList>
#include <QMutex>
#include <QSemaphore>
#include <QScopedPointer>

template<class T>
class Cache
{
public:
    Cache(quint64 maxSize = 100)
        : mMaxSize(maxSize)
    {
        mUsedSpace.reset(new QSemaphore(0));
        mFreeSpace.reset(new QSemaphore(mMaxSize));
    }

    void close()
    {
        mUsedSpace->release(1);
        mFreeSpace->release(1);
    }

    void push(T const& e)
    {
        mFreeSpace->acquire(1); //空闲的信号量-1
        mBuffer.push_back(e);
        mUsedSpace->release(1);  //已用信号量+1
    }

    T pop()
    {
        mUsedSpace->acquire(1);     //已用信号量-1
        T e;
        if(!mBuffer.isEmpty())
        {
            e= mBuffer.takeFirst();
        }
        mFreeSpace->release(1);     //空闲的信号量+1

        return e;
    }
private:
    QList<T> mBuffer;
    quint64 mMaxSize;
    QMutex mMutex;      //不可去

    QScopedPointer<QSemaphore> mUsedSpace;
    QScopedPointer<QSemaphore> mFreeSpace;
};

#endif // CONTAINER_H
