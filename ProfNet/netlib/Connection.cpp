//
//  Connection.cpp
//  superasteroids
//
//  Created by Cristian Marastoni on 03/06/14.
//
//
#include "../Debug.h"
#include "../IpAddress.h"
#include "Connection.h"
#include "EndPoint.h"
#include "Channel.h"

Connection::Connection() {
    
}
Connection::~Connection() {
    DestroyAllChannels();
}

void Connection::Init(EndPoint *endPoint) {
    mNumChannels = NetChannels::NUM_CHANNELS;
    for(uint32 channelId=0;channelId<NetChannels::NUM_CHANNELS;++channelId) {
        mChannels[channelId] = ChannelFactory::Create(NetChannels::CHANNELS_SETTINGS[channelId], channelId);
        mChannels[channelId]->SetConnection(this);
        if(NetChannels::CHANNELS_SETTINGS[channelId] == CHANNEL_TYPE_RELIABLE) {
            mReliability.RegisterChannel(mChannels[channelId]);
        }
    }
    mEndPoint = endPoint;
    mKeepAliveTimeout = NetSettings::KEEPALIVE_TIMER;
    mConnectingTimer = NetSettings::CONNECTION_TIMER;
    mNumConnectionRetry = 0;
    mSessionID = 0;
    mNumMessageReceived = 0;
    mNumMessageSent = 0;
    mDisconnectReason = 0;
    mState = STATE_DISCONNECTED;
}

void Connection::SetPeerListener(EndPointListener *listener) {
    mPeerListener = listener;
}

void Connection::SetID(uint8 peerId) {
	mNetID = peerId;
}

void Connection::DestroyAllChannels() {
    for(uint32 i=0;i<mNumChannels;++i) {
        delete mChannels[i];
        mChannels[i] = NULL;
    }
    mNumChannels = 0;
}

Connection::State Connection::Update(float dt) {
    //Update channels
    if(mState == STATE_ACCEPTED || mState == STATE_CONNECTED) {
        mReliability.Update();
    }
    if(mState == STATE_CONNECTING) {
        mConnectingTimer -= dt;
        if(mConnectingTimer < 0.f) {
            //no response received, retry again
            if(mNumConnectionRetry == NetSettings::MAX_CONNECTION_RETRY) {
                ConnectionTimeout();
            }else {
                mConnectingTimer += NetSettings::CONNECTION_TIMER;
                ++mNumConnectionRetry;
                SendConnectionAttempt();
            }
        }
    }
    else if(mState == STATE_ACCEPTED) {
        mConnectingTimer -= dt;
        if(mConnectingTimer < 0.f) {
            //no response received, retry again
            if(mNumConnectionRetry == NetSettings::MAX_CONNECTION_RETRY) {
                ConnectionTimeout();
            }else {
                mConnectingTimer += NetSettings::CONNECTION_TIMER;
                ++mNumConnectionRetry;
                SendConnectionAccepted();
            }
        }
    }
    else if(mState == STATE_CONNECTED) {
        mConnectionTimeout -= dt;
        mKeepAliveTimeout -= dt;
        if(mConnectionTimeout < 0.f) {
            ConnectionTimeout();
        }else if(mKeepAliveTimeout < 0.f) {
            SendPing();
        }
    }
    return mState;
}

void Connection::StartConnection(const IpAddress &address) {
    mAddress = address;
    mState = STATE_CONNECTING;
    mConnectingTimer = NetSettings::CONNECTION_TIMER;
    mConnectionTimeout = NetSettings::TIMEOUT;
    mNumConnectionRetry = 0;
    mNumMessageSent = 0;
    SendConnectionAttempt();
    if(mPeerListener != NULL)  {
		mPeerListener->OnStatusChange(mNetID, NetStatusCode::CONNECTING);
    }
}

void Connection::Disconnect() {
    if(mState != STATE_DISCONNECTED) {
        NetStackData<32> data;
        data.pack64(NetTime::Now());
        ProduceMessage(data, NET_MESSAGE_PING, NetChannels::UNRELIABLE);
        mDisconnectReason = NetStatusCode::DISCONNECTED;
        mState = STATE_DISCONNECTED;
        if(mPeerListener != NULL)  {
            mPeerListener->OnStatusChange(mNetID, NetStatusCode::DISCONNECTED);
        }
    }
    //do not update the timeout
    ++mNumMessageReceived;
}

