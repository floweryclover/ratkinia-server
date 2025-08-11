//
// Created by floweryclover on 2025-08-10.
//

#ifndef SESSIONRECEIVEBUFFERSVIEW_H
#define SESSIONRECEIVEBUFFERSVIEW_H

#include "Session.h"
#include <unordered_map>

class SessionReceiveBufferOnlyWrapper final
{
public:
    const uint32_t Context;

    explicit SessionReceiveBufferOnlyWrapper(Session& session)
        : Context{ session.Context },
          session_{ session }
    {
    }

    std::optional<Session::MessagePeekResult> TryPeek()
    {
        return session_.TryPeekMessage();
    }

    void Pop(const size_t size)
    {
        session_.Pop(size);
    }

private:
    Session& session_;
};

class SessionReceiveBuffersIterator final
{
public:
    using InnerIteratorType = std::unordered_map<uint32_t, Session>::iterator;
    using value_type = SessionReceiveBufferOnlyWrapper;

    explicit SessionReceiveBuffersIterator(InnerIteratorType innerIterator)
        : innerIterator_{ std::move(innerIterator) }
    {
    }

    SessionReceiveBuffersIterator& operator++()
    {
        ++innerIterator_;
        return *this;
    }

    bool operator==(const SessionReceiveBuffersIterator& other) const = default;

    bool operator!=(const SessionReceiveBuffersIterator& other) const = default;

    value_type operator*()
    {
        return SessionReceiveBufferOnlyWrapper{ innerIterator_->second };
    }

private:
    InnerIteratorType innerIterator_;
};

class SessionReceiveBuffersView final
{
public:
    explicit SessionReceiveBuffersView(std::unordered_map<uint32_t, Session>& sessions)
        : sessions_{ sessions }
    {
    }

    SessionReceiveBuffersIterator begin()
    {
        return SessionReceiveBuffersIterator{ sessions_.begin() };
    }

    SessionReceiveBuffersIterator end()
    {
        return SessionReceiveBuffersIterator{ sessions_.end() };
    }

private:
    std::unordered_map<uint32_t, Session>& sessions_;
};

#endif //SESSIONRECEIVEBUFFERSVIEW_H
