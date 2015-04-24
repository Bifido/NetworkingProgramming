//
//  NetTime.h
//  superasteroids
//
//  Created by Cristian Marastoni on 01/06/14.
//
//

#pragma once

#include "NetTypes.h"
#include <time.h>


class NetTime {
public:
    static uint64 Now();

    NetTime();
    uint64 GetServerTime() const {
        uint64 delta = Now() - mLastSyncTime;
        return mServerTime + delta;
    }
    void SetServerTime(uint64 serverTime) {
        mLastSyncTime = Now();
        UpdateAverageLatency();
        mServerTime = serverTime + mEstimatedLatency;
    }
    //int because it can be negative..
    int64 GetLocalEventTime(uint64 serverEventTime) const {
        if(serverEventTime < mServerTime) {
            int64 delta = mServerTime - serverEventTime;
            return mLastSyncTime - delta;
        }else {
            return mLastSyncTime + serverEventTime - mServerTime;
        }
    }
    uint64 GetAverageLatency() const {
        return mEstimatedLatency;
    }
    void RecordPing(uint64 ping) {
        uint32 pingSlot = ++mPingHead % PING_AVERAGE_SIZE;
        mPings[pingSlot] = ping;
        mPingCount++;
        if((mPingCount % PING_AVERAGE_SIZE) == 0) {
            UpdateAverageLatency();
        }
    }
    void UpdateAverageLatency();
private:
    static const int PING_AVERAGE_SIZE = 7;
    uint32 mPingHead;
    uint32 mPingCount;
    uint64 mPings[PING_AVERAGE_SIZE];
    uint64 mEstimatedLatency;
    uint64 mLastSyncTime;
    uint64 mServerTime;
};