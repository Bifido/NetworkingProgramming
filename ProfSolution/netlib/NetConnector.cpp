//
// Created by Cristian Marastoni on 24/04/15.
//

#include "NetConnector.h"
#include "NetReactor.h"

NetConnector::NetConnector(NetReactor *reactor, IpAddress const &serverAddr) :
        mServerAddress(serverAddr), mState(Disconnected), mReactor(reactor){

}

NetConnector::~NetConnector() {
    stop();
}

void NetConnector::connect() {
    mState = BeginConnection;
    //Create a socket and try connection
    net::TcpSocket socket;
    socket.open();
    socket.setBlocking(false);
    net::Status status = socket.connect(mServerAddress);
    if(status == net::WouldBlock || status == net::InProgress) {
        //Register to reactor, this is the expected behavior
        mChannel = new Channel(socket.handle());
        mChannel->setWriteCallback(std::bind(&NetConnector::handleWrite, this));
        mChannel->setErrorCallback(std::bind(&NetConnector::handleError, this));
        mReactor->addChannel(mChannel);
    }
}

void NetConnector::stop() {
    if(mState == Connecting)
    {
        mState = Disconnected;
        mReactor->removeChannel(mChannel);
        delete mChannel;
    }
}

void NetConnector::handleWrite() {
    if (mState == Connecting) {
        mReactor->removeChannel(mChannel);
        int err = net::socket_details::getError();
        if (err != 0) {
            //Ops.. something wrong
            mState = Disconnected;
            delete mChannel;
        }
        else {
            mState = Connected;
            if (mConnectionCallback) {
                mConnectionCallback(mChannel->fd());
            }
            delete mChannel;
        }
    }
}

void NetConnector::handleError() {
    if(mState == Connecting) {
        mState = Disconnected;
        mReactor->removeChannel(mChannel);
        delete mChannel;
    }
}

void NetConnector::retry() {

}
