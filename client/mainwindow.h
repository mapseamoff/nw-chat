#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtGui>
#include "logindialog.h"
#include "ChatClient.h"

class MainWindow: public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();
private:
    QTextEdit * chatEdit;
    QTextEdit * messageEdit;
    QPushButton * sendButton;
    QPushButton * connectButton;

//    boost::mutex clientLock;
//    boost::condition_variable waitClient;
//    ChatClient * client;

private slots:
    void sendMessage();
    void update();
    void connectWindow();
};

#endif // MAINWINDOW_H