void Connection::OnConnectionAttempt(const IpAddress &address, uint16 sessionID) {
    if(mState == STATE_DISCONNECTED
       ) {
        Debug::Log("NET", "CONNECTION %s ATTEMPT RECEIVED. CHANGE STATE TO ACCEPTED",
                   address.toString());
        mAddress = address;
        mState = STATE_ACCEPTED;
        mConnectingTimer = NetSettings::CONNECTION_TIMER;
        mNumConnectionRetry = 0;
        mNumMessageSent = 0;
        mSessionID = sessionID;
        ResetChannels();
        if(mPeerListener != NULL)  {
            mPeerListener->OnStatusChange(mNetID, NetStatusCode::CONNECTING);
        }
    }
    UpdateMessageReceived();
    //Always send back accepted
    SendConnectionAccepted();
}

void Connection::ResetChannels() {
    for(size_t i=0;i<mNumChannels;++i) {
        mChannels[i]->Reset();
    }
}
void Connection::OnConnectionAccepted(uint16 sessionID) {
    if(mState == STATE_CONNECTING) {
        Debug::Log("NET", "CONNECTION %s ACCEPTED. CHANGE STATE TO CONNECTED",
                   mAddress.toString());
        mConnectionTimeout = NetSettings::TIMEOUT;
        mConnectingTimer = 0.f;
        mNumConnectionRetry = 0;
        mSessionID = sessionID;
        SendConnectionAcknowledge();
        ConnectionEstablished();
    }
    UpdateMessageReceived();
}
void Connection::OnConnectionDenied(uint32 reason) {
    if(mState == STATE_CONNECTING) {
        mState = STATE_DISCONNECTED;
        if(reason == NetStatusCode::DISCONNECTED_USER_LIMIT || reason == NetStatusCode::DISCONNECTED_BY_SERVER) {
            mDisconnectReason = reason;
        }else {
            mDisconnectReason = NetStatusCode::DISCONNECTED_BY_SERVER;
        }
        if(mPeerListener != NULL)  {
            mPeerListener->OnStatusChange(mNetID, reason);
        }
    }
    //do not update the timeout
    ++mNumMessageReceived;
}


void Connection::OnConnectionAcknowledged() {
    if(mState == STATE_ACCEPTED) {
        Debug::Log("NET", "CONNECTION %s AKNOWLEDGE. CHANGE STATE TO CONNECTED",
                   mAddress.toString());
        mConnectingTimer = 0.f;
        mNumConnectionRetry = 0;
        ConnectionEstablished();
    }
    UpdateMessageReceived();
}

void Connection::OnPing(BitStream const &pingData) {
    ConnectionEstablished();
    UpdateMessageReceived();
    uint64 remoteTime;
    pingData.unpack64(remoteTime);
    Debug::Log("PING", "REMOTE TIME: %llu", remoteTime);
    SendPong(remoteTime);
}
void Connection::OnPong(BitStream const &pongData) {
    ConnectionEstablished();
    UpdateMessageReceived();

    uint64 remoteTime;
    uint64 pingSentTime;
    pongData.unpack64(pingSentTime);
    pongData.unpack64(remoteTime);
    Debug::Log("PING", "REMOTE TIME: %llu", remoteTime);
    Debug::Log("PING", "LOCAL TIME: %llu", NetTime::Now());
    uint64 rtt = NetTime::Now() - pingSentTime;
    Debug::Log("PING", "RTT: %d", rtt);
    mNetTime.RecordPing(rtt);
}
void Connection::OnAck(uint8 channelId, uint16 ackSeq) {
    ConnectionEstablished();
    UpdateMessageReceived();
    Debug::Log("NET", "Peer %d ACK Channel %d Seq %d", GetID(),
               channelId, ackSeq);
    mReliability.HandleAck(channelId, ackSeq);
}
void Connection::OnMessageReceived(NetMessageHeader const &header, BitStream const &data) {

    Debug::Log("NET", "Peer %d MESSAGE RECEIVED Channel %d Seq %d ID %d", GetID(),
               header.mChannelId, header.mSequenceNum, header.mMessageId);
    
    Channel *channel = GetChannel(header.mChannelId);
    if(channel->AcceptIncomingMessage(header, data)) {
        if(channel->GetType() != CHANNEL_TYPE_RELIABLE) {
            OutputUserMessage(header, data.read_ptr());
        }else {
            SendAck(header.mChannelId, header.mSequenceNum);
            mReliability.HandleMessage(header, data);
        }
    }    
    data.skip_bytes(header.mSize);
    UpdateMessageReceived();
}

