#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>

#include "datastructure.h"
#include "cache.h"
#include "decodethread.h"
#include "allocateaudiothread.h"
#include "allocatevideothread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class PCMPlayer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void setAudioParams(int sampleRate, int channels, int fmt);

    void setVideoParams(int w, int h);

    void showFrame(QImage image);

    void play();

    void playAudio(const char *data, int length);

    void on_actionOpen_triggered();

private:
    Ui::MainWindow *ui;

    QScopedPointer<Cache<AudioFrame>> mAudioCache;
    QScopedPointer<Cache<VideoFrame>> mVideoCache;
    QScopedPointer<DecodeThread> mDecodeThread;
    QScopedPointer<AllocateAudioThread> mAllocateAudioThread;
    QScopedPointer<AllocateVideoThread> mAllocateVideoThread;
    QScopedPointer<PCMPlayer> mPcmPlayer;
};
#endif // MAINWINDOW_H
