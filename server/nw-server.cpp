#include <iostream>
#include <set>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

#include "chatmessage.h"

using boost::asio::ip::tcp;

class ChatServer {
    class ChatSession : public boost::enable_shared_from_this<ChatSession> {
    public:
        ChatSession(ChatServer *srv) : server(srv), sock(srv->_io_service) {
        }

        ~ChatSession() {
            std::cout << "Session closed: " << (isLoggedIn() ? userName : "{not logged in}") << std::endl;
            sock.close();
        }

        tcp::socket &socket() {
            return sock;
        }

        bool isLoggedIn() const {
            return !userName.empty();
        }

        void waitForMessage() {
            boost::asio::async_read(sock, boost::asio::buffer(&inMessage.header, sizeof(inMessage.header)),
                                    boost::bind(&ChatSession::handleReadMessageHeader, this, boost::asio::placeholders::error));
        }

        void handleReadMessageHeader(const boost::system::error_code& error) {
            if(!error) {
                inMessage.body.resize(inMessage.header.bodyLength);
                boost::asio::async_read(sock, boost::asio::buffer(inMessage.body.data(), inMessage.body.size()),
                                        boost::bind(&ChatSession::handleReadMessageBody, this, boost::asio::placeholders::error));
            } else {
                server->closeSession(shared_from_this());
            }
        }

        void handleReadMessageBody(const boost::system::error_code& error) {
            if(!error) {
                //process message
                boost::posix_time::ptime processBegin = boost::posix_time::microsec_clock::universal_time();
                server->processMessage(this, inMessage);
                size_t processTime = (size_t)(boost::posix_time::microsec_clock::universal_time() - processBegin).total_microseconds();
//                std::cout << "Total users: " << (server->sessions.size() - 1) << " request time: " << processTime << "us" << std::endl;
                server->updateClientProcessTime(processTime);
                waitForMessage();
            } else {
                server->closeSession(shared_from_this());
            }
        }

        void sendReply(const ChatMessage &msg) {
            std::vector<char> buf = ChatMessage::serialize(msg);
            boost::asio::async_write(sock, boost::asio::buffer(buf.data(), buf.size()), boost::bind(&ChatSession::handleWrite, this, boost::asio::placeholders::error));
        }

        void handleWrite(const boost::system::error_code& error) {
            if(error) {
                std::cout << "Unable to send message to user '" << (userName.empty() ? "{not logged in}" : userName) << "': " << error.message() << std::endl;
                server->closeSession(shared_from_this());
            }
        }

        std::string userName;

    private:
        ChatServer *server;
        tcp::socket sock;
        ChatMessage inMessage;
    };

    //----------------------------------------------------------------------------------------------------

    class MessageHandler {
    public:
        MessageHandler(ChatServer *srv) : server(srv) {}

        virtual bool process(ChatSession *session, const ChatMessage &in, ChatMessage &out) = 0;

    protected:
        ChatServer *server;
    };

    class LoginMessageHandler : public MessageHandler {
    public:
        LoginMessageHandler(ChatServer *srv) : MessageHandler(srv) {}

        static const int version = 1;

        bool process(ChatSession *session, const ChatMessage &in, ChatMessage &out) {
            if(in.header.type != ChatMessage::Login || in.header.version != version) return false;
            std::string userName = std::string(in.body.data(), in.body.size());

            for(std::set<session_ptr>::iterator i = server->sessions.begin(); i != server->sessions.end(); ++i) {
                if(i->get()->userName == userName) {
                    out = ChatMessage::createMessage(version, ChatMessage::Login, "reject");
                    return true;
                }
            }
            out = ChatMessage::createMessage(version, ChatMessage::Login, boost::lexical_cast<std::string>(server->messages.size()));
            session->userName = userName;

//            std::cout << "New user: " << userName << std::endl;

            server->showClientPorcessTime();

            return true;
        }
    };

    class SendMessageHandler : public MessageHandler {
    public:
        SendMessageHandler(ChatServer *srv) : MessageHandler(srv) {}

        static const int version = 1;

        bool process(ChatSession *session, const ChatMessage &in, ChatMessage &out) {
            if(in.header.type != ChatMessage::Message || in.header.version != version) return false;
            if(!session->isLoggedIn()) {
                out = ChatMessage::createMessage(1, ChatMessage::Invalid, "not logged in");
                return true;
            }
            server->messages.push_back("[" + session->userName + "] " + std::string(in.body.data(), in.header.bodyLength));
//            std::cout << server->messages.back() << std::endl;
            out = ChatMessage::createMessage(version, ChatMessage::Message, boost::lexical_cast<std::string>(server->messages.size()));
            return true;
        }
    };

    class FetchMessageHandler : public MessageHandler {
    public:
        FetchMessageHandler(ChatServer *srv) : MessageHandler(srv) {}

        static const int version = 1;

