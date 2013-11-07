#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>

#include "logindialog.h"
#include "chatclient.h"

class MainWindow: public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QTextEdit * chatEdit;
    QTextEdit * messageEdit;
    QPushButton * sendButton;
    QPushButton * connectButton;

    ChatClient *client;
    LoginDialog *loginDialog;
    QString userName, lastMessage;

private slots:
    void sendMessage();
    void showLoginDialog();
    void disconnectFromChat();
    void connectedToChat(bool ok);
    void loggedIn(bool ok);
    void socketError();
    void protocolError();
    void newMessage(QString msg);
};

#endif // MAINWINDOW_H
