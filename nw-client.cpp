#include <iostream>
#include <queue>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/lexical_cast.hpp>

#include "chatmessage.h"

using boost::asio::ip::tcp;

class ChatClient {
public:
    ChatClient(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator, boost::condition_variable &connectLock) :
        _io_service(io_service), sock(io_service), connectLock(connectLock), loggedIn(false) {
        lastId = 0;
        sock.async_connect(*endpoint_iterator, boost::bind(&ChatClient::handleConnect, this, boost::asio::placeholders::error));
    }

private:

    //-- Connection ------------------------------------------------------------

    void handleConnect(const boost::system::error_code& error) {
        if(error) {
            lastError = error.message();
            sock.close();
        } else waitForMessage();
        connectLock.notify_all();
    }

    //-- Write message ----------------------------------------------------------

    void postMessage(const ChatMessage &msg) {
        _io_service.post(boost::bind(&ChatClient::writeMessage, this, msg));
    }

    void writeMessage(const ChatMessage &msg) {
        bool hasMsgs = !writeMsgQueue.empty();
        writeMsgQueue.push(msg);
        if(!hasMsgs) {
            std::vector<char> buf = ChatMessage::serialize(writeMsgQueue.front());
            boost::asio::async_write(sock, boost::asio::buffer(buf.data(), buf.size()), boost::bind(&ChatClient::handleWrite, this, boost::asio::placeholders::error));
        } else {
            std::cout << "message enqueued" << std::endl;
        }
    }

    void handleWrite(const boost::system::error_code& error) {
        if(!error) {
            writeMsgQueue.pop();
            if(!writeMsgQueue.empty()) {
                std::vector<char> buf = ChatMessage::serialize(writeMsgQueue.front());
                boost::asio::async_write(sock, boost::asio::buffer(buf.data(), buf.size()), boost::bind(&ChatClient::handleWrite, this, boost::asio::placeholders::error));
            }
        } else {
            std::cout << "Unable to send message: " << error.message() << std::endl;
        }
    }

    //-- Read message ----------------------------------------------------------

    void waitForMessage() {
        boost::asio::async_read(sock, boost::asio::buffer(&reply.header, sizeof(reply.header)),
                                boost::bind(&ChatClient::handleReadMessageHeader, this, boost::asio::placeholders::error));
    }

    void handleReadMessageHeader(const boost::system::error_code& error) {
        if(!error) {
            reply.body.resize(reply.header.bodyLength);
            boost::asio::async_read(sock, boost::asio::buffer(reply.body.data(), reply.body.size()),
                                    boost::bind(&ChatClient::handleReadMessageBody, this, boost::asio::placeholders::error));
        } else {
            //connection lost - stop client
            std::cout << "Unable to read message header: " << error.message() << std::endl;
        }
    }

    void handleReadMessageBody(const boost::system::error_code& error) {
        if(!error) {
            std::string replyText(reply.body.data(), reply.body.size());
            switch(reply.header.type) {
            case ChatMessage::Login: {
                try {
                    lastId = boost::lexical_cast<size_t>(replyText);
                    std::cout << "logged in" << std::endl;
                    loggedIn = true;
                } catch(...) {
                    lastError = "server rejects connection";
                }
                connectLock.notify_all();
                break;
            }
            case ChatMessage::Message: {
                try {
                    size_t id = boost::lexical_cast<size_t>(replyText);
                    if(lastId + 1 != id) {
                        fetchMessages();
                    } else {
                        ++lastId;
                        std::cout << "<print last user message here>" << std::endl;
                    }
                } catch(...) {
                    lastError = "unable to parse reply";
                }
                break;
            }
            case ChatMessage::Fetch:
                memcpy(&lastId, reply.body.data(), sizeof(size_t));
                std::cout << std::string(reply.body.data() + sizeof(size_t), reply.body.size() - sizeof(size_t)) << std::endl;
                break;
            case ChatMessage::List:
                std::cout << replyText << std::endl;
                break;
            default:
                std::cout << "Error: " << replyText << std::endl;
            }
            waitForMessage();
        } else {
            //connection lost - stop client
            std::cout << "Unable to read message: " << error.message() << std::endl;
        }
    }

public:
    bool connected() const {
        return sock.is_open();
    }

    bool isLoggedIn() const {
        return loggedIn;
    }

    std::string getLastError() const {
        return lastError;
    }

    void loginUser(const std::string &name) {
        postMessage(ChatMessage::createMessage(1, ChatMessage::Login, name));
    }

    void sendMessage(const std::string &msg) {
        if(loggedIn) postMessage(ChatMessage::createMessage(1, ChatMessage::Message, msg));
    }

    void fetchMessages() {
        if(loggedIn) postMessage(ChatMessage::createMessage(1, ChatMessage::Fetch, boost::lexical_cast<std::string>(lastId)));
    }

    void getUserList() {
        if(loggedIn) postMessage(ChatMessage::createMessage(1, ChatMessage::List, ""));
    }

    void logoutUser() {
        if(loggedIn) postMessage(ChatMessage::createMessage(1, ChatMessage::Logout, ""));
    }

private:
    boost::asio::io_service &_io_service;
    tcp::socket sock;
    size_t lastId;
    std::string lastError;
    ChatMessage reply;
    std::queue<ChatMessage> writeMsgQueue;
    boost::condition_variable &connectLock;
    bool loggedIn;
};

static boost::mutex clientLock;
static boost::condition_variable waitClient;

int main(int argc, char *argv[]) {
    if(argc != 3 && argc != 4) {
        std::cout << "Usage: " << argv[0] << " <host> <port> [username]" << std::endl;
        return 0;
    }

    boost::mutex::scoped_lock lock(clientLock);

    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(argv[1], argv[2]);
    tcp::resolver::iterator iterator = resolver.resolve(query);

    boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));

    ChatClient c(io_service, iterator, waitClient);
    waitClient.wait(lock);
    if(!c.connected()) {
        std::cout << "Unable to connect: " << c.getLastError() << std::endl;
        return 0;
    }

    std::string userName;
    if(argc != 4) {
        std::cout << "Your name: ";
        std::cout.flush();
        std::cin >> userName;
    } else {
        userName = argv[3];
    }

    c.loginUser(userName);
    waitClient.wait(lock);
    if(!c.isLoggedIn()) {
        std::cout << "Unable to login: " << c.getLastError() << std::endl;
        return 0;
    }

    std::string msg;
    while(true) {
        std::cin >> msg;
        if(msg == "/exit") {
            c.logoutUser();
            break;
        }
        else if(msg == "/list") {
            c.getUserList();
        } else {
            c.sendMessage(msg);
        }
    }

    return 0;
}
