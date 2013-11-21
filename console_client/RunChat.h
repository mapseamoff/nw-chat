#ifndef CONSOLECHAT_H
#define CONSOLECHAT_H

#include <QObject>
#include <QTimer>
#include <QCoreApplication>

#include <iostream>

#include "chatclient.h"

class RunChat: public QObject {
    Q_OBJECT

public:
    RunChat(QObject * p, QString host, QString port, QString username, size_t time): QObject(p), userName(username), msgTime(time) {
        client = new ChatClient(this);
        connect(client, SIGNAL(connectedToChat(bool)), this, SLOT(connectedToChat(bool)));
        connect(client, SIGNAL(loginResult(bool)), this, SLOT(startSpamming(bool)));
        connect(client, SIGNAL(protocolError()), this, SLOT(gotError()));
        connect(client, SIGNAL(socketError()), this, SLOT(gotError()));

        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(sendMessage()));

        std::cout << "Conneting..." << std::endl;
        client->connectToChat(host, port);
    }

private slots:
    void connectedToChat(bool res) {
        if(res) {
            std::cout << "Logging in.." << std::endl;
            client->loginUser(userName);
        } else {
            std::cout << "Unable to connect. Exiting..." << std::endl;
            qApp->exit();
        }
    }

    void startSpamming(bool res) {
        if(res) {
            std::cout << "OK" << std::endl;
            timer->start(msgTime * 1000);
        } else {
            std::cout << "Unable to login. Exiting..." << std::endl;
            qApp->exit();
        }
    }

    void gotError() {
        std::cout << "error: " << client->getLastError().toStdString() << std::endl;
        qApp->exit();
    }

    void sendMessage() {
       client->sendMessage("Hello from " + userName);
    }

private:
    QString userName;
    size_t msgTime;
    ChatClient * client;
    QTimer * timer;
};

#endif // CONSOLECHAT_H
