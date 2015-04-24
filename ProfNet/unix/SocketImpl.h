//
// Created by Cristian Marastoni on 23/04/15.
//

#ifndef NETWORK_SOCKETIMPL_H
#define NETWORK_SOCKETIMPL_H
#include <netinet/in.h>
#include "../Types.h"

namespace net {
    struct socket_details {
        typedef int socket_t;
        typedef struct sockaddr_in sockaddr_t;

        static const socket_t invalid_socket();

        static void close(socket_t &desc);

        static socket_t open(int family, int type, int proto);

        static void set_blocking(socket_t desc, bool block);

        static sockaddr_t create_address(unsigned int ip, unsigned short port);

        static int getError();

        static Status translateError(int error);

        static const char *getErrorString(int error);

        static const unsigned int addr_any = INADDR_ANY;
        static const unsigned int addr_broadcast = INADDR_BROADCAST;
        static const unsigned int addr_loopback = INADDR_LOOPBACK;
        static const unsigned int addr_none = INADDR_NONE;
    };
}

#endif //NETWORK_SOCKETIMPL_H
