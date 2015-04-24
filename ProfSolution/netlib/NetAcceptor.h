//
// Created by Cristian Marastoni on 24/04/15.
//

#ifndef NETWORK_ACCEPTOR_H
#define NETWORK_ACCEPTOR_H


#include <functional>
#include "../Socket.h"

class NetReactor;
class Channel;

class Acceptor {
public:
    typedef std::function<void(net::socket_details::socket_t, IpAddress const &)> NewConnectionCallback;
    Acceptor(NetReactor *reactor, IpAddress const &addr) : mAddress(addr), mReactor(reactor)
    {
    }
    ~Acceptor() {
        stop();
    }
    void setNewConnectionCallback(const NewConnectionCallback& callback) {
        mConnectionCallback = callback;
    }
    net::Status listen();
    void stop();

private:
    void Accept();
    net::TcpSocket mSocket;
    IpAddress mAddress;
    NetReactor *mReactor;
    Channel   *mChannel;
    NewConnectionCallback mConnectionCallback;
};


#endif //NETWORK_ACCEPTOR_H
