#pragma once
#include "../IpAddress.h"

namespace NetStatusCode {
    static const int NONE = 0;
    static const int CONNECTING = 1;
    static const int CONNECTED  = 2;
    static const int DISCONNECTED = 3;
    static const int DISCONNECTED_BY_TIMEOUT = 4;
    static const int DISCONNECTED_USER_LIMIT = 5;
    static const int DISCONNECTED_BY_SERVER = 6;
    static const int NETWORK_ERROR = 1000;
};

enum ChannelType {
    CHANNEL_TYPE_SEQUENTIAL = 0,
    CHANNEL_TYPE_RELIABLE,
    CHANNEL_TYPE_UNRELIABLE,
};

namespace NetChannels {
    const uint8 UNRELIABLE = 0;
    const uint8 RELIABLE   = 1;
    const uint8 SEQUENTIAL = 2;
    
    const ChannelType CHANNELS_SETTINGS[] = {
        CHANNEL_TYPE_UNRELIABLE,
        CHANNEL_TYPE_RELIABLE,
        CHANNEL_TYPE_SEQUENTIAL,
    };
    const uint8 NUM_CHANNELS = sizeof(CHANNELS_SETTINGS) / sizeof(ChannelType);
}

namespace NetSettings {
    const int32 MAX_PACKET_SIZE = 1024;
    const int32 DEFAULT_PORT = 21900;
    const uint8 NET_MESSAGE_NUM_RESERVED_ID = 32;
    const uint8 NET_MESSAGE_ID_CUSTOM = NET_MESSAGE_NUM_RESERVED_ID;
    //This ID is temporary and invalid. Just to ask the server a new ID during connection phase
    const uint8 SERVER_PEER_ID = 255;
    const int32 MTU_MAX_SIZE = 1400;
    
    const float UNKNOWN_RTT = 1000.f;
    const float TIMEOUT = 3.f;
    const float KEEPALIVE_TIMER = 1.f;
    const float CONNECTION_TIMER = 1.f;
    const uint32 RESEND_TIMEOUT = 200;
    const int32 MAX_CONNECTION_RETRY = 5;
}

typedef uint32 NetID;