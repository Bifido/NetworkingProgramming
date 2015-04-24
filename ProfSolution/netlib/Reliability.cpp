//
//  Reliability.cpp
//  superasteroids
//
//  Created by Cristian Marastoni on 12/06/14.
//
//
#include "Reliability.h"
#include "Channel.h"
#include "Connection.h"


Reliability::Reliability() {
}
Reliability::~Reliability() {
    Reset();
}
void Reliability::Reset() {
    ClearPacketList(&mOutOfOrder);
    ClearPacketList(&mSentPacket);
}
void Reliability::RegisterChannel(Channel *channel) {
    mChannel = channel;
}
void Reliability::AddSentPacket(NetMessageHeader const &header, uint8 const *data) {
    NetPacket *packet = NetPacket::Create(header, data);
    packet->Sent(NetTime::Now());
	mSentPacket.AddFront(packet);
}
void Reliability::HandleAck(uint8 channel, uint16 sequence) {
	ListNode<NetPacket> *pNode = mSentPacket.mNext;
    while(pNode != &mSentPacket) {
        NetPacket *p = (NetPacket*)pNode;
        if(p->GetHeader().mSequenceNum == sequence) {
            RemovePacket(&mSentPacket, p);
            break;
        }
        pNode = pNode->mNext;
    }
}

void Reliability::HandleMessage(NetMessageHeader const &header, BitStream const &data) {
    if(header.mSequenceNum == mChannel->GetIngoingSequence()) {
        mChannel->GetConnection()->OutputUserMessage(header, data.read_ptr());
        mChannel->AdvanceIngoing();
        OutputMessages();
    }else {
        //out of order. Put in message queue, waiting for other messages
        NetPacket *packet = NetPacket::Create(header, data.read_ptr());
        InsertSorted(packet);
    }
}
void Reliability::OutputMessages() {
    int seq = mChannel->GetIngoingSequence();
	while(!mOutOfOrder.empty()) {
        NetPacket *packet = (NetPacket*)mOutOfOrder.mNext;
        if(packet->GetHeader().mSequenceNum != seq) {
            break;
        }
        mChannel->GetConnection()->OutputUserMessage(packet->GetHeader(), packet->GetData());
        RemovePacket(&mOutOfOrder, packet);
        seq = mChannel->AdvanceIngoing();
    }
}
void Reliability::Update() {
	if(mSentPacket.empty()) {
        return;
    }
    listnode_t *pNode = mSentPacket.mNext;
    uint64 now = NetTime::Now();
    while(pNode != &mSentPacket) {
        NetPacket *p = (NetPacket*)pNode;
        if(now >= p->Timeout()) {
            p->Sent(now);
            mChannel->GetConnection()->SendMessage(p->GetHeader(), p->GetData());
        }
        pNode = pNode->mNext;
    }
}

void Reliability::InsertSorted(NetPacket *packet) {
	if(mOutOfOrder.empty()) {
		mOutOfOrder.AddFront(packet);
        return;
    }
    
    NetPacket *oldPacket = (NetPacket*)mOutOfOrder.mNext;
    if(!mChannel->SequenceMoreRecent(packet->GetSequenceNum(), oldPacket->GetSequenceNum())) {
		mOutOfOrder.AddFront(packet);
        return;
    }
    oldPacket = (NetPacket*)mOutOfOrder.mPrev;
    if(mChannel->SequenceMoreRecent(packet->GetSequenceNum(), oldPacket->GetSequenceNum())) {
        mOutOfOrder.AddBack(packet);
    }else {
        listnode_t *pNode = mOutOfOrder.mNext;
        while (pNode != &mOutOfOrder) {
            oldPacket = (NetPacket*)pNode;
            if(mChannel->SequenceMoreRecent(packet->GetSequenceNum(), oldPacket->GetSequenceNum()))
            {
                InsertPacket(oldPacket, packet);
                break;
            }
            pNode = pNode->mNext;
        }
    }
}
void Reliability::ClearPacketList(listnode_t *root) {
    listnode_t *pNode = root->mNext;
    while(pNode != root) {
        root->RemoveNode(pNode);
        NetPacket::Destroy((NetPacket*)pNode);
        pNode = root->mNext;
    }
    root->Init();
}
void Reliability::RemovePacket(listnode_t *root, NetPacket *packet) {
    root->RemoveNode(packet);
    NetPacket::Destroy(packet);
}
void Reliability::InsertPacket(listnode_t *node, NetPacket *packet) {
    packet->mNext = node->mNext;
    packet->mPrev = node;
    node->mNext->mPrev = packet;
    node->mNext = packet;
}
