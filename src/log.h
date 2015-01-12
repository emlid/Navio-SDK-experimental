#ifndef LOG_H
#define LOG_H

#include "iostream"
#include "sstream"
#include "string"

enum MessageType {
    MessageDebug,
    MessageInfo,
    MessageWarn,
    MessageError
};

class MessageContext
{
public:
    MessageContext(int line, const char *file, const char *function)
        : _line(line), _file(file), _function(function) {}
    int _line;
    const char *_file;
    const char *_function;
};

class Message
{
    struct Stream {
        Stream(const MessageContext &ctx, MessageType type)
            : ref(1), space(true), ss(), context(ctx), type(type){}
        int ref;
        bool space;
        std::stringstream ss;
        MessageContext context;
        MessageType type;
    } *stream;

public:
    inline Message(const MessageContext &context, MessageType type) : stream(new Stream(context, type)) {}
    inline Message(const Message &o):stream(o.stream) { ++stream->ref; }
    inline Message &operator=(const Message &other);
    inline ~Message() {
        if (!--stream->ref) {
            std::cerr << stream->context._file << ":";
            std::cerr << stream->context._line << " ";
            std::cerr << stream->context._function << "(): ";
            std::cerr << stream->ss.str() << std::endl;
            delete stream;
        }
    }

    inline Message &space() { stream->space = true; stream->ss << ' '; return *this; }
    inline Message &nospace() { stream->space = false; return *this; }
    inline Message &maybeSpace() { if (stream->space) stream->ss << ' '; return *this; }

    bool autoInsertSpaces() const { return stream->space; }
    void setAutoInsertSpaces(bool b) { stream->space = b; }

    inline Message &operator<<(bool t) { stream->ss << (t ? "true" : "false"); return maybeSpace(); }
    inline Message &operator<<(char t) { stream->ss << t; return maybeSpace(); }
    inline Message &operator<<(signed short t) { stream->ss << t; return maybeSpace(); }
    inline Message &operator<<(unsigned short t) { stream->ss << t; return maybeSpace(); }
    inline Message &operator<<(signed int t) { stream->ss << t; return maybeSpace(); }
    inline Message &operator<<(unsigned int t) { stream->ss << t; return maybeSpace(); }
    inline Message &operator<<(signed long t) { stream->ss << t; return maybeSpace(); }
    inline Message &operator<<(unsigned long t) { stream->ss << t; return maybeSpace(); }
    inline Message &operator<<(unsigned long long t) { stream->ss << t; return maybeSpace(); }
    inline Message &operator<<(float t) { stream->ss << t; return maybeSpace(); }
    inline Message &operator<<(double t) { stream->ss << t; return maybeSpace(); }
    inline Message &operator<<(const char* t) { stream->ss << t; return maybeSpace(); }
    inline Message &operator<<(const void * t) { stream->ss << t; return maybeSpace(); }
};

class MessageLogger
{
public:
    MessageLogger(int line, const char *file, const char *function): context(line, file, function) {}
    MessageLogger(const MessageLogger& that) = delete;

    Message debug() const;
    Message info() const;
    Message warn() const;
    Message error() const;

private:
    MessageContext context;
};

#define Debug MessageLogger(__LINE__, __FILE__, __FUNCTION__).debug
#define Info  MessageLogger(__LINE__, __FILE__, __FUNCTION__).info
#define Warn  MessageLogger(__LINE__, __FILE__, __FUNCTION__).warn
#define Error MessageLogger(__LINE__, __FILE__, __FUNCTION__).error

#endif
