#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <boost/lexical_cast.hpp>
#include <queue>
#include <sstream>

#include <QObject>
#include <QTcpSocket>

#include "../chatmessage.h"

class ChatClient : public QObject {
    Q_OBJECT

public:
    ChatClient(QObject *parent = 0);
    ~ChatClient();

    void connectToChat(const QString &host, const QString &port);

    bool connected() const {
        return sock->isOpen();
    }

    bool isLoggedIn() const {
        return loggedIn;
    }

    QString getLastError() const {
        return lastError;
    }

    void loginUser(const QString &name) {
        writeMessage(ChatMessage::createMessage(1, ChatMessage::Login, name.toStdString()));
    }

    void sendMessage(const QString &msg) {
        if(loggedIn) writeMessage(ChatMessage::createMessage(1, ChatMessage::Message, msg.toStdString()));
    }

    void fetchMessages() {
        if(loggedIn) writeMessage(ChatMessage::createMessage(1, ChatMessage::Fetch, boost::lexical_cast<std::string>(lastId)));
    }

    void getUserList() {
        if(loggedIn) writeMessage(ChatMessage::createMessage(1, ChatMessage::List, ""));
    }

    void logoutUser() {
        if(loggedIn) {
            writeMessage(ChatMessage::createMessage(1, ChatMessage::Logout, ""));
            sock->disconnectFromHost();
            loggedIn = false;
        }
    }

signals:
    void connectedToChat(bool);
    void loginResult(bool);
    void socketError();
    void protocolError();
    void gotMessage(QString);

private slots:
    void handleError();
    void handleConnect();
    void handleRead();
    void handleWrite(qint64 bw);

private:
    void writeMessage(const ChatMessage &msg);
    void processMessage(const ChatMessage &msg);

private:
    QTcpSocket *sock;
    QString lastError;
    ChatMessage reply;
    std::queue<ChatMessage> writeMsgQueue;
    bool loggedIn;
    size_t lastId;
};

#endif // CHATCLIENT_H
