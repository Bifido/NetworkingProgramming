//
//  NetLanDiscovery.h
//  superasteroids
//
//  Created by Cristian Marastoni on 05/06/14.
//
//

#pragma once
#include "NetTypes.h"
#include "BitStream.h"
#include "../Socket.h"
#include <vector>
#include <string>

namespace BeaconSettings {
    static const uint32 LAN_BEACON_TIMER = 3000;
    static const uint32 LAN_DISCOVERY_TIMEOUT_MS = 10000;
    static const uint32 LAN_DISCOVERY_PACKET_MIN_SIZE = 4;
}

class ServerInfo {
public:
    IpAddress mAddress;
    uint64 mLastUpdate;
    uint32 sessionID;
    std::string mName;
};
typedef std::vector<ServerInfo *> ServerList;

class NetLanBeacon {
public:
    NetLanBeacon(uint32 protocolID, uint16 listenerPort, uint16 serverPort);    
    ~NetLanBeacon();
    uint32 Start(const char *name, uint32 sessionID, uint16 beaconPort = 0);
    void Stop();
    void Update();
    bool Running() const {
        return mSocket.isOpen();
    }
private:
    void SendBeacon();
    std::string mName;
    uint32 mProtocolID;
    uint32 mSessionID;
    uint16 mPort;
    uint16 mServerPort;
    uint64 mNextBeaconTime;
    IpAddress mBroadcastAddress;
	net::UdpSocket mSocket;
};

class NetLanDiscovery {
public:
    NetLanDiscovery(uint32 protocolID, uint16 port);
    ~NetLanDiscovery();
    uint32 Start();
    void Stop();
    void Update();
    bool Running() const {
        return mSocket.isOpen();
    }
   ServerList const &GetServers() const {
        return mServers;
    }
private:
    void ProcessBeacon(IpAddress const &from, BitStream &data);
    void UpdateServerList();
	ServerList mServers;
    uint32 mProtocolID;
    uint16 mBeaconPort;
	net::UdpSocket mSocket;
    float mTimer;
};