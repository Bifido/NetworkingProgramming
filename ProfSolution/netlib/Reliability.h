//
//  Reliability.h
//  superasteroids
//
//  Created by Cristian Marastoni on 12/06/14.
//
//

#pragma once
#include "Internal.h"

class Connection;
class Channel;

class Reliability {
public:
    Reliability();
    ~Reliability();
    void Reset();
    void RegisterChannel(Channel *channel);
    void AddSentPacket(NetMessageHeader const &header, uint8 const *data);
    void HandleAck(uint8 channel, uint16 sequence);
    void HandleMessage(NetMessageHeader const &header, BitStream const &data);
    void OutputMessages();
    void Update();
private:
	typedef ListNode<NetPacket> listnode_t;
	void InsertSorted(NetPacket *packet);
    void ClearPacketList(listnode_t *root);
    void RemovePacket(listnode_t *root, NetPacket *packet);
    void InsertPacket(listnode_t *node, NetPacket *packet);
    Channel *mChannel;
    listnode_t mSentPacket;
    listnode_t mOutOfOrder;
};
