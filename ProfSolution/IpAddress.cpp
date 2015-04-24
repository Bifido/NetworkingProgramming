//
// Created by Cristian Marastoni on 23/04/15.
//

#include "IpAddress.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <memory.h>
#include <stdio.h>

const IpAddress IpAddress::AnyAddress(net::socket_details::addr_any, 0);
const IpAddress IpAddress::Loopback(net::socket_details::addr_loopback, 0);
const IpAddress IpAddress::None(net::socket_details::addr_none, 0);
const IpAddress IpAddress::Broadcast(net::socket_details::addr_broadcast, 0);

IpAddress IpAddress::resolve(const char *address) {
    if (address == NULL) {
        return None;
    }
    if (strcmp(address, "255.255.255.255") == 0) {
        return Broadcast;
    } else {
        unsigned int resolved = ::inet_addr(address);
        if (resolved != net::socket_details::addr_none) {
            return IpAddress(resolved, 0);
        }
        // Not a valid address, try to convert it as a host name
        addrinfo hints;
        ::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        addrinfo *result = NULL;
        if (getaddrinfo(address, NULL, &hints, &result) == 0) {
            if (result) {
                resolved = reinterpret_cast<net::socket_details::sockaddr_t *>(result->ai_addr)->sin_addr.s_addr;
                freeaddrinfo(result);
                return IpAddress(resolved, 0);
            }
        }
        return IpAddress::None;
    }
}

const char * IpAddress::toString() const {
    //TODO: This is a good candidate for thread local storage
    static char temp[32];
    sprintf(temp, "%d.%d.%d.%d:%d", ip >> 24, (ip >> 16) & 0xFF,  (ip >> 8) & 0xFF,  (ip & 0xFF), port);
    return temp;
}

net::socket_details::sockaddr_t IpAddress::toSockaddr() const {
    return net::socket_details::create_address(ip, port);
}

void IpAddress::set(net::socket_details::sockaddr_t const &addr) {
    port = ntohs(addr.sin_port);
    ip = ntohl(addr.sin_addr.s_addr);
}