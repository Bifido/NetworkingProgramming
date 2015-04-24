//
//  BitStream.cpp
//  superasteroids
//
//  Created by Cristian Marastoni on 31/05/14.
//
//
#include "NetTypes.h"
#include "BitStream.h"
#include "BitUtils.h"
#include "../Debug.h"

BitStream::BitStream() : mBitLeft(32), mBitBuf(0), mBitCount(0), mBitPos(0)
{
}
BitStream::BitStream(const uint8 *data, size_t size) : mBitLeft(32), mBitBuf(0), mBitPos(0)
{
    setData(data, size);
}
BitStream::BitStream(uint8 *data, size_t size, bool read) : mBitLeft(32), mBitBuf(0),mBitPos(0)
{
    //in write mode open both for read an writing
    if(read) {
        setData((uint8 const*)data, size);
    }else {
        setData(data, size);
    }
}
BitStream::~BitStream() {
    
}

//Always return the read buffer
const uint8 *BitStream::buffer() const {
    return mReadBuffer;
}
const uint8 *BitStream::read_ptr() const {
    return mReadBuffer + bitpos() / 8;
}
uint32 BitStream::size() const {
    if(mBitCount  == 0) {
        return 0;
    }else {
        return (mBitCount + 7) / 8;
    }
}
uint32 BitStream::bitpos() const {
    return mBitPos + 32 - mBitLeft;
}
uint32 BitStream::bytepos() const {
    return bitpos() >> 3;
}

bool BitStream::eof() const {
    return mBitPos >= mBufferSize;
}

uint32 BitStream::remaining_bytes() const {
    return (mBufferSize - bitpos()) / 8;
}

void BitStream::seek(int32 bit) const {
    mBitLeft = 32 - (bit & 31);
    mBitPos = bit - (bit & 31);
    cacheBits(&mBitBuf);
}
void BitStream::skip(uint32 nbits) const {
    if(nbits < mBitLeft) {
        mBitLeft -= nbits;
    }else {
        seek(mBitPos + 32 - mBitLeft + nbits);
    }
}
void BitStream::skip_bytes(uint32 nbytes) const {
    skip(nbytes * 8);
}


void BitStream::flush() {
    if(mBitLeft < 32) {
        mBitBuf <<= mBitLeft;
        writeToBuffer(mBitBuf);
        mBitBuf = 0;
        mBitLeft = 32;
    }
}

void BitStream::setData(uint8 *data, size_t size) {
    mWriteBuffer = data;
    mBufferSize = size*8;
    mBitPos = 0;
    mBitCount = 0;
    mBitLeft = 32;
    mBitBuf = 0;
    memset(data, 0, size);
}
void BitStream::setData(uint8 const *data, size_t size) {
    mReadBuffer = data;
    mBufferSize = size*8;
    mBitPos = 0;
    mBitCount = mBufferSize;
    cacheBits(&mBitBuf);
}

void BitStream::subStream(uint32 bytePos, uint32 size, BitStream &out) const {
    //IwAssert("NET", (bytePos + size) < mBufferSize);
    out.setData(mReadBuffer + bytePos, size);
}

void BitStream::copyData(uint32 bytePos, uint32 size, void *out) const {
    //IwAssert("NET", (bytePos + size) <= (mBitCount/8));
    memcpy(out, mReadBuffer + bytePos, size);
}

//probably is much better to write the data aligned. So flush the buffer bits after encoding the
//data size
BitStream &BitStream::packData(void const *data, size_t size) {
    int neededBits = 16 + (8 - mBitLeft & 0x7);
    packBits(size, neededBits);
    uint32 bitLeft = mBitLeft;
    uint8 const *ptr = reinterpret_cast<uint8 const*>(data);
    while(mBitLeft < 32 && size > 0) {
        packBits(*ptr, 8);
        --size;
        ++ptr;
    }
    if(size > 0) {
        uint8 *outPtr = mWriteBuffer + mBitPos/8;
        memcpy(outPtr, ptr, size);
        mBitPos += size*8;
        mBitCount += size*8;
    }
    return *this;
}

