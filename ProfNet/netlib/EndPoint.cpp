//
//  Peer.cpp
//  superasteroids
//
//  Created by Cristian Marastoni on 21/05/14.
//
//
#include "EndPoint.h"
#include "Internal.h"
#include "NetTime.h"
#include "Connection.h"
#include "Channel.h"
#include <algorithm>
#include "../Debug.h"

namespace {
    struct IfDisconnected {
        bool operator()(Connection const *connection) {
            return connection->GetState() == Connection::STATE_DISCONNECTED;
        }
    };
}


EndPoint::EndPoint():
    mPeerListener(NULL),
    mLocalPeer(0),
    mMaxConnections(0),
    mPacketID(0),
    mAcceptConnections(false),
    mIsServer(false)
{
    
}

EndPoint::~EndPoint() {
    Close();
    for(uint32 i=0;i<mMaxConnections;++i) {
        delete mConnectionsPool[i];
    }
}

bool EndPoint::IsNetworkAvailable() const {
	//TODO: Should find an interface and check if connected
	return true;
};

void EndPoint::Init(Settings const &settings, EndPointListener *listener) {
    mMaxConnections = settings.maxNumConnections;
    mPacketID = (settings.packetId[1] << 8) | settings.packetId[0];
    mConnections.reserve(mMaxConnections);
    mConnectionsPool = new Connection*[mMaxConnections];
    mPeerListener = listener;
    mLocalPeer = 0;
    mPort = settings.port;
    for(uint32 peer=0;peer<mMaxConnections;++peer) {
        Connection *connection = new Connection();
        connection->Init(this);
        connection->SetID(peer);
        connection->SetPeerListener(mPeerListener);
        mConnectionsPool[peer] = connection;
    }
}

bool EndPoint::Listen() {
    if(SocketOpen(true)) {
        mIsServer = true;
        mAcceptConnections = true;
        mLocalPeer = NetSettings::SERVER_PEER_ID;
        return true;
    }else {
        mIsServer = false;
        mAcceptConnections = false;
        mLocalPeer = 0;
        return false;
    }
}

bool EndPoint::Connect(IpAddress const &address) {
    if(SocketOpen(true)) {
        mIsServer = false;
        mAcceptConnections = false;
        Connection *connection = CreateConnection();
        connection->SetID(NetSettings::SERVER_PEER_ID);
        connection->StartConnection(address);
        return true;
    }else {
        return false;
    }
}

void EndPoint::Close() {
    CloseAllConnections();
    mSocket.close();
    for(auto it = mConnections.begin(); it != mConnections.end();++it) {
        ReleaseConnection(*it);
    }
    mConnections.clear();
    mAcceptConnections = false;
    mIsServer = false;
    Debug::Log("NET", "NetPort closed");
}


void EndPoint::Disconnect(NetID peer) {
    Connection *connection = FindConnectionById(peer);
    if(connection == NULL) {
        return;
    }
    connection->Disconnect();    
    ReleaseConnection(connection);
    mConnections.erase(std::remove(mConnections.begin(), mConnections.end(), connection));
}

void EndPoint::AcceptIncomingConnection(bool accept) {
    mAcceptConnections = accept;
}

void EndPoint::Update(float dt) {
    if(!IsOpened()) {
        return;
    }
    SocketReceive();
    for(auto it=mConnections.begin(); it!=mConnections.end();++it) {
        if((*it)->Update(dt) == Connection::STATE_DISCONNECTED) {
            ReleaseConnection((*it));
        }
    }
    mConnections.erase(std::remove_if(mConnections.begin(), mConnections.end(), IfDisconnected()), mConnections.end());
}

bool EndPoint::IsOpened() const {
    return mSocket.isOpen();
}

bool EndPoint::IsServer() const {
    return mIsServer;
}

void EndPoint::SendMessageTo(NetID peer, uint8 messageID, uint8 channelId, BitStream &data) {
    
    Connection *remote = FindConnectionById(peer);
    if(remote != NULL) {
        remote->ProduceMessage(data, messageID, channelId);
    }
}

void EndPoint::BroadcastMessage(uint8 messageID, uint8 channelId, BitStream &data) {
    for(auto it = mConnections.begin();it!=mConnections.end();++it) {
        (*it)->ProduceMessage(data, messageID, channelId);
    }
}

NetID EndPoint::GetLocalPeer() const {
    return mLocalPeer;
}

void EndPoint::CloseAllConnections() {
    for(unsigned i=0;i<mConnections.size();++i) {
        mConnections[i]->Disconnect();
    }
}


