//
//  Internal.h
//  superasteroids
//
//  Created by Cristian Marastoni on 03/06/14.
//
//

#pragma once
#include "NetTypes.h"
#include "BitStream.h"
#include <new>
#include <stdlib.h>

enum SystemMessages {
    NET_MESSAGE_CONNECT = 0,
    NET_MESSAGE_ACCEPTED,
    NET_MESSAGE_CONNECT_ACK,
    NET_MESSAGE_DENIED,
    NET_MESSAGE_DISCONNECT,
    NET_MESSAGE_PING,
    NET_MESSAGE_PONG,
    NET_MESSAGE_ACK,
    NET_MESSAGE_SYSTEM_LAST = NetSettings::NET_MESSAGE_NUM_RESERVED_ID
};

namespace NetSettings {
    const int32 MAX_PACKET_READ_PER_FRAME = 30;
    const uint16 MAX_SEQUENCE = (uint16)(1u << 16);
    const uint16 SEQUENCE_WINDOW_SIZE = 32;
    const uint16 SEQUENCE_WINDOW_HALF = SEQUENCE_WINDOW_SIZE / 2;
    const uint16 SEQUENCE_WINDOWS_COUNT = 16;
    const uint32 SEQUENCE_WRAP = MAX_SEQUENCE/2;
    const uint8 MAX_MESSAGES_PER_PACKET = 32;
}

struct NetPacketHeader {
    uint16 mProtocolID;
    uint8 mNumMessages;
    uint8 mPeerID;
    
    friend BitStream &operator<<(BitStream &out, NetPacketHeader const &data) {
        out.pack16(data.mProtocolID);
        out.pack8(data.mNumMessages);
        out.pack8(data.mPeerID);
        return out;
    }
    friend BitStream const &operator>>(BitStream const &in, NetPacketHeader &data) {
        in.unpack16(data.mProtocolID);
        in.unpack8(data.mNumMessages);
        in.unpack8(data.mPeerID);
        return in;
    }
    
};

struct NetMessageHeader {
    uint8 mMessageId;
    uint8 mChannelId;
    uint16 mSequenceNum;
    uint16 mSize;
    
    friend BitStream &operator<<(BitStream &out, NetMessageHeader const &data) {
        out.pack8(data.mMessageId);
        out.pack8(data.mChannelId);
        out.pack16(data.mSequenceNum);
        out.pack16(data.mSize);
        return out;
    }
    friend BitStream const &operator>>(BitStream const &in, NetMessageHeader &data) {
        in.unpack8(data.mMessageId);
        in.unpack8(data.mChannelId);
        in.unpack16(data.mSequenceNum);
        in.unpack16(data.mSize);
        return in;
    }
};

class SocketLayer {
public:
    virtual void SendPacket(IpAddress const &address, NetMessageHeader const &header, uint8 const *data) = 0;
};

template<class T>
class ListNode {
public:
	ListNode<T> *mNext;
	ListNode<T> *mPrev;

	void Init() {
		mNext = mPrev = this;
	}
	void AddBack(ListNode *node) {
		node->mNext = this;
		node->mPrev = this->mPrev;
		node->mPrev->mNext = node;
		this->mPrev = node;
	}
	void AddFront(ListNode *node) {
		node->mNext = this->mNext;
		node->mPrev = this;
		node->mNext->mPrev = node;
		this->mNext = node;
	}
	void RemoveNode(ListNode *node) {
		node->mPrev->mNext = node->mNext;
		node->mNext->mPrev = node->mPrev;
		node->Init();
	}
	bool empty() {
		return mNext == this;
	}
};

class NetPacket : public ListNode<NetPacket>
{
public:
    static NetPacket *Create(NetMessageHeader const &header, uint8 const *data) {
        uint32 size = sizeof(NetPacket) + header.mSize;
        void *ptr = ::malloc(size);
        NetPacket *packet = new (ptr) NetPacket(header, data);
        return packet;
    }
    static void Destroy(NetPacket *packet) {
        ::free(packet);
    }
    NetMessageHeader const &GetHeader() const {
        return mHeader;
    }
    uint16  GetSequenceNum() const {
        return mHeader.mSequenceNum;
    }
    uint16  GetId() const {
        return mHeader.mMessageId;
    }
    void SetId(uint8 messageId) {
        mHeader.mMessageId = messageId;
    }
    void SetChannel(uint8 channelId) {
        mHeader.mChannelId = channelId;
    }
    void SetSequence(uint16 sequenceNum) {
        mHeader.mSequenceNum = sequenceNum;
    }
    void Sent(uint64 now) {
        ++mSentCount;
        mTimeout = now + mSentCount * NetSettings::RESEND_TIMEOUT;
    }
    uint16 GetSentCount() const {
        return mSentCount;
    }
    uint64 Timeout() const {
        return mTimeout;
    }
    uint16 Size() const {
        return mHeader.mSize;
    }
    uint8 const *GetData() const {
        return mData;
    }
private:
    NetPacket(NetMessageHeader const &header, uint8 const *data) :
    mHeader(header), mSentCount(0), mTimeout(0) {
        memcpy(mData, data, header.mSize);
    }
    virtual ~NetPacket() {
    }
    NetPacket(NetPacket const &);
    NetPacket &operator=(NetPacket const &);
private:
    NetMessageHeader mHeader;
    uint16 mSentCount;
    uint64 mTimeout;
    uint8 mData[1];
};


