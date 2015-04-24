//
// Created by Cristian Marastoni on 23/04/15.
//

#include "SocketImpl.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <memory.h>

namespace net {
    const char *socket_details::getErrorString(int error) {
        //use thread local storage for that... how ?
        static char temp[256];

        if (errno == EAGAIN) return "EAGAIN";
        else if (errno == EINPROGRESS) return "EINPROGRESS";

#define ERR_RET(x) case x: return #x
        switch (errno) {
            ERR_RET(EINPROGRESS);
            ERR_RET(EWOULDBLOCK);
            ERR_RET(ECONNABORTED);
            ERR_RET(ECONNRESET);
            ERR_RET(ETIMEDOUT);
            ERR_RET(ENETRESET);
            ERR_RET(ENOTCONN);
            ERR_RET(EADDRINUSE);
            ERR_RET(EPIPE);
            default: {
                sprintf(temp, "error %d", error);
                return temp;
            }
        }
    }

    int socket_details::getError() {
        // The followings are sometimes equal to EWOULDBLOCK,
        // so we have to make a special case for them in order
        // to avoid having double values in the switch case
        return errno;
    }

    Status socket_details::translateError(int error) {
        if ((error == EAGAIN) || (error == EINPROGRESS)) {
            return net::InProgress;
        }
        switch (error) {
            case EWOULDBLOCK:
                return net::WouldBlock;
            case ECONNABORTED:
                return Disconnected;
            case ECONNRESET:
                return Disconnected;
            case ETIMEDOUT:
                return Disconnected;
            case ENETRESET:
                return Disconnected;
            case ENOTCONN:
                return Disconnected;
            case EPIPE:
                return Disconnected;
            case EADDRINUSE:
                return net::AddrInUse;
            default:
                return net::Error;
        }
    }

    socket_details::sockaddr_t socket_details::create_address(unsigned int address, unsigned short port) {
        struct sockaddr_in addr;
        ::memset(&addr, 0, sizeof(addr));
        addr.sin_addr.s_addr = htonl(address);
        addr.sin_port = htons(port);
        addr.sin_family = AF_INET;
        return addr;
    }

    socket_details::socket_t const socket_details::invalid_socket() {
        return -1;
    }

    void socket_details::close(socket_details::socket_t &desc) {
        if (desc != -1) {
            ::close(desc);
            desc = -1;
        }
    }

    socket_details::socket_t socket_details::open(int family, int type, int proto) {
        return socket(family, type, proto);
    }

    void socket_details::set_blocking(socket_details::socket_t desc, bool block) {
        int status = fcntl(desc, F_GETFL);
        if (block)
            fcntl(desc, F_SETFL, status & ~O_NONBLOCK);
        else
            fcntl(desc, F_SETFL, status | O_NONBLOCK);
    }
}