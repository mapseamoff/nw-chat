#include "chatclient.h"

ChatClient::ChatClient(QObject *parent) : QObject(parent) {
    sock = new QTcpSocket(this);
    connect(sock, SIGNAL(connected()), this, SLOT(handleConnect()));
    connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(handleError()));
    connect(sock, SIGNAL(bytesWritten(qint64)), this, SLOT(handleWrite(qint64)));
}

ChatClient::~ChatClient() {
    if(sock->isOpen()) sock->close();
}

void ChatClient::connectToChat(const QString &host, const QString &port) {
    loggedIn = false;
    lastId = 0;
    if(sock->isOpen()) sock->disconnectFromHost();
    sock->connectToHost(host, port.toInt());
}

void ChatClient::handleError() {
    lastError = sock->errorString();
    emit socketError();
}

void ChatClient::handleConnect() {
    emit connectedToChat(true);
    connect(sock, SIGNAL(readyRead()), this, SLOT(handleRead()));
}

void ChatClient::handleRead() {
    while(sock->bytesAvailable() >= sizeof(reply.header)) {
        sock->read((char*)&reply.header, sizeof(reply.header));
        reply.body.resize(reply.header.bodyLength);
        sock->read(reply.body.data(), reply.body.size());
        processMessage(reply);
    }
}

void ChatClient::handleWrite(qint64 bw) {
    writeMsgQueue.pop();
    if(!writeMsgQueue.empty()) {
        std::vector<char> buf = ChatMessage::serialize(writeMsgQueue.front());
        sock->write(buf.data(), buf.size());
    }
}

void ChatClient::writeMessage(const ChatMessage &msg) {
    bool hasMsgs = !writeMsgQueue.empty();
    writeMsgQueue.push(msg);
    if(!hasMsgs) {
        std::vector<char> buf = ChatMessage::serialize(writeMsgQueue.front());
        sock->write(buf.data(), buf.size());
    }
}

void ChatClient::processMessage(const ChatMessage &msg) {
    std::string replyText(msg.body.data(), msg.body.size());
    switch(msg.header.type) {
    case ChatMessage::Login: {
        try {
            lastId = boost::lexical_cast<size_t>(replyText);
            loggedIn = true;
            emit loginResult(true);
        } catch(...) {
            lastError = "server rejects connection";
            emit loginResult(false);
        }
        break;
    }
    case ChatMessage::Message: {
        try {
            size_t id = boost::lexical_cast<size_t>(replyText);
            if(lastId + 1 != id) {
                fetchMessages();
            } else {
                ++lastId;
                emit gotMessage(QString());
            }
        } catch(...) {
            lastError = "unable to parse reply";
            emit protocolError();
        }
        break;
    }
    case ChatMessage::Fetch:
        memcpy(&lastId, msg.body.data(), sizeof(size_t));
        emit gotMessage(QString::fromStdString(std::string(msg.body.data() + sizeof(size_t), msg.body.size() - sizeof(size_t))));
        break;
    case ChatMessage::List:
//        streamMessages << replyText << std::endl;
        break;
    default:
        lastError = "Error: " + QString::fromStdString(replyText);
        emit protocolError();
        break;
    }
}
