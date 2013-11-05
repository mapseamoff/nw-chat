#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <vector>
#include <algorithm>

#include <string.h>

class ChatMessage {
public:
    enum MessageType { Invalid, Login, Message, Fetch, List, Logout };

    struct MessageHeader {
        MessageHeader(int v, MessageType t, size_t l, bool z = false, bool s = false) :
            version(v), type(t), bodyLength(l), zip(z), sec(s) {}

        int version;
        MessageType type;
        size_t bodyLength;
        bool zip;
        bool sec;
    };

    ChatMessage(int v = 1, MessageType t = Invalid, size_t l = 0, bool z = false, bool s = false) : header(v, t, l, z, s) {}

    static ChatMessage createMessage(int v, ChatMessage::MessageType t, const std::string &content) {
        ChatMessage msg(v, t, content.size());
        msg.body.resize(content.size());
        std::copy(content.begin(), content.end(), msg.body.begin());
        return msg;
    }

    static std::vector<char> serialize(const ChatMessage &msg) {
        std::vector<char> buf(msg.body.size() + sizeof(msg.header));
        memcpy(buf.data(), &msg.header, sizeof(msg.header));
        memcpy(buf.data() + sizeof(msg.header), msg.body.data(), msg.body.size());
        return buf;
    }

    MessageHeader header;
    std::vector<char> body;
};

#endif
