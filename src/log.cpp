#include "log.h"

Message MessageLogger::debug() const
{
    return Message(context, MessageType::MessageDebug);
}

Message MessageLogger::info() const
{
    return Message(context, MessageType::MessageInfo);
}

Message MessageLogger::warn() const
{
    return Message(context, MessageType::MessageWarn);
}

Message MessageLogger::error() const
{
    return Message(context, MessageType::MessageError);
}