void EndPoint::SendPacket(IpAddress const &address, NetMessageHeader const &header, uint8 const *data)
{
    uint8 outgoingPacket[NetSettings::MTU_MAX_SIZE];
    BitStream packet(outgoingPacket, NetSettings::MTU_MAX_SIZE);
    
    NetPacketHeader packetHeader;
    packetHeader.mProtocolID = mPacketID;
    packetHeader.mPeerID = mLocalPeer;
    packetHeader.mNumMessages = 1;
    
    packet << packetHeader;
    packet << header;
    if(header.mSize > 0) {
        packet.append(data, header.mSize);
    }
    packet.flush();
    net::Status status = mSocket.send(packet.buffer(), packet.size(), address);
    if(status != net::Ok) {
        OnSocketError(status);
    }
}


void EndPoint::HandleReceivedPacket(const uint8 *buffer, size_t bufferSize, IpAddress const &remoteAddress) {
    Debug::Log("NET", "Received message size %d from %s", bufferSize, remoteAddress.toString());
    
    BitStream bs(buffer, bufferSize);
    if(!ParseHeader(bs)) {
        return;
    }
    Connection *remote = FindConnectionByAddress(remoteAddress);
    NetMessageHeader msgHeader;
    while(ParseNextMessage(bs, msgHeader)) {
        if(msgHeader.mMessageId < NetSettings::NET_MESSAGE_NUM_RESERVED_ID) {
            HandleSystemMessage(remoteAddress, remote, msgHeader, bs);
        }else if(remote != NULL) {
            remote->OnMessageReceived(msgHeader, bs);
        }else {
            Debug::Error("Received CUSTOM message from unknown sender %s. Discarding packet", remoteAddress.toString());
            break;
        }
    }
}

bool EndPoint::ParseHeader(BitStream const &bs) {
    const uint8 MAX_MESSAGE_PER_PACKET = static_cast<uint8 >((NetSettings::MTU_MAX_SIZE - sizeof(NetPacketHeader)) / 4);
    
    if(bs.size() < sizeof(NetPacketHeader)) {
        Debug::Error("NET", "Invalid Packet (size %d)", bs.size());
        return false;
    }
    NetPacketHeader packetHeader;
    bs >> packetHeader;
    if(packetHeader.mProtocolID != mPacketID) {
        Debug::Error("NET","Received message with ID: %d", packetHeader,mPacketID);
        return false;
    }
    if(packetHeader.mNumMessages > MAX_MESSAGE_PER_PACKET) {
        Debug::Error("NET","Invalid packet num messages");
        return false;
    }
    return true;
}
bool EndPoint::ParseNextMessage(BitStream const &bs, NetMessageHeader &msgHeader) {
    if(bs.remaining_bytes() < sizeof(msgHeader)) {
        return false;
    }

    bs >> msgHeader;
    if(msgHeader.mChannelId > NetChannels::NUM_CHANNELS) {
        Debug::Error("NET","ChannelNo > MAX_NUM_CHANNELS");
        return false;
    }
    uint32 remainingBytes = bs.remaining_bytes();
    if(msgHeader.mSize > bs.remaining_bytes()) {
        Debug::Error("NET","Invalid message size. (MessageSize >= Buffer Size)");
        return false;
    }
    return true;
}

void EndPoint::HandleSystemMessage(IpAddress const &remoteAddress, Connection *remote, NetMessageHeader const &msgHeader, BitStream const &bs)
{
    if(msgHeader.mMessageId == NET_MESSAGE_CONNECT) {
        if(remote == NULL && (mConnections.size() == mMaxConnections)) {
            DenyConnection(remoteAddress, NetStatusCode::DISCONNECTED_USER_LIMIT);
        }else {
            if(remote == NULL) {
                remote = CreateConnection();
            }
            remote->OnConnectionAttempt(remoteAddress,mSessionID);
        }
    }
    else if(remote != NULL) {
        if(msgHeader.mMessageId == NET_MESSAGE_ACCEPTED) {
            uint8 peerID;
            bs.unpack16(mSessionID);
            bs.unpack8(peerID);
            mLocalPeer = peerID;
            remote->OnConnectionAccepted(mSessionID);            
        }else if(msgHeader.mMessageId == NET_MESSAGE_CONNECT_ACK) {
            remote->OnConnectionAcknowledged();
        }else if(msgHeader.mMessageId == NET_MESSAGE_DENIED) {
            Debug::Log("NET", "CONNECTION DENIED BY %s", remote->GetAddress().toString());
            if(remote->GetState() == Connection::STATE_CONNECTING) {
                uint32 denialReason;
                bs.unpack32(denialReason);
                remote->OnConnectionDenied(denialReason);
            }else {
                Debug::Log("MESSAGE_DENIED from %s RECEIVED IN STATE %d", remoteAddress.toString(), remote->GetState());
            }
        }else if(msgHeader.mMessageId == NET_MESSAGE_DISCONNECT) {
            remote->Disconnect();
        }else if(msgHeader.mMessageId == NET_MESSAGE_PING) {
            remote->OnPing(bs);
        }else if(msgHeader.mMessageId == NET_MESSAGE_PONG) {
            remote->OnPong(bs);
        }else if(msgHeader.mMessageId == NET_MESSAGE_ACK){
            remote->OnAck(msgHeader.mChannelId, msgHeader.mSequenceNum);
        }else {
            Debug::Error("NET", "Unknown SYSTEM MESSAGE %d received from %s",
                         msgHeader.mMessageId, remoteAddress.toString());
        }
    }else {
       Debug::Log("Message %d received from %s but no connection found", remoteAddress.toString());
    }
}