void Connection::SendConnectionAttempt() {
    Debug::Log("NET", "SENDING CONNECTION ATTEMPT NO %d TO %s",
               mNumConnectionRetry,
               mAddress.toString());
    BitStream temp;
    ProduceMessage(temp, NET_MESSAGE_CONNECT, NetChannels::UNRELIABLE);
}
void Connection::SendConnectionAccepted() {
    Debug::Log("NET", "SENDING CONNECTION ACCEPTED TO %s",
               mAddress.toString());
    NetStackData<32> data;
    data.pack16(mSessionID);
    data.pack8(mNetID);
    ProduceMessage(data, NET_MESSAGE_ACCEPTED, NetChannels::UNRELIABLE);
}
void Connection::SendConnectionAcknowledge() {
    Debug::Log("NET", "SENDING CONNECTION ACKNOLEDGE TO %s",
               mAddress.toString());
    BitStream temp;
    ProduceMessage(temp, NET_MESSAGE_CONNECT_ACK, NetChannels::UNRELIABLE);
}

void Connection::SendPing() {
    NetStackData<32> data;
    data.pack64(NetTime::Now());
    ProduceMessage(data, NET_MESSAGE_PING, NetChannels::UNRELIABLE);
}
void Connection::SendPong(uint64 remoteTime) {
    NetStackData<32> data;
    data.pack64(remoteTime);
    data.pack64(NetTime::Now());
    ProduceMessage(data, NET_MESSAGE_PONG, NetChannels::UNRELIABLE);
}


void Connection::UpdateMessageReceived() {
    ++mNumMessageReceived;
    mConnectionTimeout = NetSettings::TIMEOUT;
}

void Connection::ConnectionTimeout() {
    Debug::Log("NET", "connection %s timeout", mAddress.toString());
    mState = STATE_DISCONNECTED;
    mDisconnectReason = NetStatusCode::DISCONNECTED_BY_TIMEOUT;
    if(mPeerListener != NULL)  {
        mPeerListener->OnStatusChange(mNetID, NetStatusCode::DISCONNECTED_BY_TIMEOUT);
    }
}

void Connection::ConnectionEstablished() {
    if(mState == STATE_ACCEPTED || mState == STATE_CONNECTING) {
        mState = STATE_CONNECTED;
        if(mPeerListener != NULL)  {
            mPeerListener->OnStatusChange(mNetID, NetStatusCode::CONNECTED);
        }
    }
}

void Connection::ProduceMessage(BitStream &data, uint8 messageId, uint8 channelID) {
    Channel *channel = GetChannel(channelID);
    NetMessageHeader header;
    header.mChannelId = channelID;
    header.mSequenceNum = channel->ProduceOutgoing();
    header.mMessageId = messageId;    
    data.flush();
    header.mSize = static_cast<uint16>(data.size());
    SendMessage(header, data.buffer());
    if(channel->GetType() == CHANNEL_TYPE_RELIABLE) {
        mReliability.AddSentPacket(header, data.buffer());
    }
}

void Connection::SendMessage(NetMessageHeader const &header, uint8 const *data) {
    mEndPoint->SendPacket(mAddress, header, data);
    ++mNumMessageSent;
    //Reset the Keep Alive Timer
    mKeepAliveTimeout = NetSettings::KEEPALIVE_TIMER;
}

void Connection::SendAck(uint8 channelID, uint16 sequenceNum) {
    //ack doesn't contains data. Only
    NetMessageHeader header;
    header.mChannelId = channelID;
    header.mSequenceNum = sequenceNum;
    header.mMessageId = NET_MESSAGE_ACK;
    header.mSize = 0;
    SendMessage(header, NULL);
}

void Connection::OutputUserMessage(const NetMessageHeader &header, uint8 const *data) {
    if(mPeerListener != NULL) {
        BitStream b(data, header.mSize);
        mPeerListener->OnMessageReceived(mNetID, header.mMessageId,  b);
    }
}
