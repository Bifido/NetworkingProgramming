//
//  Channel.h
//  superasteroids
//
//  Created by Cristian Marastoni on 31/05/14.
//
//

#pragma once
#include "NetTypes.h"
#include "Internal.h"
#include <vector>
#include <list>
#include <bitset>

class Connection;
class BitStream;

class Channel {
public:
    Channel(ChannelType channelType, uint8 channelId);
    virtual ~Channel();
    uint8 GetID() const {
        return mID;
    }
    ChannelType GetType() const {
        return mChannelType;
    }
    uint16 GetIngoingSequence() const {
        return mSequenceIn;
    };
    Connection *GetConnection() const {
        return mConnection;
    }
    void SetConnection(Connection *connection);
    bool SequenceMoreRecent( uint16 const newSeq, uint16 const oldSeq) const;
    int16 SequenceDistance(uint16 const newSeq, uint16 const oldSeq) const;
    uint16 ProduceOutgoing();
    uint16 AdvanceIngoing();
    virtual bool AcceptIncomingMessage(NetMessageHeader const &header, const BitStream &data) = 0;
    //virtual NetInternalMessage ProduceMessage(uint8 messageId, BitStream &data);
    virtual void Reset();
protected:
    Connection *mConnection;
    unsigned short mSequenceOut;
    unsigned short mSequenceIn;
    const ChannelType mChannelType;
    uint8 mID;
};


class SequentialChannel : public Channel {
public:
    SequentialChannel(unsigned char channelId);
    virtual bool AcceptIncomingMessage(NetMessageHeader const &header, const BitStream &data);
};

class UnreliableChannel : public Channel {
public:
    UnreliableChannel(unsigned char channelId);
    virtual bool AcceptIncomingMessage(NetMessageHeader const &header, const BitStream &data);
    virtual void Reset();
private:
    bool SequenceAlreadyReceived(uint16 receivedWindow, uint16 sequence) const;
    void MarkReceived(uint16 receivedWindow, uint16 sequence);
    uint32 mSequenceWindows[NetSettings::SEQUENCE_WINDOWS_COUNT];
};

class ReliableChannel : public Channel {
public:
    ReliableChannel(unsigned char channelId);
    virtual bool AcceptIncomingMessage(NetMessageHeader const &header, const BitStream &data);
    virtual void Reset();    
private:
    bool SequenceAlreadyReceived(uint16 receivedWindow, uint16 sequence) const;
    void MarkReceived(uint16 receivedWindow, uint16 sequence);
    void InsertSorted(NetPacket *packet);
    uint32 mSequenceWindows[NetSettings::SEQUENCE_WINDOWS_COUNT];
};

struct ChannelFactory {
    static Channel *Create(ChannelType const type, int channelId);
};
