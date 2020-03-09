#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QAudioFormat>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QAudioFormat audioFmt;
    audioFmt.setSampleRate(44100);
    audioFmt.setChannelCount(2);
    audioFmt.setSampleSize(16);
    audioFmt.setCodec("audio/pcm");
    audioFmt.setSampleType(QAudioFormat::SampleType::SignedInt);
    audioFmt.setByteOrder(QAudioFormat::LittleEndian);

    mAudioOutput = new QAudioOutput(audioFmt);
    mAudioStream = mAudioOutput->start();

    connect(&mDecodeThread, &DecodeThread::frame, this, &MainWindow::showFrame);
    connect(&mDecodeThread, &DecodeThread::loaded, this, &MainWindow::playFrame);
    connect(&mDecodeThread, &DecodeThread::audio, this, &MainWindow::playAudio);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showFrame(QImage image)
{
    if(!image.isNull())
    {
        ui->lblImage->setPixmap(QPixmap::fromImage(image));
    }
}

void MainWindow::playFrame()
{
    mDecodeThread.start();
}

void MainWindow::playAudio(unsigned char *data, int length)
{
    mAudioStream->write((char *)data, length);
}


void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open"));
    if(!filename.isEmpty())
    {
        mDecodeThread.load(filename);
    }
}
