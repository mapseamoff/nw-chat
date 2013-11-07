#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QTimer>
#include <QListWidget>

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
    QListWidget * userList;

    ChatClient *client;
    QTimer *fetchTimer, *userListTimer;
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
    void updateUserList(QString list);
    void fetchMessages();
    void fetchUserList();
};

#endif // MAINWINDOW_H