        bool process(ChatSession *session, const ChatMessage &in, ChatMessage &out) {
            if(in.header.type != ChatMessage::Fetch || in.header.version != version) return false;
            if(!session->isLoggedIn()) {
                out = ChatMessage::createMessage(1, ChatMessage::Invalid, "not logged in");
                return true;
            }
            size_t lastId = -1;
            try {
                lastId = boost::lexical_cast<size_t>(std::string(in.body.data(), in.body.size()));
            } catch(...) {
                std::cout << "unable to get lastId" << std::endl;
                out = ChatMessage::createMessage(1, ChatMessage::Invalid, "bad arguments");
                return true;
            }
            if(lastId > server->messages.size()) {
                std::cout << "got last id: " << lastId << "max: " << server->messages.size() << std::endl;
                out = ChatMessage::createMessage(1, ChatMessage::Invalid, "bad arguments");
                return true;
            }

            out = ChatMessage(version, ChatMessage::Fetch);
            out.body.resize(sizeof(size_t));
            size_t msize = server->messages.size();
            memcpy(out.body.data(), &msize, sizeof(size_t));
            for(std::vector<std::string>::iterator m = server->messages.begin() + lastId; m != server->messages.end(); ++m) {
                size_t oldSize = out.body.size();
                out.body.resize(oldSize + m->size() + 1);
                std::copy(m->begin(), m->end(), out.body.begin() + oldSize);
                out.body.back() = '\n';
            }
            out.body.pop_back();
            out.header.bodyLength = out.body.size();
            return true;
        }
    };

    class UserListMessageHandler : public MessageHandler {
    public:
        UserListMessageHandler(ChatServer *srv) : MessageHandler(srv) {}

        static const int version = 1;

        bool process(ChatSession *session, const ChatMessage &in, ChatMessage &out) {
            if(in.header.type != ChatMessage::List || in.header.version != version) return false;
            if(!session->isLoggedIn()) {
                out = ChatMessage::createMessage(1, ChatMessage::Invalid, "not logged in");
                return true;
            }

            std::string userList;
            for(std::set<session_ptr>::iterator i = server->sessions.begin(); i != server->sessions.end(); ++i) {
                if(!i->get()->userName.empty()) userList += i->get()->userName + "\n";
            }
            userList.erase(userList.end() - 1);

            out = ChatMessage::createMessage(version, ChatMessage::List, userList);
            return true;
        }
    };

    class LogoutMessageHandler : public MessageHandler {
    public:
        LogoutMessageHandler(ChatServer *srv) : MessageHandler(srv) {}

        static const int version = 1;

        bool process(ChatSession *session, const ChatMessage &in, ChatMessage &out) {
            if(in.header.type != ChatMessage::Logout || in.header.version != version) return false;
//            server->closeSession(session->shared_from_this());
            return true;
        }
    };

    //----------------------------------------------------------------------------------------------------

    typedef boost::shared_ptr<tcp::socket> socket_ptr;
    typedef boost::shared_ptr<ChatSession> session_ptr;

public:
    ChatServer(boost::asio::io_service &io_service, unsigned short port) : _io_service(io_service), _acceptor(_io_service, tcp::endpoint(tcp::v4(), port)) {
        clientProcessTime = 0;
        requestCount = 0;
        handlers.push_back(new LoginMessageHandler(this));
        handlers.push_back(new SendMessageHandler(this));
        handlers.push_back(new FetchMessageHandler(this));
        handlers.push_back(new UserListMessageHandler(this));
        handlers.push_back(new LogoutMessageHandler(this));
        waitForConnection();
    }

    void waitForConnection() {
        session_ptr session(new ChatSession(this));
        sessions.insert(session);
        _acceptor.async_accept(session->socket(), boost::bind(&ChatServer::handleAccept, this, session, boost::asio::placeholders::error));
    }

    void handleAccept(session_ptr session, const boost::system::error_code &error) {
        if(!error) session->waitForMessage();
        waitForConnection();
    }

    void processMessage(ChatSession *s, const ChatMessage &msg) {
        ChatMessage reply = ChatMessage::createMessage(1, ChatMessage::Invalid, "Unsupported message type");
        for(std::vector<MessageHandler*>::iterator h = handlers.begin(); h != handlers.end(); ++h) {
            if((*h)->process(s, msg, reply)) break;
        }
        if(s) s->sendReply(reply);
    }

    void closeSession(session_ptr s) {
        std::set<session_ptr>::iterator si = sessions.find(s);
        if(si != sessions.end()) sessions.erase(si);
    }

private:
    void showClientPorcessTime() {
        boost::mutex::scoped_lock lock(lockCounters);
        if(requestCount > 0) {
            std::cout << "Users: " << (sessions.size() - 2) << " | mean request time: " << clientProcessTime / requestCount << " us" << std::endl;
            clientProcessTime = 0;
            requestCount = 0;
        }
    }

    void updateClientProcessTime(size_t time) {
        boost::mutex::scoped_lock lock(lockCounters);
//        std::cout << "Request process time: " << time << " us" << std::endl;
        clientProcessTime += time;
        requestCount++;
//        if(requestCount == 20) {
//        clientProcessTime = (clientProcessTime * requestCount + time) / (requestCount + 1);
//        ++requestCount;
//        }
    }

    boost::asio::io_service &_io_service;
    boost::mutex lockCounters;
    tcp::acceptor _acceptor;
    std::set<session_ptr> sessions;
    std::vector<MessageHandler*> handlers;
    std::vector<std::string> messages;
    size_t clientProcessTime, requestCount;

};

int main(int argc, char *argv[]) {
    if(argc != 2) {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return 0;
    }

    boost::asio::io_service io_service;
    ChatServer server(io_service, boost::lexical_cast<unsigned short>(argv[1]));

    io_service.run();
    
    return 0;
}
