//
//  Channel.cpp
//  superasteroids
//
//  Created by Cristian Marastoni on 31/05/14.
//
//
#include "../Debug.h"
#include "Channel.h"
#include "Connection.h"
#include "BitStream.h"

Channel *ChannelFactory::Create(const ChannelType type, int channelId) {
    switch(type) {
        case CHANNEL_TYPE_SEQUENTIAL:
            return new SequentialChannel(channelId);
        case CHANNEL_TYPE_RELIABLE:
            return new ReliableChannel(channelId);
        case CHANNEL_TYPE_UNRELIABLE:
            return new UnreliableChannel(channelId);
		default:
			return NULL;
    }
}

Channel::Channel(ChannelType channelType, uint8 channelId) :
    mConnection(NULL), mSequenceIn(1), mSequenceOut(0),
    mID(channelId), mChannelType(channelType)
{
    
}

Channel::~Channel() {
    
}

void Channel::Reset() {
    mSequenceIn = 1;
    mSequenceOut = 0;
}

void Channel::SetConnection(Connection *connection) {
    mConnection = connection;
}
bool Channel::SequenceMoreRecent( uint16 const newSeq, uint16 const oldSeq) const
{
    return (( newSeq > oldSeq ) && ( oldSeq - oldSeq <= NetSettings::SEQUENCE_WRAP )) ||
            (( oldSeq > newSeq ) && ( oldSeq - newSeq > NetSettings::SEQUENCE_WRAP ));
}
int16 Channel::SequenceDistance(uint16 const newSeq, uint16 const oldSeq) const {
    if(newSeq > oldSeq) {
        int16 delta = newSeq - oldSeq;
        if(delta > NetSettings::SEQUENCE_WRAP) {
            return delta - NetSettings::MAX_SEQUENCE;
        }
        return delta;
    }else {
        int16 delta = oldSeq - newSeq;
        if(delta > NetSettings::SEQUENCE_WRAP) {
            return NetSettings::MAX_SEQUENCE - delta;
        }
        return -delta;
    }
}
uint16 Channel::ProduceOutgoing() {
    //unnecessary if the MAX_SQUENCE == MAX_SHORT
    if(++mSequenceOut == NetSettings::MAX_SEQUENCE) {
        mSequenceOut = 0;
    }
    return mSequenceOut;
}

uint16 Channel::AdvanceIngoing() {
    //unnecessary if the MAX_SQUENCE == MAX_SHORT
    ++mSequenceIn;
    if(mSequenceIn > NetSettings::MAX_SEQUENCE) {
        mSequenceIn = 0;
    }
    return mSequenceIn;
}

SequentialChannel::SequentialChannel(unsigned char channelId) : Channel(CHANNEL_TYPE_SEQUENTIAL, channelId)
{
}
bool SequentialChannel::AcceptIncomingMessage(NetMessageHeader const &header, BitStream const &data) {
    if(!SequenceMoreRecent(header.mSequenceNum, mSequenceIn)) {
        return false;
    }
    mSequenceIn = header.mSequenceNum;
    return true;
}

UnreliableChannel::UnreliableChannel(unsigned char channelId) : Channel(CHANNEL_TYPE_UNRELIABLE, channelId)
{
    memset(mSequenceWindows, 0, sizeof(mSequenceWindows));
}

void UnreliableChannel::Reset() {
    Channel::Reset();
    memset(mSequenceWindows, 0, sizeof(mSequenceWindows));
}
bool UnreliableChannel::AcceptIncomingMessage(NetMessageHeader const &header, const BitStream &data) {
    //check if can be accepted or not if it fit the current message window
    uint16 receivedWindow = header.mSequenceNum / NetSettings::SEQUENCE_WINDOW_SIZE;
    uint16 oldWindow = mSequenceIn / NetSettings::SEQUENCE_WINDOW_SIZE;
    if(receivedWindow < oldWindow) {
        receivedWindow += NetSettings::SEQUENCE_WINDOWS_COUNT;
    }
    if(receivedWindow - oldWindow > (NetSettings::SEQUENCE_WINDOWS_COUNT/2)) {
        Debug::Log("NET","Reliable message %s sequence %d discarded", header.mMessageId, header.mSequenceNum);
        return false;
    }
    if(SequenceAlreadyReceived(receivedWindow, header.mSequenceNum)) {
        //sequence already received..
        Debug::Log("NET","Unreliable message %s sequence %d duplicate", header.mMessageId, header.mSequenceNum);
        return false;
    }
    MarkReceived(receivedWindow, header.mSequenceNum);
    if(SequenceMoreRecent(header.mSequenceNum, mSequenceIn)) {
        mSequenceIn = header.mSequenceNum;
        if(receivedWindow != oldWindow) {
            mSequenceWindows[oldWindow] = 0;
        }
    }
    return true;
}

void UnreliableChannel::MarkReceived(uint16 window, uint16 sequence) {
    uint16 index = sequence % NetSettings::SEQUENCE_WINDOW_SIZE;
    mSequenceWindows[window] |= (1 << index);
}
bool UnreliableChannel::SequenceAlreadyReceived(uint16 window, uint16 sequence) const {
    uint16 index = sequence % NetSettings::SEQUENCE_WINDOW_SIZE;
    return (mSequenceWindows[window] & (1 << index)) != 0;
}

ReliableChannel::ReliableChannel(unsigned char channelId) :
    Channel(CHANNEL_TYPE_RELIABLE, channelId)
{
    memset(mSequenceWindows, 0, sizeof(mSequenceWindows));
}

void ReliableChannel::Reset() {
    Channel::Reset();
    memset(mSequenceWindows, 0, sizeof(mSequenceWindows));
}

void ReliableChannel::MarkReceived(uint16 window, uint16 sequence) {
    uint16 index = sequence % NetSettings::SEQUENCE_WINDOW_SIZE;
    mSequenceWindows[window] |= (1 << index);
}
bool ReliableChannel::SequenceAlreadyReceived(uint16 window, uint16 sequence) const {
    uint16 index = sequence % NetSettings::SEQUENCE_WINDOW_SIZE;
    return (mSequenceWindows[window] & (1 << index)) != 0;
}

bool ReliableChannel::AcceptIncomingMessage(NetMessageHeader const &header, const BitStream &data) {
    //check if can be accepted or not if it fit the current message window
    uint16 receivedWindow = header.mSequenceNum / NetSettings::SEQUENCE_WINDOW_SIZE;
    uint16 oldWindow = mSequenceIn / NetSettings::SEQUENCE_WINDOW_SIZE;
    if(receivedWindow < oldWindow) {
        receivedWindow += NetSettings::SEQUENCE_WINDOWS_COUNT;
    }
    if(receivedWindow - oldWindow > (NetSettings::SEQUENCE_WINDOWS_COUNT/2)) {
        Debug::Log("NET","Reliable message %s sequence %d discarded", header.mMessageId, header.mSequenceNum);
        return false;
    }
    if(SequenceAlreadyReceived(receivedWindow, header.mSequenceNum)) {
        //sequence already received..
        Debug::Log("NET","Unreliable message %d sequence %d duplicate", header.mMessageId, header.mSequenceNum);
        return false;
    }
    MarkReceived(receivedWindow, header.mSequenceNum);
    uint16 currentWindow = header.mSequenceNum / NetSettings::SEQUENCE_WINDOW_SIZE;
    if(currentWindow != oldWindow) {
        mSequenceWindows[oldWindow] = 0;
    }
    return true;
}
