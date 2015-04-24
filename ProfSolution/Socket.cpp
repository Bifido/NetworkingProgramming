//
// Created by Cristian Marastoni on 23/04/15.
//

#include "Socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include "Debug.h"
#include <memory.h>

namespace net {
    SocketBase::~SocketBase() {
    }

    Status SocketBase::open(socket_details::socket_t handle) {
        if(mHandle != socket_details::invalid_socket()) {
            close();
        }
        mHandle = handle;
        return Ok;
    }
    void SocketBase::close() {
        socket_details::close(mHandle);
    }

    void SocketBase::setBlocking(bool blocking) {
        socket_details::set_blocking(mHandle, blocking);
    }
    void SocketBase::setReusePort(bool reuse) {
        int opt = reuse ? 1 : 0;
        if(setsockopt(mHandle, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
            Debug::Error("SOCKET", "cannot set SO_REUSEPORT");
        }
    }

    void SocketBase::setReuseAddr(bool reuse) {
        int opt = reuse ? 1 : 0;
        if(setsockopt(mHandle, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            Debug::Error("SOCKET", "cannot set SO_REUSEADDR");
        }
    }

    IpAddress SocketBase::getLocalAddr() const
    {
        socket_details::sockaddr_t addr;
        ::memset(&addr, 0, sizeof addr);
        socklen_t addrlen = static_cast<socklen_t>(sizeof addr);
        if (getsockname(mHandle, reinterpret_cast<sockaddr*>(&addr), &addrlen) < 0) {
            Debug::Log("SOCKET","getsockname error: %d\n", socket_details::getError());
        }
        return IpAddress(addr);
    }

    Status SocketBase::bind(unsigned short port) {
        socket_details::sockaddr_t addr = socket_details::create_address(socket_details::addr_any, port);
        int ret = ::bind(mHandle, reinterpret_cast<::sockaddr *>(&addr), sizeof(addr));
        if (ret < 0) {
            return getLastError();
        }
        return Status::Ok;
    }

    Status TcpSocket::open() {
        // Don't create the socket if it already exists
        if(mHandle == socket_details::invalid_socket()) {
            mHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(mHandle == -1) {
                return getLastError();
            }
            return Ok;
        }
        return Ok;
    }

    Status TcpSocket::accept(socket_details::socket_t &peer, IpAddress &remoteAddress) {
        socket_details::sockaddr_t addr;
        unsigned int addrLen = sizeof(addr);
        peer = ::accept(mHandle, reinterpret_cast<sockaddr*>(&addr), &addrLen);
        if(peer < 0) {
            return getLastError();
        }
        remoteAddress.set(addr);
        return net::Ok;
    }
    Status TcpSocket::listen(int backlockSize) {
        //the
        int ret = ::listen(mHandle, backlockSize);
        if(ret < 0) {
            return getLastError();
        }
        return net::Ok;
    }
    Status TcpSocket::connect(IpAddress const &remote) {
        socket_details::sockaddr_t addr = remote.toSockaddr();
        int ret = ::connect(mHandle, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        if(ret < 0) {
            return getLastError();
        }else {
            return net::Ok;
        }
    }

    void TcpSocket::setKeepAlive(bool reuse) {
        int opt = reuse ? 1 : 0;
        if(setsockopt(mHandle, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0) {
            Debug::Error("SOCKET", "cannot set SO_KEEPALIVE");
        }
    }
    void TcpSocket::setNoDelay(bool noDelay) {
        int opt = noDelay ? 1 : 0;
        if(setsockopt(mHandle, SOL_SOCKET, TCP_NODELAY, &opt, sizeof(opt)) < 0) {
            Debug::Error("SOCKET", "cannot set TCP_NODELAY");
        }
    }

    IpAddress TcpSocket::getRemoteAddr() const
    {
        socket_details::sockaddr_t addr;
        ::memset(&addr, 0, sizeof addr);
        socklen_t addrlen = static_cast<socklen_t>(sizeof addr);
        if (getpeername(mHandle, reinterpret_cast<sockaddr*>(&addr), &addrlen) < 0)
        {
            Debug::Error("SOCKET","getpeername error: %d\n", socket_details::getError());
        }
        return IpAddress(addr);
    }

    Status UdpSocket::open() {
        // Don't create the socket if it already exists
        if(mHandle == socket_details::invalid_socket()) {
            mHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if(mHandle == -1) {
                return getLastError();
            }
            return Ok;
        }
        return Ok;
    }

    Status UdpSocket::enableBroadcast(bool enable) {
        // Enable broadcast by default for UDP sockets
        int yes = 1;
        if (setsockopt(mHandle, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&yes), sizeof(yes)) == -1) {
            return Error;
        }
        return Ok;
    }

    Status UdpSocket::send(void const *data, size_t size, const IpAddress &address) {
        if (size > MTU_SIZE) {
            return Status::Error;
        }
        socket_details::sockaddr_t addr = socket_details::create_address(address.ip, address.port);
        ssize_t ret = ::sendto(mHandle, data, size, 0, reinterpret_cast<::sockaddr *>(&addr), sizeof(addr));
        if (ret < 0) {
            return getLastError();
        } else if (ret < size) {
            //Partial sent...
            return Error;
        }
        return Ok;
    }

    net::Status UdpSocket::recv(void *data, size_t buffer_size, size_t &received, IpAddress &from) {
        if (data == NULL) {
            received = 0;
            return Error;
        }
        socket_details::sockaddr_t addr = socket_details::create_address(0, 0);
        unsigned int addr_len = sizeof(addr);
        ssize_t ret = ::recvfrom(handle(), static_cast<char *>(data), buffer_size, 0,
                                 reinterpret_cast<::sockaddr *>(&addr), &addr_len);
        if (ret < 0) {
            return getLastError();
            received = 0;
        } else {
            received = static_cast<size_t>(ret);
            from.set(addr);
        }
        return Ok;
    }

}
