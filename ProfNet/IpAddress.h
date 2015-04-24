//
// Created by Cristian Marastoni on 23/04/15.
//

#ifndef NETWORK_IPADDRESS_H
#define NETWORK_IPADDRESS_H

#include "SocketDetails.h"

class IpAddress {
public:
    unsigned int ip;
    unsigned short port;

    static const IpAddress AnyAddress;
    static const IpAddress Loopback;
    static const IpAddress None;
    static const IpAddress Broadcast;

    static const unsigned int ANY_ADDRESS = 0;
    static const unsigned int BROADCAST_ADDRESS = 0xFFFFFFFF;
    static const unsigned int LOOPBACK_ADDRESS = 0x7f00001;
    static const unsigned int PORT_ANY = 0;

    static IpAddress resolve(const char *address);

    IpAddress() : ip(net::socket_details::addr_any), port(0) {
    }

    IpAddress(unsigned int addr_ip) : ip(addr_ip), port(0) {
    }

    IpAddress(unsigned int addr_ip, unsigned short addr_port) : ip(addr_ip), port(addr_port) {
    }

    IpAddress(net::socket_details::sockaddr_t const &addr) {
        set(addr);
    }

    void set(unsigned char b3, unsigned char b2, unsigned char b1, unsigned char b0) {
        ip = b3 << 24 | b2<< 16 | b1 << 8 | b0;
    }

    void set(unsigned int addr_ip, unsigned short addr_port) {
        ip = addr_ip;
        port = addr_port;
    }

    void set(net::socket_details::sockaddr_t const &addr);

    net::socket_details::sockaddr_t toSockaddr() const;

    const char *toString() const;

    struct Less {
        bool operator()(IpAddress const &addr1, IpAddress const &addr2) const {
            if(addr1.ip > addr2.ip) {
                return false;
            }else if(addr1.ip < addr2.ip) {
                return true;
            }else {
                return addr1.port < addr2.port;
            }
        }
    };
    friend bool operator==(IpAddress const &addr1, IpAddress const &addr2) {
        return addr1.ip == addr2.ip && addr1.port == addr2.port;
    }
};

#endif //NETWORK_IPADDRESS_H
