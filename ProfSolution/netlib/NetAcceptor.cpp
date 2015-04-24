//
// Created by Cristian Marastoni on 24/04/15.
//

#include "NetAcceptor.h"
#include "NetReactor.h"
#include "../Socket.h"

net::Status Acceptor::listen() {
    if(mSocket.open() != net::Ok) {
        return net::Error;
    }
    mSocket.setBlocking(false);
    mSocket.setReuseAddr(true);
    if(mSocket.bind(mAddress.port) != net::Ok) {
        mSocket.close();
        return net::Error;
    }
    if(mSocket.listen(100) != net::Error) {
        mChannel = new Channel(mSocket.handle());
        mChannel->setReadCallback(std::bind(&Acceptor::Accept, this));
        mReactor->addChannel(mChannel);
        return net::Ok;
    }else {
        mSocket.close();
        return net::Error;
    }
}

void Acceptor::stop() {
    if(mChannel != NULL) {
        mReactor->removeChannel(mChannel);
        delete mChannel;
    }
    mSocket.close();
}

void Acceptor::Accept() {
    int count = 0;
    int MAX_ACCEPT = 100;
    net::socket_details::socket_t peerFd;
    IpAddress peerAddr;
    while(count < MAX_ACCEPT) {
        if(mSocket.accept(peerFd, peerAddr) == net::Ok) {
            if(mConnectionCallback) {
                mConnectionCallback(peerFd, peerAddr);
            }else {
                net::socket_details::close(peerFd);
            }
        }
        ++count;
    }
}