#ifndef PCMPLAYER_H
#define PCMPLAYER_H

#include <QObject>
#include <QIODevice>
#include <QByteArray>
#include <QAudioOutput>
#include <QScopedPointer>
#include <QMutex>

class PCMDevice : public QIODevice
{
public:
    explicit PCMDevice(QObject *parent = nullptr);
    ~PCMDevice();

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
    qint64 bytesAvailable() const override;
private:
    QByteArray mBuffer;
    QMutex mutex;
};

class PCMPlayer : public QObject
{
    Q_OBJECT
public:
    explicit PCMPlayer(QObject *parent = nullptr);

    ///
    /// \brief 设置音频参数，需在播放前设置
    /// \param sampleRate: 采样率
    /// \param channels: 通道数
    /// \param bitSize: 编码精度[bit]
    /// \param type: 编码类型
    ///
    void setParams(int sampleRate, int channels, int bitSize, QAudioFormat::SampleType type);

    ///
    /// \brief 开始播放
    ///
    void start();

    ///
    /// \brief 停止播放
    ///
    void stop();

    ///
    /// \brief 填入音频数据
    /// \param data
    /// \param len
    ///
    void addData(const char *data, int len);

signals:

public slots:

private:
    QScopedPointer<PCMDevice> mDevice;
    QScopedPointer<QAudioOutput> mAudioOutput;
    QIODevice *mIODevice;
};

#endif // PCMPLAYER_H
