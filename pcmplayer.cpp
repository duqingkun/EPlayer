#include "pcmplayer.h"

#include <QDebug>
#include <QAudioDeviceInfo>

PCMDevice::PCMDevice(QObject *parent)
    : QIODevice(parent)
{
    open(ReadWrite);
}

PCMDevice::~PCMDevice()
{
    close();
}

qint64 PCMDevice::readData(char *data, qint64 maxlen)
{
    qint64 size = 0;
   // mutex.lock();
    if (!mBuffer.isEmpty())
    {
        size = qMin((qint64)mBuffer.size(), maxlen);
        memcpy(data, mBuffer.constData(), size);
        mBuffer.remove(0, size);
    }
   // mutex.unlock();
    qDebug() << "readData: " << size << " \tremain: " << mBuffer.size();
    return size;
}

qint64 PCMDevice::writeData(const char *data, qint64 len)
{
   // mutex.lock();
    mBuffer.append(data, len);
   // mutex.unlock();
    qDebug() << "writeData: " << len << " \tremain: " << mBuffer.size();
}

qint64 PCMDevice::bytesAvailable() const
{
    return mBuffer.size() + QIODevice::bytesAvailable();
}

PCMPlayer::PCMPlayer(QObject *parent)
    : QObject(parent)
{

}

void PCMPlayer::setParams(int sampleRate, int channels, int bitSize, QAudioFormat::SampleType type)
{
    QAudioDeviceInfo deviceInfo = QAudioDeviceInfo::defaultOutputDevice();

    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(channels);
    format.setSampleSize(bitSize);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(type);

    if (!deviceInfo.isFormatSupported(format))
    {
        format = deviceInfo.nearestFormat(format);
    }

    mAudioOutput.reset(new QAudioOutput(deviceInfo, format));
    //mDevice.reset(new PCMDevice(this));
}

void PCMPlayer::start()
{
//    if(mDevice.isNull())
//    {
//        mDevice.reset(new PCMDevice(this));
//    }
//    mAudioOutput->start(mDevice.data());
    mIODevice = mAudioOutput->start();
}

void PCMPlayer::stop()
{
    mAudioOutput->stop();
}

void PCMPlayer::addData(const char *data, int len)
{
    //mDevice->write(data, len);
    mIODevice->write(data, len);
}
