//
// Created by Cristian Marastoni on 24/04/15.
//

#ifndef NETWORK_NETCONNECTOR_H
#define NETWORK_NETCONNECTOR_H


#include <functional>
#include "../IpAddress.h"
#include "../Socket.h"

class NetReactor;
class Channel;

class NetConnector {
public:
    typedef std::function<void(net::socket_details::socket_t)> ConnectionCallback;

    NetConnector(NetReactor *reactor, IpAddress const & serverAddr);
    ~NetConnector();

    void setConnectedCallaback(const ConnectionCallback& callback) {
        mConnectionCallback = callback;
    }

    const IpAddress& serverAddress() const {
        return mServerAddress;
    }

    void connect();
    void stop();
private:
    void handleWrite();
    void handleError();
    void retry();
    enum State {
        BeginConnection,
        Connecting,
        Connected,
        Disconnected,
    };

    ConnectionCallback mConnectionCallback;
    NetReactor *mReactor;
    IpAddress mServerAddress;
    Channel *mChannel;
    State mState;
};


#endif //NETWORK_NETCONNECTOR_H
