#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pcmplayer.h"

#include <QDebug>
#include <QFileDialog>
#include <QAudioFormat>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mPcmPlayer.reset(new PCMPlayer(this));

    QAudioFormat audioFmt;
    audioFmt.setSampleRate(44100);
    audioFmt.setChannelCount(2);
    audioFmt.setSampleSize(16);
    audioFmt.setCodec("audio/pcm");
    audioFmt.setSampleType(QAudioFormat::SampleType::SignedInt);
    audioFmt.setByteOrder(QAudioFormat::LittleEndian);

    connect(&mDecodeThread, &DecodeThread::frame, this, &MainWindow::showFrame);
    connect(&mDecodeThread, &DecodeThread::loaded, this, &MainWindow::play);
    connect(&mDecodeThread, &DecodeThread::audio, this, &MainWindow::playAudio);
    connect(&mDecodeThread, &DecodeThread::audioInfo, this, &MainWindow::setAudioParams);
    connect(&mDecodeThread, &DecodeThread::videoInfo, this, &MainWindow::setVideoParams);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setAudioParams(int sampleRate, int channels, int fmt)
{
    int bitSize = 16;
    QAudioFormat::SampleType type = QAudioFormat::SignedInt;
    switch (fmt)
    {
    case AVSampleFormat::AV_SAMPLE_FMT_U8P:
        bitSize = 8;
        type = QAudioFormat::UnSignedInt;
        break;
    case AVSampleFormat::AV_SAMPLE_FMT_S16P:
        bitSize = 16;
        type = QAudioFormat::SignedInt;
        break;
    case AVSampleFormat::AV_SAMPLE_FMT_S32P:
        bitSize = 32;
        type = QAudioFormat::SignedInt;
        break;
    case AVSampleFormat::AV_SAMPLE_FMT_FLTP:
        bitSize = 32;
        type = QAudioFormat::Float;
        break;
    }

    mPcmPlayer->setParams(sampleRate, channels, bitSize, type);
}

void MainWindow::setVideoParams(int w, int h)
{
    qDebug() << "Video Info: (" << w << "," << h << ")";
}

void MainWindow::showFrame(QImage image)
{
    if(!image.isNull())
    {
        ui->lblImage->setPixmap(QPixmap::fromImage(image));
    }
}

void MainWindow::play()
{
    mDecodeThread.start();
    mPcmPlayer->start();
}

void MainWindow::playAudio(const char *data, int length)
{
    mPcmPlayer->addData(data, length);
}

void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open"));
    if(!filename.isEmpty())
    {
        mDecodeThread.load(filename);
    }
}
