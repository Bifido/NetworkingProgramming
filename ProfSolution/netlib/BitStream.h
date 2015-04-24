//
//  BitStream.h
//  superasteroids
//
//  Created by Cristian Marastoni on 31/05/14.
//
//

#pragma once
#include <string.h>
#include "NetTypes.h"
#include <string>

class BitStream {
public:
    BitStream();
    BitStream(uint8 *data, size_t size, bool read=false);
    BitStream(const uint8 *data, size_t size);
    virtual ~BitStream();
    const uint8 *buffer() const;
    const uint8 *read_ptr() const;
    uint32 size() const;
    void seek(int32 nbits) const;
    void skip(uint32 nbits) const;
    void skip_bytes(uint32 nbytes) const;
    uint32 bitpos() const;
    uint32 bytepos() const;
    uint32 remaining_bytes() const;
    bool eof() const;
    void flush();

    void setData(uint8 *data, size_t size);
    void setData(uint8 const *data, size_t size);

    void subStream(uint32 bitPos, uint32 size, BitStream &out) const;
    void copyData(uint32 bytePos, uint32 size, void *out) const;
    
    BitStream &pack_s8(char data);
    BitStream &pack_s16(int16 data);
    BitStream &pack_s32(int32 data);
    BitStream &pack8(uint8 data);
    BitStream &pack16(uint16 data);
    BitStream &pack32(uint32 data);
    BitStream &pack64(uint64 data);
    BitStream &packFloat(float data);
    BitStream &packString(const char *str);
    BitStream &packString(std::string const &str);
    BitStream &packData(void const *data, size_t size);

    
    //Attention! data must be byte aligned to append the content (for both performance and correctness)
    BitStream &append(void const *data, size_t size);
    BitStream &align();
    BitStream const &align() const;
    
    BitStream const &unpack8(uint8 &data) const;
    BitStream const &unpack16(uint16 &data) const;
    BitStream const &unpack32(uint32 &data) const;
    BitStream const &unpack64(uint64 &data) const;
    BitStream const &unpackFloat(float &data) const;

    BitStream const &unpack_s8(char &data) const;
    BitStream const &unpack_s16(int16 &data) const;
    BitStream const &unpack_s32(int32 &data) const;
    BitStream const &unpackString(char *buffer, uint32 bufferSize) const;
    BitStream const &unpackString(std::string &str) const;
    uint32 unpackData(uint8 *buffer, uint32 bufferSize) const;
    
private:
    BitStream &packBits(unsigned int data, int nBits);
    uint32 unpackBits(int nBits) const ;
    void writeToBuffer(uint32 bits);
    void readFromBuffer(uint32 *bits) const;
    void cacheBits(uint32 *bits) const;
private:
    union {
        uint8 *mWriteBuffer;
        uint8 const *mReadBuffer;
    };
    uint32 mBufferSize;
    uint32 mBitCount;
    mutable uint32 mBitPos;
    mutable uint32 mBitLeft;
    mutable uint32 mBitBuf;
};


template<int SIZE>
class NetStackData : public BitStream {
public:
    NetStackData() : BitStream(mBuffer, SIZE) {
    }
    uint32 capacity() const {
        return SIZE;
    }
private:
    uint8 mBuffer[SIZE];
};
