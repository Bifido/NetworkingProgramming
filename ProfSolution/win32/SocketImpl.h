//
// Created by Cristian Marastoni on 23/04/15.
//

#ifndef NETWORK_SOCKETIMPL_H
#define NETWORK_SOCKETIMPL_H
#include "../Types.h"

#ifdef WIN32
struct socket_details {
    typedef SOCKET socket_t;
    typedef struct sockaddr_in sockaddr_t;
    static const socket_t invalid_socket();
    static void close(socket_t &desc);
    static socket_t open(int family, int type, int proto);
    static void setblocking(socket_t desc, bool block);
    static sockaddr_t create_address(unsigned int ip, unsigned short port);
    static NetworkError::Code getError();
    static const char* getErrorString();
};

const char* socket_details::getErrorString(int error) {
    #define ERR_RET(x) case x: return #x
    switch (error)
    {
        ERR_RET(WSAEWOULDBLOCK);
        ERR_RET(WSAEALREADY);
        ERR_RET(WSAECONNABORTED);
        ERR_RET(WSAECONNRESET);
        ERR_RET(WSAETIMEDOUT);
        ERR_RET(WSAENETRESET);
        ERR_RET(WSAENOTCONN);
        ERR_RET(WSAEISCONN);
        default:
            return s;
    }
}

NetworkError::Code socket_details::getError() {
    switch (WSAGetLastError())
    {
        case WSAEWOULDBLOCK:  return Socket::WouldBlock;
        case WSAEALREADY:     return Socket::NotReady;
        case WSAECONNABORTED: return Socket::Disconnected;
        case WSAECONNRESET:   return Socket::Disconnected;
        case WSAETIMEDOUT:    return Socket::Disconnected;
        case WSAENETRESET:    return Socket::Disconnected;
        case WSAENOTCONN:     return Socket::Disconnected;
        case WSAEISCONN:      return Socket::Done;
        default:              return Socket::Error;
    }
}
socket_details::sockaddr_t socket_details::create_address(unsigned int address, unsigned short port)
{
    ::sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = htonl(address);
    addr.sin_port        = htons(port);
    addr.sin_family      = AF_INET;
    return addr;
}

socket_details::socket_t const socket_details::invalid_socket() {
    return INVALID_SOCKET;
}
void socket_details::close(socket_details::socket_t &desc) {
    if(desc != -1) {
        ::closesocket(desc);
        desc = INVALID_SOCKET;
    }
}
socket_details::socket_t socket_details::open(int family, int type, int proto) {
    return socket(family, type, proto);
}

void socket_details::setblocking(socket_details::socket_t desc, bool block) {
   u_long blocking = block ? 0 : 1;
   ioctlsocket(sock, FIONBIO, &blocking);
}
#endif

#endif //NETWORK_SOCKETIMPL_H
