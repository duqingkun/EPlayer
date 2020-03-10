#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>

#include "decodethread.h"

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

    DecodeThread mDecodeThread;
    QScopedPointer<PCMPlayer> mPcmPlayer;
};
#endif // MAINWINDOW_H
