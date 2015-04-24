//
// Created by Cristian Marastoni on 23/04/15.
//

#ifndef NETWORK_TYPES_H
#define NETWORK_TYPES_H

namespace net {
    enum Status {
        Error = -1,
        Ok = 0,
        Disconnected,
        WouldBlock,
        InProgress,
        ConnResett,
        AddrInUse
    };
}

typedef int int32;
typedef long long int64;
typedef short int16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef unsigned short uint16;
typedef unsigned char uint8;

#endif //NETWORK_TYPES_H
