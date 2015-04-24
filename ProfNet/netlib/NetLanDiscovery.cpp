//
//  NetLanDiscovery.cpp
//  superasteroids
//
//  Created by Cristian Marastoni on 05/06/14.
//
//
#include "NetLanDiscovery.h"
#include "NetTime.h"
#include <algorithm>
#include "../Debug.h"

NetLanBeacon::NetLanBeacon(uint32 protocolID, uint16 listenerPort, uint16 serverPort)
	: mProtocolID(protocolID), mPort(listenerPort), mServerPort(serverPort)
{
}
NetLanBeacon::~NetLanBeacon() {
    Stop();
    
}
uint32 NetLanBeacon::Start(const char* name, uint32 sessionID, uint16 beaconPort) {
    if(mSocket.isOpen()) {
        return net::Ok;
    }
    mSocket.open();
    if(!mSocket.isOpen()) {
        return net::Error;
    }
    if(mSocket.enableBroadcast(true) != net::Ok) {
        return net::Error;
    }
    if(mSocket.bind(beaconPort) != net::Ok) {
        return net::Error;
    }
    mBroadcastAddress.set(IpAddress::BROADCAST_ADDRESS, mPort);
    mName = name;
    mNextBeaconTime = 0;
    mSessionID = sessionID;
    return 0;
}
void NetLanBeacon::Stop() {
    mSocket.close();
}
void NetLanBeacon::Update() {
    if(mSocket.isOpen()) {
        uint64 now = NetTime::Now();
        if (now > mNextBeaconTime) {
            SendBeacon();
            mNextBeaconTime = now + BeaconSettings::LAN_BEACON_TIMER;
        }
    }
}
void NetLanBeacon::SendBeacon() {
    NetStackData<256> data;
    data.pack32(mProtocolID);
    data.pack32(mSessionID);
    data.pack16(mServerPort);
    data.packData(mName.c_str(), mName.size());
    data.flush();

    if(mSocket.send(data.read_ptr(), data.size(), mBroadcastAddress) != net::Ok) {
        Debug::Error("BEACON", "Cannot send beacon. Error %d", mSocket.getLastErrorCode());
    }
}


NetLanDiscovery::NetLanDiscovery(uint32 protocolID, uint16 port) : mProtocolID(protocolID), mBeaconPort(port)
{
}
NetLanDiscovery::~NetLanDiscovery() {
    
}
uint32 NetLanDiscovery::Start() {
    if(mSocket.isOpen()) {
        return net::Ok;
    }
    mSocket.open();
    if(!mSocket.isOpen()) {
        return net::Error;
    }
    if(mSocket.bind(mBeaconPort) != net::Ok) {
        return net::Error;
    }
    return 0;
}
void NetLanDiscovery::Stop() {
    mSocket.close();
    for(auto it = mServers.begin();it != mServers.end(); ++it) {
        delete *it;
    }
    mServers.clear();
}
void NetLanDiscovery::Update() {
    if(!mSocket.isOpen()) {
        return;
    }
    struct sockaddr from;
    uint8 packet[net::UdpSocket::MTU_SIZE];
    size_t byteReceived = 0;
    do {
        IpAddress from;
        net::Status status = mSocket.recv(packet, net::UdpSocket::MTU_SIZE, byteReceived, from);
        if(status == net::Ok) {
            if(byteReceived > BeaconSettings::LAN_DISCOVERY_PACKET_MIN_SIZE) {
                //Received something.. that seem to make sense
                Debug::Log("DISCOVERY", "Received beacon");
                BitStream data(packet,byteReceived, true);
                ProcessBeacon(from, data);
            }
        }else {
            Debug::Error("LAN DISCOVERY", "Failed to receive data. Error %d", mSocket.getLastErrorCode());
        }
    }while(byteReceived > 0);
    UpdateServerList();
}

void NetLanDiscovery::ProcessBeacon(IpAddress const &from, BitStream &data) {
    uint32 proto;
    data.unpack32(proto);
    if(proto != mProtocolID) {
        Debug::Log("DISCOVERY", "No Beacon Protocol");
        return;
    }
    uint32 sessionID;
    uint16 serverPort;
    data.unpack32(sessionID);
    data.unpack16(serverPort);
    //Search if the address is already inside the list
	auto it =std::find_if(mServers.begin(), mServers.end(), [from](ServerInfo *serverInfo){
		return serverInfo->mAddress == from;
	});
   
    if(it != mServers.end()) {
        (*it)->mLastUpdate = NetTime::Now();
    }else {
        ServerInfo *info = new ServerInfo();
		info->mAddress = from;
        info->mLastUpdate = NetTime::Now();
        data.unpackString(info->mName);
        Debug::Log("DISCOVERY", "Added Server %s Address %s",
                   info->mName.c_str(),
                   info->mAddress.toString());
		mServers.push_back(info);
    }
}

void NetLanDiscovery::UpdateServerList() {
    uint64 now = NetTime::Now();
    for(auto it=mServers.begin();it!=mServers.end();) {
        if((now - (*it)->mLastUpdate) > BeaconSettings::LAN_DISCOVERY_TIMEOUT_MS) {
            //Remove from list
            delete *it;
            it = mServers.erase(it);
        }else {
            ++it;
        }
    }
}