Connection *EndPoint::CreateConnection() {
    Debug::Log("NET", "Creating new connection");
    
    Connection *remote = NULL;
    for(int i=0;i<mMaxConnections && remote==NULL;++i) {
        if(mConnectionsPool[i] != NULL) {
            remote = mConnectionsPool[i];
            mConnectionsPool[i] = NULL;
            mConnections.push_back(remote);
        }
    }
    return remote;
}
void EndPoint::ReleaseConnection(Connection *remote) {
    if(remote != NULL) {
        Debug::Log("NET", "Releasing connection %s", remote->GetAddress().toString());
        mConnectionsPool[remote->GetID()] = remote;
    }
}

void EndPoint::DenyConnection(IpAddress const &address, uint32 reason) {
    NetStackData<32> data;
    data.pack32(reason);
    data.flush();
    
    NetMessageHeader header;
    header.mSequenceNum = 0;
    header.mChannelId = NetChannels::UNRELIABLE;
    header.mMessageId = NET_MESSAGE_DENIED;
    header.mSize = data.size();
    
    Debug::Log("NET", "Deny connection to %s reason %d",
               address.toString(), reason);

    SendPacket(address, header, data.buffer());
}

Connection *EndPoint::FindConnectionById(uint8 peerID) const{
    std::vector<Connection*>::const_iterator it = std::find_if(mConnections.begin(), mConnections.end(), Connection::FindById(peerID));
    if(it != mConnections.end()) {
        return *it;
    }else {
        Debug::Log("NET", "Cannot find connection with id %d", peerID);
        return NULL;
    }
}
Connection *EndPoint::FindConnectionByAddress(IpAddress const &address) const{
    std::vector<Connection*>::const_iterator it = std::find_if(mConnections.begin(), mConnections.end(), Connection::FindByAddress(address));
    if(it != mConnections.end()) {
        return *it;
    }else {
        Debug::Log("NET", "Cannot find connection with address %s", address.toString());
        return NULL;
    }
}

bool EndPoint::SocketOpen(bool blocking) {
    if(mSocket.open() != net::Ok) {
        return false;
    }
    mSocket.setBlocking(blocking);
    if(mSocket.bind(mPort) != net::Ok) {
        mSocket.close();
        return false;
    }
    Debug::Log("NET", "Socket opened");
    return true;
}

void EndPoint::SocketReceive() {
    uint8 buffer[NetSettings::MTU_MAX_SIZE];
    IpAddress remoteAddress;
    size_t byteReceived;
    size_t numPacketRead = 0;
    net::Status status;
    do {
        status = mSocket.recv(buffer, NetSettings::MTU_MAX_SIZE, byteReceived, remoteAddress);
        if(status == net::Ok) {
            if(byteReceived > 0) {
                ++numPacketRead;
                HandleReceivedPacket(buffer, byteReceived, remoteAddress);
            }
        }else if(status != net::WouldBlock) {
            OnSocketError(status);
        }
    }while(status == net::Ok && numPacketRead >= NetSettings::MAX_PACKET_READ_PER_FRAME);
}

void EndPoint::OnSocketError(net::Status status) {
    Close();
    if(mPeerListener != NULL) {
        if(status == net::Disconnected) {
            mPeerListener->OnNetworkError(NetStatusCode::DISCONNECTED);
        }else {
            mPeerListener->OnNetworkError(NetStatusCode::NETWORK_ERROR);
        }
    }
}


