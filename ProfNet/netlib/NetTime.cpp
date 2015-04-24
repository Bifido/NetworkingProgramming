//
//  NetTime.cpp
//  superasteroids
//
//  Created by Cristian Marastoni on 01/06/14.
//
//
#include "NetTime.h"
#include "../Debug.h"
#include <string.h>
#include <algorithm>

uint64 NetTime::Now() {
#ifdef WIN32
    return GetTickCount64();
#else
    //TODO:
    return 0;
#endif
}

NetTime::NetTime() : mPingHead(0), mEstimatedLatency(1000), mLastSyncTime(0), mServerTime(0){
    
}

void NetTime::UpdateAverageLatency() {
    //sort the ping
    //get the median
    //discard the worst (2*median)
    //recompute the mean value
    uint64 temp[PING_AVERAGE_SIZE];
    memcpy(temp, mPings, sizeof(mPings));
    std::sort(temp, temp+PING_AVERAGE_SIZE);
    int64 const meanValue = temp[PING_AVERAGE_SIZE/2];
    uint32 count = 0;
    uint64 sum = 0;
    for(int i=0;i<PING_AVERAGE_SIZE;++i) {
        int64 ping = temp[i];
        int64 delta = ping - meanValue;
        if((delta < -2*meanValue) || (delta > 2*meanValue)) {
            continue;
        }
        sum += ping;
        count+=2;
    }
    if(count > 0) {
        mEstimatedLatency = sum / count;
        Debug::Log("NET", "Estimated latency: %d", mEstimatedLatency);
    }
    else {
        Debug::Error("NET", "Sample Count == 0. Skip ping estimate");
    }
}
