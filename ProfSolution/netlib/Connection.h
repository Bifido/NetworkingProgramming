//
//  Connection.h
//  superasteroids
//
//  Created by Cristian Marastoni on 03/06/14.
//
//

#include "NetTypes.h"
#include "NetTime.h"
#include "Reliability.h"
#include "../IpAddress.h"

class SocketLayer;
class BitStream;
class Channel;
class NetPacket;
class EndPoint;
class EndPointListener;
struct NetMessageHeader;

class Connection {
public:
    enum State {
        STATE_CONNECTED,
        STATE_DISCONNECTED,
        STATE_CONNECTING,
        STATE_ACCEPTED,
        STATE_DISCONNECTING,
        STATE_ERROR
    };
    Connection();
    ~Connection();
    void Init(EndPoint *endPoint);
    void SetID(uint8 peerId);
	void SetPeerListener(EndPointListener *listener);
    void Disconnect();
    void StartConnection(const IpAddress &address);
    
    void ProduceMessage(BitStream &data, uint8 messageId, uint8 channelID);
    void SendMessage(NetMessageHeader const &header, uint8 const *data);
    
    void OnConnectionAttempt(const IpAddress &address, uint16 sessionID);
    void OnConnectionAccepted(uint16 sessionID);
    void OnConnectionAcknowledged();
    void OnConnectionDenied(uint32 reason);
    void OnPing(BitStream const &pingData);
    void OnPong(BitStream const &pongData);
    void OnAck(uint8 channelId, uint16 ackSeq);
    void OnMessageReceived(NetMessageHeader const &header, BitStream const &data);
    void OutputUserMessage(const NetMessageHeader &header, uint8 const *data);
    
    State GetState() const {
        return mState;
    }
    uint32 GetDisconnectReason() const {
        return mDisconnectReason;
    }
    State Update(float dt);
   
    NetID GetID() const {
        return mNetID;
    }
    IpAddress const &GetAddress() const {
        return mAddress;
    }
    Channel *GetChannel(uint8 channelId) const {
        return mChannels[channelId];
    }
public:
    struct FindByAddress {
        FindByAddress(IpAddress const &address) : mIP(address) { }
        bool operator()(Connection const *connection) const {
            return connection->GetAddress() == mIP;
        }
        IpAddress const &mIP;
    };
    struct FindById {
        FindById(NetID peerID) : mID(peerID) { }
        bool operator()(Connection const *connection) const {
            return connection->GetID() == mID;
        }
        uint8 const mID;
    };
private:
    void ResetChannels();
    void DestroyAllChannels();
    void SendPing();
    void SendPong(uint64 remoteTime);
    void SendAck(uint8 channelId, uint16 messageSeq);
    void SendConnectionAttempt();
    void SendConnectionAccepted();
    void SendConnectionAcknowledge();
    void ConnectionTimeout();
    void ConnectionError();
    void ConnectionEstablished();
    void UpdateMessageReceived();
private:
    State mState;
    uint8 mNumConnectionRetry;
    uint16 mSessionID;
    EndPoint *mEndPoint;
	EndPointListener *mPeerListener;
    Reliability mReliability;
    Channel *mChannels[NetChannels::NUM_CHANNELS];
    uint32 mNumChannels;
    uint32 mDisconnectReason;
    uint32 mNumMessageReceived;
    uint32 mNumMessageSent;
    float mKeepAliveTimeout;
    float mConnectionTimeout;
    float mConnectingTimer;
    NetTime mNetTime;
    NetID mNetID;
	IpAddress mAddress;
};
