#include "chatclient.h"

ChatClient::ChatClient(QObject *parent) : QObject(parent) {
    sock = new QTcpSocket(this);
    connect(sock, SIGNAL(connected()), this, SLOT(handleConnect()));
    connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(handleError()));
    connect(sock, SIGNAL(bytesWritten(qint64)), this, SLOT(handleWrite(qint64)));
    consumers.push_back(new LoginConsumer(this));
    consumers.push_back(new MessageConsumer(this));
    consumers.push_back(new FetchConsumer(this));
    consumers.push_back(new ListConsumer(this));
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
    while(sock->bytesAvailable() >= sizeof(ChatMessage::header)) {
        ChatMessage reply;
        sock->read((char*)&reply.header, sizeof(reply.header));
        reply.body.resize(reply.header.bodyLength);
        sock->read(reply.body.data(), reply.body.size());
        processMessage(reply);
    }
}

void ChatClient::handleWrite(qint64 bw) {
    writeMsgQueue.pop_front();
    if(!writeMsgQueue.empty()) {
        std::vector<char> buf = ChatMessage::serialize(writeMsgQueue.front());
        sock->write(buf.data(), buf.size());
    }
}

void ChatClient::writeMessage(const ChatMessage &msg) {
    bool hasMsgs = !writeMsgQueue.empty();
    writeMsgQueue.push_back(msg);
    if(!hasMsgs) {
        std::vector<char> buf = ChatMessage::serialize(writeMsgQueue.front());
        sock->write(buf.data(), buf.size());
    }
}

void ChatClient::processMessage(const ChatMessage &msg) {
    for (std::vector<Consumer *>::iterator it = consumers.begin(); it != consumers.end(); ++it) {
        if ((*it)->handleMessage(msg)) {
            return;
        }
    }
    lastError = "Error: " + QString::fromStdString(std::string(msg.body.data(), msg.body.size()));
    emit protocolError();
}

