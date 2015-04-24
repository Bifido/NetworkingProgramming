//
// Created by Cristian Marastoni on 23/04/15.
//

#ifndef NETWORK_SOCKET_H
#define NETWORK_SOCKET_H
#include "SocketDetails.h"
#include "IpAddress.h"

namespace net {
    class SocketBase {
    public:
        SocketBase() : mHandle(socket_details::invalid_socket()), mLastError(0) {
        }

        //Move semantic make sense in that case
        SocketBase(SocketBase &&ref) : mHandle(ref.mHandle), mLastError(0) {
            ref.mHandle = socket_details::invalid_socket();
        }

        virtual Status open() = 0;
        virtual Status open(socket_details::socket_t handle);
        virtual void close();

        inline socket_details::socket_t handle() {
            return mHandle;
        }
        inline bool isOpen() const {
            return mHandle != socket_details::invalid_socket();
        }

        int getLastErrorCode() const {
            return mLastError;
        }

        void setReuseAddr(bool reuse);
        void setReusePort(bool reuse);
        void setBlocking(bool block);

        IpAddress getLocalAddr() const;
        Status bind(unsigned short port);

    protected:
        inline Status getLastError() {
            mLastError = socket_details::getError();
            return socket_details::translateError(mLastError);
        }

        SocketBase(SocketBase const &ref) { }

        SocketBase &operator=(SocketBase const &ref) {
            return *this;
        }

        virtual ~SocketBase() = 0;

        socket_details::socket_t mHandle;
        //Stored for later usage
        int mLastError;
    };

    class TcpSocket : public SocketBase {
    public:
        virtual Status open();
        Status accept(socket_details::socket_t &peer, IpAddress &remoteAddress);
        Status listen(int backlogSize);
        Status connect(IpAddress const &remote);
        void setKeepAlive(bool reuse);
        void setNoDelay(bool noDelay);
        IpAddress getRemoteAddr() const;
    };

    class UdpSocket : public SocketBase {
    public:
        static const int MTU_SIZE = 1500;

        virtual Status open();

        Status enableBroadcast(bool enable);
        Status send(void const *data, size_t size, const IpAddress &address);
        net::Status recv(void *data, size_t buffer_size, size_t &received, IpAddress &from);
    private:
        socket_details::socket_t mSocket;
    };
}

#endif //NETWORK_SOCKET_H
