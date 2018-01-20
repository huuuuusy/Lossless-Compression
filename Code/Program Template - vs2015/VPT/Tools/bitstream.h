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


#ifndef BITSTREAM_H
#define BITSTREAM_H

// Data structure for Bitstream
typedef struct BitStream {
	unsigned char *start ;		// Pointer to the starting address of the bitstream buffer
	unsigned char *wCur ;		// Pointer to the current address of the bitstream buffer for writing
	unsigned char *rCur ;		// Pointer to the current address of the bitstream buffer for reading
	unsigned char *end ;		// Pointer to the ending address of the bitstream buffer

	unsigned int wBytePos ;		// The byte aligned offset from the starting address for writing the next byte
	unsigned int wBitPos ;		// The bit offset from the byte aligned boundary for writing the next bit
	unsigned int rBytePos ;		// The byte aligned offset from the starting address for reading the next byte
	unsigned int rBitPos ;		// The bit offset from the byte aligned boundary for reading the next bit
	unsigned int nByte ;		// Number of not yet read bytes in the bitstream buffer
	unsigned int nBit ;			// Number of not yet read bits within the ***current byte***
	unsigned int size ;			// Size of the bitstream buffer
} BITSTREAM ;

#ifdef __cplusplus
extern "C" {
#endif

// This function inits a bitstream structure and associate internal data
// to a pre-allocated buffer bsBuf with size bufSize
void bsInit(BITSTREAM *bs, unsigned char *bsBuf, unsigned int bufSize) ;

// This function clear the bitstream buffers. The bitstream will be empty after calling
void bsClear(BITSTREAM *bs) ;

// This function tests whether the current bitstream is full (i.e. No more write should be done)
int bsFull(BITSTREAM *bs) ;

// This function tests whether the current bitstream is empty (i.e. No more read could be done)
int bsEmpty(BITSTREAM *bs) ;

// This function returns the bitstream size in bytes (rounded above, e.g. 9 bits -> 2 bytes in size)
int bsRoundedSizeInBytes(BITSTREAM *bs) ;

// This function returns the bitstream size in bits
int bsFilledSizeInBits(BITSTREAM *bs) ;

// This function returns the size of the associated bitstream buffer
int bsBufSizeInBytes(BITSTREAM *bs) ;

// This function returns the fullness of the bitstream buffer in terms of percentage
int bsFullness(BITSTREAM *bs) ;

// This function write out the data of size bitCount to the bitstream bs
int bsWrite(BITSTREAM *bs, unsigned int data, unsigned int bitCount) ;

// This function write out the data of size byteCount to the bitstream bs, this function will deny
// writing data into the bitstream if the data in the current bitstream are not byte-aligned
// Return the number of bits that has been successfully written to bs
int bsWriteByteStreamAligned(BITSTREAM *bs, unsigned char *data, unsigned int byteCount) ;

// This function read out the data of size bitCount from the bitstream bs
int bsRead(BITSTREAM *bs, unsigned int *data, unsigned int bitCount) ;

// bsPeek(...) is equivalent to next_bits(n) in ISO/IEC 14496-10:2003(E) Section 7.2
int bsPeek(BITSTREAM *bs, unsigned int *data, unsigned int bitCount) ;

// Check whether the next few bytes (number defined by byteCount) indicates a startCode
// Return 1 if startCode is found, otherwise return 0
int bsPeekStartCode(BITSTREAM *bs, unsigned int startCode, unsigned int byteCount) ;

// Read the next few byte-aligned bytes (number defined by byteCount) without altering internal
// reading pointer
// Return 1 if succeed, otherwise return 0
int bsPeekBytes(BITSTREAM *bs, unsigned int *data, unsigned int byteCount) ;

// Read the next byte-aligned single byte
// Return 1 if succeed, otherwise return 0
int bsReadByte(BITSTREAM *bs, unsigned int *data) ;

// Write the next byte-aligned single byte
// Return 1 if succeed, otherwise return 0
int bsWriteByte(BITSTREAM *bs, unsigned int data) ;

// This functions read and return the number of zeros before reading the bit '1'.
// The first bit '1' encountered will also be removed from the bitstream 
// e.g. Bitstream 0000100010101...., The call will return 4 (4 zeros preceding a '1')
//		and the bitstream becomes 00010101....
int bsReadZerosAndOne(BITSTREAM *bs) ;

// Function to read an unsigned Exp-Golomb Code from the bitstream bs
// See ISO/IEC 14496-10:2003(E) 9.1 for details
int bsRead_ue(BITSTREAM *bs) ;

// Function to read a signed Exp-Golomb Code from the bitstream bs
// See ISO/IEC 14496-10:2003(E) 9.1.1 for details
int bsRead_se(BITSTREAM *bs) ;

// Function to read a truncated Exp-Golomb Code from the bistream bs
// See ISO/IEC 14496-10:2003(E) 9.1 for details
int bsRead_te(BITSTREAM *bs, int x) ;

// Helper function to determine the size (in bits) of an unsigned Exp-Golomb Code
int bsSize_ue(unsigned int v) ;

// Helper function to determine the size (in bits) of a signed Exp-Golomb Code
int bsSize_ve(int v) ;

// Function to write out an unsigned Exp-Golomb Code to the bitstream bs
int bsWrite_ue(BITSTREAM *bs, unsigned int v) ;

// Function to write out a truncated Exp-Golomb Code to the bitstream bs
void bsWrite_te(BITSTREAM *bs, unsigned int x, unsigned int v) ;

// Function to write out a signed Exp-Golomb Code to the bitstream bs
void bsWrite_se(BITSTREAM *bs, int v) ;

// Boolean Function to tell whether the next bit to be read is at a byte aligned boundary
int bsReadByteAligned(BITSTREAM *bs) ;

// Boolean Function to tell whether the next bit to be written is at a byte aligned boundary
int bsWriteByteAligned(BITSTREAM *bs) ;

// Function to read out padding trailing bits from the bitstream bs
// Padding trailing bits is used to ensure an NAL unit is always end at a byte aligned boundary
// See ISO/IEC 14496-10:2003(E) 7.3.2.11 for details
int bsRBSPReadTrailing(BITSTREAM *bs) ;

// Function to write out padding trailing bits to the bitstream bs
void bsRBSPWriteTrailing(BITSTREAM *bs) ;

// Function to write out Mapped Code of Coded Block Pattern to the bistream bs
// See ISO/IEC 14496-10:2003(E) 9.1.2 for details
void bsWrite_me(BITSTREAM *bs, int cbp, int mode) ;

// Function to read out Mapped Code of Coded Block Pattern from the bistream bs
int bsRead_me(BITSTREAM *bs, int mode) ;

#ifdef __cplusplus
}
#endif

#endif
