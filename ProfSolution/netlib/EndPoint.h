//
//  Peer.h
//  superasteroids
//
//  Created by Cristian Marastoni on 21/05/14.
//
//

#pragma once

#include "NetTypes.h"
#include "Internal.h"
#include "../Socket.h"
#include <vector>

class Connection;
class IpAddress;
class BitStream;

class EndPointListener {
public:
    virtual void OnNetworkError(NetID statusCode) = 0;
    virtual void OnStatusChange(NetID peerID, uint32 statusCode) = 0;
    virtual void OnMessageReceived(NetID peerID, uint8 messageID, BitStream const &data) = 0;
};

class EndPoint
{
public:
    struct Settings {
        uint16 port;
        uint16 maxNumConnections;
        char packetId[2];
    };
    EndPoint();
    virtual ~EndPoint();
    bool IsNetworkAvailable() const;
    void Init(Settings const &settings, EndPointListener *listener);
    bool Listen();
    bool Connect(IpAddress const &address);
    void Disconnect(NetID peer);
    void Close();
    void Update(float dt);
    void AcceptIncomingConnection(bool accept);
    void SendMessageTo(NetID peer, uint8 messageID, uint8 channelId, BitStream &data);
    void BroadcastMessage(uint8 messageID, uint8 channelId, BitStream &data);
    NetID GetLocalPeer() const;
    bool IsOpened() const;
    bool IsServer() const;
private:
    friend class Connection;
    void SendPacket(IpAddress const &address, NetMessageHeader const &header, uint8 const *data);
    bool SocketOpen(bool blocking);
    void SocketReceive();
    void HandleReceivedPacket(const uint8 *buffer, size_t bufferSize, IpAddress const &remoteAddress);
    bool ParseHeader(BitStream const &bs);
    bool ParseNextMessage(BitStream const &bs, NetMessageHeader &msgHeader);
    void OnSocketError(net::Status status);
    void DenyConnection(IpAddress const &address, uint32 reason);
    void HandleSystemMessage(IpAddress const &address, Connection *connection, NetMessageHeader const & msgHeader, BitStream const &data);
    
    Connection *CreateConnection();
    void ReleaseConnection(Connection *connection);
    Connection *FindConnectionById(uint8 peerId) const;
    Connection *FindConnectionByAddress(IpAddress const &address) const;
    void CloseAllConnections();
private:
    net::UdpSocket mSocket;
    NetID mLocalPeer;
	EndPointListener *mPeerListener;
    std::vector<Connection *> mConnections;
    Connection **mConnectionsPool;
    uint16 mPacketID;
    uint16 mSessionID;
    uint16 mMaxConnections;
    uint16 mPort;
    bool mAcceptConnections;
    bool mIsServer;
};



