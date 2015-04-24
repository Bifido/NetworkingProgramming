//
// Created by Cristian Marastoni on 24/04/15.
//

#include "NetReactor.h"
#include <sys/poll.h>
#include <sys/fcntl.h>
#include <assert.h>

void Channel::HandleEvents() {
    if(mEvents == 0) {
        return;
    }
    if((mEvents & POLLHUP) != 0 && (mEvents & POLLIN) == 0) {
        if(mCloseCbk) {
            mCloseCbk();
        }
    }
    if((mEvents & (POLLNVAL | POLLERR)) != 0) {
        if(mErrorCbk) {
            mErrorCbk();
        }
    }
    if((mEvents & (POLLIN | POLLPRI)) != 0) {
        if(mReadCbk) {
            mReadCbk();
        }
    }
    if((mEvents & (POLLOUT)) != 0) {
        if(mWriteCbk) {
            mWriteCbk();
        }
    }
}

PollReactor::PollReactor()
{
}
PollReactor::~PollReactor() {

}

void PollReactor::poll(int pollMs, std::vector<Channel*> activeChannels) {
    pollfd *desc = reinterpret_cast<pollfd*>(alloca(sizeof(pollfd) * mHandlesMap.size()));
    for(int i=0;i<mHandlesMap.size();++i) {
        desc[i].fd = mHandlesMap[i]->fd();
        desc[i].revents = 0;
        desc[i].events = POLLPRI | POLLIN | POLLOUT;
        mHandlesMap[i]->setEvents(0);
    }
    int n = ::poll(desc, (int)mHandlesMap.size(), pollMs);
    if(n > 0) {
        for(int i=0; i<mHandlesMap.size() && n > 0; ++i) {
            if(desc[i].revents != 0) {
                activeChannels.push_back(mHandlesMap[i]);
                --n;
            }
        }
    }
}

void PollReactor::addChannel(Channel *channel) {
    HandleList::iterator it = findHandle(channel->fd());
    if(it != mHandlesMap.end()) {
        mHandlesMap.insert(it, channel);
    }
}

void PollReactor::removeChannel(Channel *channel) {
    HandleList::iterator it = findHandle(channel->fd());
    if(it != mHandlesMap.end()) {
        mHandlesMap.erase(it);
    }
}
PollReactor::HandleList::iterator PollReactor::findHandle(int fd) {
    LessPred comp;
    HandleList::iterator it = std::lower_bound(mHandlesMap.begin(), mHandlesMap.end(), fd, comp);
    if(it != mHandlesMap.end() && !comp(fd, *it)) {
        return it;
    }
    return mHandlesMap.end();
}