#include <QCoreApplication>
#include <boost/lexical_cast.hpp>
#include <QTimer>
#include <QObject>

#include "chatclient.h"

class RunChat: public QObject {
    Q_OBJECT
public:
    RunChat(QObject * p, QString host, QString port, QString username, size_t time): QObject(p) {
        ChatClient *client = new ChatClient(this);
        client->connectToChat(host, port);
        client->loginUser(username);
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(sendMessage()));
        timer->start(time * 1000);
    }
    virtual ~RunChat() {}
    void run() {
        while(true);
    }
private slots:
    void sendMessage();

private:
    ChatClient * client;
    QTimer * timer;
};

void ::RunChat::sendMessage()  {
    client->sendMessage("Hello!");
}

int main(int argc, char *argv[])
{
    if(argc != 5) {
        std::cout << "Usage: " << argv[0] << " <host> <port> <username> <time>" << std::endl;
        return 0;
    }

    QCoreApplication a(argc, argv);
    RunChat chat(&a, argv[1], argv[2], argv[3], boost::lexical_cast<size_t>(argv[4]));
    return a.exec();
}
