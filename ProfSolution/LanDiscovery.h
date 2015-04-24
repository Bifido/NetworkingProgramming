//
// Created by Cristian Marastoni on 23/04/15.
//

#ifndef NETWORK_LANDISCOVERY_H
#define NETWORK_LANDISCOVERY_H

#include "IpAddress.h"

namespace net {
#pragma once

#include "NetTypes.h"
#include "IwString.h"
#include "IwArray.h"
#include "s3eSocket.h"

    namespace DiscoverySettings {
        static const size_t LAN_BEACON_TIMER = 3000;
        static const size_t LAN_DISCOVERY_TIMEOUT_MS = 10000;
        static const size_t LAN_DISCOVERY_PACKET_MIN_SIZE = 4;
        static const size_t DISCOVERY_NAME_SIZE = 32;
    }

    class ServerInfo {
    public:
        IpAddress mAddress;
        uint64 mLastUpdate;
        uint32 sessionID;
        char mName[DISCOVERY_NAME_SIZE];
    };

    class LanBeacon {
    public:
        NetLanBeacon(unsigned short listenerPort, unsigned short serverPort, const char *name);

        ~NetLanBeacon();

        void open(size_t protocolID, unsigned short beaconPort);

        void Stop();

        void Update();

        bool Running() const {
            return mSocket != NULL;
        }

    private:
        void SendBeacon();

        char mName[DISCOVERY_NAME_SIZE];
        IpAddress mBroadcastAddress;
        uint32 mProtocolID;
        uint16 mBeaconPort;
        uint16 mServerPort;
        uint64 mLastTimeSent;
        Socket socket;
    };

    class LanDiscovery {
    public:
        NetLanDiscovery(size_t
        protocolID,
        uint16 port
        );

        ~NetLanDiscovery();

        uint32 Start();

        void Stop();

        void Update();

        bool Running() const {
            return mSocket != NULL;
        }

        CIwArray<ServerInfo *> const &GetServers() const {
            return mServerInfo;
        }

    private:
        void ProcessBeacon(s3eInetAddress const &from, BitStream &data);

        void UpdateServerList();

        NetAddress mBroadcastAddress;
        uint32 mProtocolID;
        s3eSocket *mSocket;
        CIwArray<ServerInfo *> mServerInfo;
        uint16 mPort;
        float mTimer;

    };
}
#endif //NETWORK_LANDISCOVERY_H
