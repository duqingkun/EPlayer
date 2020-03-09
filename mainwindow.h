#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudioOutput>

#include "decodethread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showFrame(QImage image);

    void playFrame();

    void playAudio(unsigned char *data, int length);

    void on_actionOpen_triggered();

private:
    Ui::MainWindow *ui;

    DecodeThread mDecodeThread;
    QAudioOutput *mAudioOutput;
    QIODevice *mAudioStream;
};
#endif // MAINWINDOW_H