BitStream &BitStream::append(void const *data, size_t size) {
    //align();
    uint32 bitLeft = mBitLeft;
    uint8 const *ptr = reinterpret_cast<uint8 const*>(data);
    while(mBitLeft < 32 && size > 0) {
        packBits(*ptr, 8);
        --size;
        ++ptr;
    }
    if(size > 0) {
        uint8 *outPtr = mWriteBuffer + mBitPos/8;
        memcpy(outPtr, ptr, size);
        mBitPos += size*8;
        mBitCount += size*8;
    }
    return *this;
}

BitStream &BitStream::align() {
    int skipBits = (8 - mBitLeft & 0x7);
    if(skipBits > 0) {
        packBits(0, skipBits);
    }
    return *this;
}
BitStream const &BitStream::align() const {
    int skipBits = (8 - mBitLeft & 0x7);
    if(skipBits >= 0) {
        unpackBits(skipBits);
    }
    return *this;
}

BitStream &BitStream::pack_s8(char data) {
    return packBits(data, 8);
}
BitStream &BitStream::pack_s16(int16 data) {
    return packBits(data, 16);
}
BitStream &BitStream::pack_s32(int32 data) {
    return packBits(data, 32);
}

BitStream &BitStream::pack8(uint8 data) {
    return packBits(data, 8);
}
BitStream &BitStream::pack16(uint16 data) {
    return packBits(data, 16);
}
BitStream &BitStream::pack32(uint32 data) {
    return packBits(data, 32);
}
BitStream &BitStream::pack64(uint64 data) {
    packBits(static_cast<uint32>((data >> 32) & 0xFFFFFFFF), 32);
    packBits(static_cast<uint32>(data & 0xFFFFFFFF), 32);
    return *this;
}
BitStream &BitStream::packFloat(float data) {
    union {
        float fValue;
        uint32 iValue;
    }conv;
    conv.fValue = data;
    packBits(conv.iValue, sizeof(data));
    return *this;
}
BitStream &BitStream::packString(const char *str) {
    size_t slen = strlen(str);
    packData(str, slen);
    return *this;
}

BitStream const &BitStream::unpack8(uint8 &data) const {
    uint32 temp = unpackBits(8);
    data = static_cast<uint8>(temp);
    return *this;
}
BitStream const &BitStream::unpack16(uint16 &data) const  {
    uint32 temp = unpackBits(16);
    data = static_cast<uint16>(temp);
    return *this;
}
BitStream const &BitStream::unpack32(uint32 &data) const {
    data = unpackBits(32);
    return *this;
    
}
BitStream const &BitStream::unpack64(uint64 &data) const {
    data = unpackBits(32);
    data <<= 32;
    data |= unpackBits(32);
    return *this;
}
BitStream const &BitStream::unpackFloat(float &data) const {
    union {
        float fValue;
        uint32 iValue;
    }conv;
    conv.iValue = unpackBits(32);
    data = conv.fValue;
    return *this;
}

BitStream const &BitStream::unpack_s8(char &data) const {
    uint32 temp = unpackBits(8);
    data = static_cast<char>(temp);
    return *this;
}
BitStream const &BitStream::unpack_s16(int16 &data) const {
    uint32 temp = unpackBits(16);
    data = static_cast<int16>(temp);
    return *this;
}
BitStream const &BitStream::unpack_s32(int32 &data) const {
    data = static_cast<int32>(unpackBits(32));
    return *this;
}
BitStream const &BitStream::unpackString(char *buffer, uint32 bufferSize) const {
    uint32 size = unpackData((uint8*)buffer, bufferSize);
    buffer[size]=0;
    return *this;
}

BitStream const &BitStream::unpackString(std::string &str) const {
    int neededBits = 16 + (8 - mBitLeft & 0x7);
    //This will align the data to the byte boundary
    uint32 size = unpackBits(neededBits);
    if(size == 0) {
        return *this;
    }
    str.reserve(size);
    while(size > 0) {
        str.push_back(static_cast<char>(unpackBits(8)));
        --size;
    }
    return *this;
}


