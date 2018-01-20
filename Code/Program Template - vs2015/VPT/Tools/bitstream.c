//	===========================================================
//
//	Copyright 2006-2008 by CyberView Inc. All rights reserved.
//
//	It is prohibited to disclose or redistribute any portion
//	of the code in this file without prior permission from
//	CyberView Inc.
//
//	===========================================================

// bitstream.h/bitstream.c
// This is a module that provides utilities and data structure to perform bitstream manipulations in a codec instance

#include <string.h>
#include <stdlib.h>
#include "bitstream.h"

#ifdef __cplusplus
extern "C" {
#endif

// Lookup table to expedite the coding of Exp-Golomb Code
// Further revised to include 65535 and 131071, as there is a bug discovered in encoding
//		pEnc->iFrameNum
unsigned int ue_range[18] = {
		0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767, 65535, 131071} ;

// Lookup tables for mapped code of Coded Block Pattern
// See ISO/IEC 14496-10:2003(E) 9.1.2 for details

// For Writing...
short Intra4x4Me_w[2][48]	= {{3, 29, 30, 17, 31, 18, 37, 8, 32, 38,
								19, 9, 20, 10, 11, 2, 16, 33, 34, 21,
								35, 22, 39, 4, 36, 40, 23, 5, 24, 6,
								7, 1, 41, 42, 43, 25, 44, 26, 46, 12,
								45, 47, 27, 13, 28, 14, 15, 0},
							  {0, 2, 3, 7, 4, 8, 17, 13, 5, 18, 9,
								14, 10, 15, 16, 11, 1, 32, 33, 36, 34,
								37, 44, 40, 35, 45, 38, 41, 39, 42, 43,
								19, 6, 24, 25, 20, 26, 21, 46, 28, 27,
								47, 22, 29, 23, 30, 31, 12}} ;

// For Reading...
short Intra4x4Me_r[2][48]	= {{47, 31, 15, 0, 23, 27, 29, 30, 7, 11,
								13, 14, 39, 43, 45, 46, 16, 3, 5, 10,
								12, 19, 21, 26, 28, 35, 37, 42, 44, 1,
								2, 4, 8, 17, 18, 20, 24, 6, 9, 22,
								25, 32, 33, 34, 36, 40, 38, 41},
							  {0, 16, 1, 2, 4, 8, 32, 3, 5, 10,
								12, 15, 47, 7, 11, 13, 14, 6, 9, 31,
								35, 37, 42, 44, 33, 34, 36, 40, 39, 43,
								45, 46, 17, 18, 20, 24, 19, 21, 26, 28,
								23, 27, 29, 30, 22, 25, 38, 41}} ;

// This function inits a bitstream structure and associate internal data
// to a pre-allocated buffer bsBuf with size bufSize
void bsInit(BITSTREAM *bs, unsigned char *bsBuf, unsigned int bufSize) {
	bs->start = bsBuf ;
	bs->wCur = bsBuf ;
	bs->rCur = bsBuf ;
	bs->end = bsBuf + bufSize ;

	bs->size = bufSize ;
	bs->wBytePos = 0 ;
	bs->wBitPos = 0 ;
	bs->rBytePos = 0 ;
	bs->rBitPos = 0 ;
	bs->nByte = 0 ;
	bs->nBit = 0 ;
}

// This function clear the bitstream buffers. The bitstream will be empty after calling
void bsClear(BITSTREAM *bs) {

	bs->wCur = bs->start ;
	bs->rCur = bs->start ;

	bs->wBytePos = 0 ;
	bs->wBitPos = 0 ;
	bs->rBytePos = 0 ;
	bs->rBitPos = 0 ;
	bs->nByte = 0 ;
	bs->nBit = 0 ;
}

// This function tests whether the current bitstream is full (i.e. No more write should be done)
int bsFull(BITSTREAM *bs) {

	return ((bs->nByte << 3) + bs->nBit) >= (bs->size << 3) ;
}

// This function tests whether the current bitstream is empty (i.e. No more read could be done)
int bsEmpty(BITSTREAM *bs) {

	return ((bs->nByte << 3) + bs->nBit == 0) ;
}

// This function returns the bitstream size in bytes (rounded above, e.g. 9 bits -> 2 bytes in size)
int bsRoundedSizeInBytes(BITSTREAM *bs) {

	return (((bs->nByte << 3) + bs->nBit + 7) >> 3) ;
}

// This function returns the bitstream size in bits
int bsFilledSizeInBits(BITSTREAM *bs) {
	
	return ((bs->nByte << 3) + bs->nBit) ;
}

// This function returns the size of the associated bitstream buffer
int bsBufSizeInBytes(BITSTREAM *bs) {

	return bs->size ;
}

// This function returns the fullness of the bitstream buffer in terms of percentage
int bsFullness(BITSTREAM *bs) {
	return bsFilledSizeInBits(bs) * 100 / (bsBufSizeInBytes(bs) * 8) ;
}

int bsUnusedBits(BITSTREAM *bs) {

	return ((bs->size << 3) - (((bs->nByte << 3) + bs->nBit))) ;
}

// This function write out the data of size bitCount to the bitstream bs
// Return the number of bits that has been successfully written to bs
int bsWrite(BITSTREAM *bs, unsigned int data, unsigned int bitCount) {

	unsigned int wBitCount ;
	unsigned int tBitCount ;
	unsigned int uBitCount ;
	unsigned char mask ;


	uBitCount = bsUnusedBits(bs) ;
	if(uBitCount < bitCount) {
		bitCount = uBitCount ;
	}

	tBitCount = 0 ;

//	while(bitCount > 0 && !bsFull(bs)) {
	while(bitCount > 0) {

		wBitCount = (8 - bs->wBitPos) > bitCount ? bitCount : (8 - bs->wBitPos) ;
		mask = (1 << wBitCount) - 1 ;
		mask <<= (8 - bs->wBitPos - wBitCount) ;
		*bs->wCur = (*bs->wCur & ((0x000000FF << (8 - bs->wBitPos)) & 0x000000FF)) | (((data >> (bitCount - wBitCount)) << (8 - bs->wBitPos - wBitCount)) & mask) ;
		bitCount -= wBitCount ;						// Update the remainding bit for processing
		bs->wBitPos += wBitCount ;					// Update the number of bits written
		bs->wBytePos += (bs->wBitPos >> 3) ;
		bs->wCur += (bs->wBitPos >> 3) ;				// Update the current pointer for writing
		bs->nBit += wBitCount ;
		bs->nByte += (bs->nBit >> 3) ;
//		bs->nBit %= 8 ;
		bs->nBit &= 7 ;
		tBitCount += wBitCount ;
//		bs->wBitPos %= 8 ;
		bs->wBitPos &= 7 ;
		if(bs->wCur >= bs->end) {
			bs->wBytePos = 0 ;						// Wrap around the buffer.. kind of circular buffer mechanism
			bs->wCur = bs->start ;
		}
	}

	return tBitCount ;
}

// This function write out the data of size byteCount to the bitstream bs, this function will deny
// writing data into the bitstream if the data in the current bitstream are not byte-aligned
// Return the number of bits that has been successfully written to bs
int bsWriteByteStreamAligned(BITSTREAM *bs, unsigned char *data, unsigned int byteCount) {

	unsigned int firstHalfCount ;
	unsigned int uBitCount ;

	if(bs->wBitPos) {
		return 0 ;	// Not byte aligned, deny writing to the bitstream
	}

	uBitCount = bsUnusedBits(bs) ;
	if(uBitCount < (byteCount << 3)) {
		byteCount = (uBitCount >> 3) ;
	}


	firstHalfCount = (unsigned int) (bs->end - bs->wCur) ;

	if(firstHalfCount > byteCount) {
		memcpy(bs->wCur, data, byteCount) ;
		bs->wCur += byteCount ;
		bs->wBytePos += byteCount ;
		bs->nByte += byteCount ;
	} else if(firstHalfCount == byteCount) {
		memcpy(bs->wCur, data, byteCount) ;
		bs->wCur = bs->start ;
		bs->wBytePos = 0 ;
		bs->nByte += byteCount ;
	} else if(firstHalfCount < byteCount) {
		memcpy(bs->wCur, data, firstHalfCount) ;
		bs->wCur = bs->start ;
		bs->wBytePos = 0 ;
		memcpy(bs->wCur, data + firstHalfCount, byteCount - firstHalfCount) ;
		bs->wCur += (byteCount - firstHalfCount) ;
		bs->wBytePos += (byteCount - firstHalfCount) ;
		bs->nByte += byteCount ;
	}

	return byteCount ;
}

// This function read out the data of size bitCount from the bitstream bs
// Return the number of bits that has been successfully read from bs
int bsRead(BITSTREAM *bs, unsigned int *data, unsigned int bitCount) {

	unsigned int rdata ;
	unsigned int rBitCount ;
	unsigned int tBitCount ;
	unsigned int fBitCount ;

	fBitCount = bsFilledSizeInBits(bs) ;

	if(fBitCount < bitCount) {
		bitCount = fBitCount ;
	}

	tBitCount = 0 ;
	*data = 0 ;

	while(bitCount > 0) {
		rdata = (unsigned int) *bs->rCur ;
		rBitCount = ((8 - bs->rBitPos) > bitCount) ? bitCount : (8 - bs->rBitPos) ;
		rdata = ((rdata << bs->rBitPos) & 0x000000FF) >> (8 - rBitCount) ; 
		*data = ((*data) << rBitCount) + rdata ;
		tBitCount += rBitCount ;
		bs->rBitPos += rBitCount ;
		bs->rBytePos += (bs->rBitPos >> 3) ;
		bs->rCur += (bs->rBitPos >> 3) ;
//		bs->rBitPos %= 8 ;
		bs->rBitPos &= 7 ;
//		bs->nBit = (bs->nBit + 8 - rBitCount) % 8 ;
		bs->nBit = (bs->nBit + 8 - rBitCount) & 7 ;
		bs->nByte -= ((rBitCount + bs->nBit) >> 3) ;
		bitCount -= rBitCount ;
		if(bs->rCur >= bs->end) {
			bs->rBytePos = 0 ;
			bs->rCur = bs->start ;
		}
	}
	return tBitCount ;
}

// bsPeek(...) is equivalent to next_bits(n) in ISO/IEC 14496-10:2003(E) Section 7.2
// This function is useful for reading the next 'bitCount' bits without altering the
// internal reading pointer
int bsPeek(BITSTREAM *bs, unsigned int *data, unsigned int bitCount) {

	BITSTREAM outBs ;
	BITSTREAM dupBs ;

	bsInit(&outBs, (unsigned char *) data, sizeof(unsigned int)) ;
	memcpy(&dupBs, bs, sizeof(BITSTREAM)) ;

	if(bsRead(&dupBs, data, bitCount) != bitCount) {
		*data = 0 ;
		return 0 ;
	}
	return bitCount ;
}

// Check whether the next few byte-aligned bytes (number defined by byteCount) indicates a startCode
// Return 1 if startCode is found, otherwise return 0
int bsPeekStartCode(BITSTREAM *bs, unsigned int startCode, unsigned int byteCount) {

	unsigned char rStartCode ;
	unsigned int i ;

	if(bs->nByte < byteCount) return 0 ;

	i = 0 ;
	rStartCode = 0 ;
	while(i < byteCount) {
		rStartCode <<= 8 ;
		rStartCode |= bs->start[(bs->rBytePos + i) % bs->size] ;
		i++ ;
	}

	if(rStartCode == startCode) return 1 ;
	else return 0 ;
}

// Read the next few byte-aligned bytes (number defined by byteCount) without altering internal
// reading pointer
// Return 1 if succeed, otherwise return 0
int bsPeekBytes(BITSTREAM *bs, unsigned int *data, unsigned int byteCount) {

	unsigned char *pBytes ;
	unsigned int i ;

	pBytes = (unsigned char *) data ;

	if(bs->nByte < byteCount) return 0 ;

	i = 0 ;
	*data = 0 ;
	pBytes += (byteCount - 1) ;
	while(i < byteCount) {
		*(pBytes--) = bs->start[(bs->rBytePos + i) % bs->size] ;
		i++ ;
	}
	return 1 ;
}

// Read the next byte-aligned single byte
// Return 1 if succeed, otherwise return 0
int bsReadByte(BITSTREAM *bs, unsigned int *data) {

	if(bs->nByte < 1) return 0 ;

	*data = *bs->rCur ;
	bs->rCur++ ;
	bs->rBytePos++ ;
	if(bs->rBitPos) {
		bs->nBit = (bs->nBit - (8 - bs->rBitPos) + 8) % 8 ;
		bs->rBitPos = 0 ;
	} else {
		bs->nByte-- ;
	}
	if(bs->rCur >= bs->end) {
		bs->rBytePos = 0 ;
		bs->rCur = bs->start ;
	}
	return 1 ;
}

// Write the next byte-aligned single byte
// Return 1 if succeed, otherwise return 0
int bsWriteByte(BITSTREAM *bs, unsigned int data) {

	unsigned int uBitCount ;

	uBitCount = bsUnusedBits(bs) ;
	if(uBitCount < 8) {
		return 0 ;
	}

	*bs->wCur = data ;
	bs->wCur++ ;
	bs->wBytePos++ ;
	if(bs->wBitPos) {
		bs->nBit = (bs->nBit + (8 - bs->wBitPos)) % 8 ;
		bs->wBitPos = 0 ;
	}
	bs->nByte++ ;
	if(bs->wCur >= bs->end) {
		bs->wBytePos = 0 ;
		bs->wCur = bs->start ;
	}
	return 1 ;
}

// This functions read and return the number of zeros before reading the bit '1'.
// The first bit '1' encountered will also be removed from the bitstream 
// e.g. Bitstream 0000100010101...., The call will return 4 (4 zeros preceding a '1')
//		and the bitstream becomes 00010101....
// Return 0x80000000 upon error
int bsReadZerosAndOne(BITSTREAM *bs) {

	unsigned int bit ;
	int readOK ;
	int numZeros ;

	numZeros = 0 ;
	readOK = bsRead(bs, &bit, 1) ;
	while(bit == 0 && readOK) {
		numZeros++ ;
		readOK = bsRead(bs, &bit, 1) ;
	}
	if(readOK) {
		return numZeros ;
	} else {
		return 0x80000000 ;	// Error, the read is unsuccessful
	}
}

// Function to read an unsigned Exp-Golomb Code from the bitstream bs
// See ISO/IEC 14496-10:2003(E) 9.1 for details
// Return 0x80000000 upon error
int bsRead_ue(BITSTREAM *bs) {

	int numZeros ;
	unsigned int value ;

	numZeros = bsReadZerosAndOne(bs) ;
	if(bsRead(bs, &value, numZeros) != numZeros) {
		return 0x80000000 ;
	}
	return (1 << numZeros) + value - 1 ;
}

// Function to read a signed Exp-Golomb Code from the bitstream bs
// See ISO/IEC 14496-10:2003(E) 9.1.1 for details
// Return 0x80000000 upon error
int bsRead_se(BITSTREAM *bs) {

	unsigned int codeNum ;
	unsigned int value ;
	int sign ;

	codeNum = bsRead_ue(bs) ;
	if(codeNum == 0x80000000) return 0x80000000 ;
	sign = ((codeNum & 1) << 1) - 1 ;
	value = ((codeNum + 1) >> 1) ;

	return sign * value ;
}

// Function to read a truncated Exp-Golomb Code from the bistream bs
// See ISO/IEC 14496-10:2003(E) 9.1 for details
// Return 0x80000000 upon error
int bsRead_te(BITSTREAM *bs, int x) {

	unsigned int bit ;

	if(x == 1) {
		if(bsRead(bs, &bit, 1)) {
			return 1 - bit ;
		} else {
			return 0x80000000 ;
		}
	} else {
		return bsRead_ue(bs) ;
	}
	return 0x80000000 ;
}

// Helper function to determine the size (in bits) of an unsigned Exp-Golomb Code
int bsSize_ue(unsigned int v) {

	int i, j, m ;

	i = 0 ;
	j = 15 ;
	m = (i + j) >> 1 ;
	while(!(ue_range[m] <= v && v < ue_range[m + 1])) {
		if(v > ue_range[m]) {
			i = m ;
		} else {
			j = m ;
		}
		m = (i + j) >> 1 ;
	}
	return (m << 1) + 1 ;
}

// Helper function to determine the size (in bits) of a signed Exp-Golomb Code
int bsSize_ve(int v) {

	return bsSize_ue(v <= 0 ? (unsigned int) ((-v) << 1) : (unsigned int) ((v << 1) - 1));
}

// Function to write out an unsigned Exp-Golomb Code to the bitstream bs
// Return the number of bits written
int bsWrite_ue(BITSTREAM *bs, unsigned int v) {

	int i, j, m ;
	unsigned int val ;

	i = 0 ;
	j = 15 ;
	m = (i + j) >> 1 ;
	while(!(ue_range[m] <= v && v < ue_range[m + 1])) {
		if(v > ue_range[m]) {
			i = m ;
		} else {
			j = m ;
		}
		m = (i + j) >> 1 ;
	}
	val = (1 << m) + v - ue_range[m] ;
	return bsWrite(bs, val, (m << 1) + 1) ;
}

// Function to write out a truncated Exp-Golomb Code to the bitstream bs
void bsWrite_te(BITSTREAM *bs, unsigned int x, unsigned int v) {

	if( x == 1 )
    {
		bsWrite(bs, ~v, 1) ;
    }
    else if( x > 1 )
    {
        bsWrite_ue(bs, v);
    }
}

// Function to write out a signed Exp-Golomb Code to the bitstream bs
void bsWrite_se(BITSTREAM *bs, int v)
{
    bsWrite_ue(bs, v <= 0 ? (unsigned int) ((-v) << 1) : (unsigned int) ((v << 1) - 1));
}

// Boolean Function to tell whether the next bit to be read is at a byte aligned boundary
int bsReadByteAligned(BITSTREAM *bs) {

	return 1 - ((bs->rBitPos + 7) >> 3) ;
}

// Boolean Function to tell whether the next bit to be written is at a byte aligned boundary
int bsWriteByteAligned(BITSTREAM *bs) {

	return 1 - ((bs->wBitPos + 7) >> 3) ;
}

// Function to read out padding trailing bits from the bitstream bs
// Padding trailing bits is used to ensure an NAL unit is always end at a byte aligned boundary
// See ISO/IEC 14496-10:2003(E) 7.3.2.11 for details
int bsRBSPReadTrailing(BITSTREAM *bs)
{
	unsigned int data ;

    bsRead(bs, &data, 1);
	if(!data) return -1 ;
	if(!bsReadByteAligned(bs)) {
        bsRead(bs, &data, 8 - bs->rBitPos);
		if(data) return -1 ;
    }

	return 0 ;
}

// Function to write out padding trailing bits to the bitstream bs
void bsRBSPWriteTrailing(BITSTREAM *bs)
{
    bsWrite(bs, 1, 1);
    if(!bsWriteByteAligned(bs))
    {
        bsWrite(bs, 0, 8 - bs->wBitPos);
    }
}

// Function to write out Mapped Code of Coded Block Pattern to the bistream bs
// See ISO/IEC 14496-10:2003(E) 9.1.2 for details
void bsWrite_me(BITSTREAM *bs, int cbp, int mode) {
	// mode 0 : Intra_4x4
	// mode 1: Inter

	bsWrite_ue(bs, Intra4x4Me_w[mode][cbp]) ;
}

// Function to read out Mapped Code of Coded Block Pattern from the bistream bs
int bsRead_me(BITSTREAM *bs, int mode) {

	int code ;

	// mode 0 : Intra_4x4
	// mode 1: Inter

	code = bsRead_ue(bs) ;
	if(code > 47) {
		return -1 ;
	}

	return Intra4x4Me_r[mode][code] ;
}

#ifdef __cplusplus
}
#endif
