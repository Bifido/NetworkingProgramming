//
// Created by Cristian Marastoni on 24/04/15.
//

#ifndef NETWORK_NETREACTOR_H
#define NETWORK_NETREACTOR_H

#include <vector>
#include <functional>
#include "../Socket.h"

//Server is implemented using Reactor (something very similar) pattern.

class Channel;

//Very Very simple reactor interface
class NetReactor {
public:
    virtual void addChannel(Channel *handle) = 0;
    virtual void removeChannel(Channel *handle) = 0;
    virtual void poll(int pollMs, std::vector<Channel*> activeChannels) = 0;
};


//abstract handle used by the poller to keep track of current active descriptors.
class Channel {
public:
    typedef typename std::function<void(void)> EventCallback;
    typedef int HandleType;

    explicit Channel(net::socket_details::socket_t socket) : mFd(socket) {
    }

    void setReadCallback(const EventCallback& cb)
    { mReadCbk = cb; }

    void setWriteCallback(const EventCallback& cb)
    { mWriteCbk = cb; }

    void setCloseCallback(const EventCallback& cb)
    { mCloseCbk = cb; }

    void setErrorCallback(const EventCallback& cb)
    { mErrorCbk = cb; }

    void setEvents(int events) {
        mEvents = events;
    }
    net::socket_details::socket_t fd() const {
        return mFd;
    }
    void HandleEvents();
private:
    EventCallback mReadCbk;
    EventCallback mWriteCbk;
    EventCallback mCloseCbk;
    EventCallback mErrorCbk;
    int mEvents;
    net::socket_details::socket_t mFd;
};


class PollReactor : public NetReactor{
public:
    PollReactor();
    ~PollReactor();
    void poll(int pollMs, std::vector<Channel*> activeChannels);
    void addChannel(Channel *handle);
    void removeChannel(Channel *handle);
private:
    //sorted array
    typedef std::vector<Channel*> HandleList;
private:
    HandleList::iterator findHandle(int fd);
    struct LessPred {
        bool operator()(Channel const *ref, int value) const {
            return ref->fd() < value;
        }
        bool operator()(int value, Channel const *ref) const {
            return value < ref->fd();
        }
    };
private:
    HandleList mHandlesMap;;
};

#endif //NETWORK_NETREACTOR_H
