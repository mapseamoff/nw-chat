#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <boost/lexical_cast.hpp>

#include <QObject>
#include <QTcpSocket>
#include <QQueue>
#include <QDebug>
#include <QMutex>

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
        if(loggedIn) {
//            QMutexLocker lock(&idMutex);
            writeMessage(ChatMessage::createMessage(1, ChatMessage::Fetch, boost::lexical_cast<std::string>(lastId)));
//            qDebug() << lastId;
        }
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
    void gotUserList(QString);

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
    QQueue<ChatMessage> writeMsgQueue;
    bool loggedIn;
    size_t lastId;
    QMutex idMutex;


    class Consumer {
    public:
        Consumer(ChatClient *client): client(client) {}
        virtual bool handleMessage(const ChatMessage& msg) = 0;
    protected:
        ChatClient *client;
    };

    class LoginConsumer: public Consumer {
    public:
        LoginConsumer(ChatClient *client): Consumer(client) {}

        virtual bool handleMessage(const ChatMessage &msg) {
            if (msg.header.type != ChatMessage::Login) {
                return false;
            }
            try {
                std::string replyText(msg.body.data(), msg.body.size());
                QMutexLocker lock(&client->idMutex);
                client->lastId = boost::lexical_cast<size_t>(replyText);
                client->loggedIn = true;
                emit client->loginResult(true);
            } catch(...) {
                client->lastError = "server rejects connection";
                emit client->loginResult(false);
            }
            return true;
        }
    };
    class MessageConsumer: public Consumer {
    public:
        MessageConsumer(ChatClient *client): Consumer(client) {}
        virtual bool handleMessage(const ChatMessage &msg) {
            if (msg.header.type != ChatMessage::Message) {
                return false;
            }
            try {
                std::string replyText(msg.body.data(), msg.body.size());
                size_t id = boost::lexical_cast<size_t>(replyText);
//                QMutexLocker lock(&client->idMutex);
                client->idMutex.lock();
                if(client->lastId + 1 != id) {
                    client->idMutex.unlock();
                    client->fetchMessages();
                } else {
                    ++client->lastId;
                    client->idMutex.unlock();
                    emit  client->gotMessage(QString());
                }
            } catch(...) {
                client->lastError = "unable to parse reply";
                emit client->protocolError();
            }
            return true;
        }
    };

    class FetchConsumer: public Consumer {
    public:
        FetchConsumer(ChatClient *client): Consumer(client) {}
        virtual bool handleMessage(const ChatMessage &msg) {
            if (msg.header.type != ChatMessage::Fetch) {
                return false;
            }


            {
                QMutexLocker lock(&client->idMutex);
                memcpy(&client->lastId, msg.body.data(), sizeof(size_t));
            }
            qint64 ln = msg.body.size() - sizeof(size_t);
            if(ln > 0) emit client->gotMessage(QString::fromStdString(std::string(msg.body.data() + sizeof(size_t), ln)));
            return true;
        }
    };

    class ListConsumer: public Consumer {
    public:
        ListConsumer(ChatClient *client): Consumer(client) {}
        virtual bool handleMessage(const ChatMessage &msg) {
            if (msg.header.type != ChatMessage::List) {
                return false;
            }
            emit client->gotUserList(QString::fromStdString(std::string(msg.body.data(), msg.body.size())));
            return true;
        }
    };
    std::vector<Consumer *> consumers;
};

#endif // CHATCLIENT_H