uint32 BitStream::unpackData(uint8 *data, uint32 bufferSize) const {
    int neededBits = 16 + (8 - mBitLeft & 0x7);
    uint32 size = unpackBits(neededBits);
    if(size > bufferSize) {
        size = bufferSize;
    }
    uint32 bitLeft = mBitLeft;
    uint8 *ptr = reinterpret_cast<uint8 *>(data);
    while(mBitLeft < 32 && size > 0) {
        *ptr = static_cast<uint8>(unpackBits(8));
        --size;
        ++ptr;
    }
    if(size > 0) {
        uint8 const *inPtr = mReadBuffer + mBitPos/8;
        memcpy(ptr, inPtr, size);
        ptr += size;
        mBitPos += size*8;
    }
    return static_cast<uint32>(ptr - data);
}

BitStream &BitStream::packBits(unsigned int data, int nBits) {
    uint32 bitBuf = mBitBuf;
    int32 bitLeft = mBitLeft;
    if(nBits <= bitLeft) {
        if(nBits == 32) {
            bitBuf = 0;
        }else {
            bitBuf <<= nBits;
        }
        bitBuf |= maskbits_32(data, nBits);
        bitLeft -= nBits;
        if(bitLeft == 0) {
            writeToBuffer(bitBuf);
            bitBuf = 0;
            bitLeft = 32;
        }
    }else {
        int dbits = nBits - bitLeft;
        bitBuf <<= bitLeft;
        bitBuf |= (data >> dbits);
        writeToBuffer(bitBuf);
        bitBuf = maskbits_32(data, dbits);
        bitLeft += 32 - nBits;
    }
    mBitLeft = static_cast<uint32 >(bitLeft);
    mBitBuf = bitBuf;
    mBitCount += nBits;
    return *this;
}

uint32 BitStream::unpackBits(int nBits) const {
    unsigned int data = 0;
    uint32 bitBuf =  mBitBuf;
    int bitLeft = mBitLeft;
    const uint32 mask = BITMASK(nBits);
    if(nBits <= bitLeft) {
        data = bitBuf >> (bitLeft - nBits);
        data &= mask;
        bitLeft -= nBits;
        if(bitLeft == 0) {
            readFromBuffer(&bitBuf);
            bitLeft = 32;
        }
    }else {
        int dbits = nBits - bitLeft;
        data = bitBuf << dbits;
        readFromBuffer(&bitBuf);
        bitLeft += 32 - nBits;
        data |= (bitBuf >> bitLeft);
        data &= mask;
    }
    mBitLeft = static_cast<uint32>(bitLeft);
    mBitBuf = bitBuf;
    //mBitCount += nBits;
    return data;
}

void BitStream::writeToBuffer(uint32 bits) {
    if(mBitPos <= (mBufferSize-32)) {
        //I should write byte in big endian order,
        uint8 *inPtr  = (uint8*)&bits;
        uint8 *outPtr = mWriteBuffer + (mBitPos / 8);
    #if __BYTE_ORDER == __BIG_ENDIAN
        outPtr[0] = inPtr[0];
        outPtr[1] = inPtr[1];
        outPtr[2] = inPtr[2];
        outPtr[3] = inPtr[3];
    #else
        outPtr[0] = inPtr[3];
        outPtr[1] = inPtr[2];
        outPtr[2] = inPtr[1];
        outPtr[3] = inPtr[0];
    #endif
        mBitPos += 32;
    }else {
        Debug::Error("BITSTREAM", "Not enought space in buffer");
    }
}

void BitStream::cacheBits(uint32 *bits) const {
    //I should write byte in big endian order,
    uint8 *outPtr  = (uint8*)bits;
    uint8 const *inPtr = mReadBuffer + (mBitPos / 8);
#if __BYTE_ORDER == __BIG_ENDIAN
    outPtr[0] = inPtr[0];
    outPtr[1] = inPtr[1];
    outPtr[2] = inPtr[2];
    outPtr[3] = inPtr[3];
#else
    outPtr[0] = inPtr[3];
    outPtr[1] = inPtr[2];
    outPtr[2] = inPtr[1];
    outPtr[3] = inPtr[0];
#endif

}

void BitStream::readFromBuffer(uint32 *bits) const {
    //is allowed to bitPos to go beyond the buffersize end (because of the alignment)
    if(mBitPos < mBufferSize) {
        mBitPos += 32;
        cacheBits(bits);
    }
}
