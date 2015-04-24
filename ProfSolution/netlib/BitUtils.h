//
//  BitUtils.h
//  superasteroids
//
//  Created by Cristian Marastoni on 12/06/14.
//
//

#pragma once

static inline uint64 BIT(uint64 x, int bit) {
    return x & (1LL << bit);
}
static inline uint32 BIT(uint32 x, int bit) {
    return x & (1 << bit);
}
static inline uint32 BITMASK(int nbits) {
    return (~0u >> (32 - nbits));
}
static inline uint32 maskbits_32(uint32 x, int nbits) {
    const uint32 mask = BITMASK(nbits);
    return x & mask;
}
