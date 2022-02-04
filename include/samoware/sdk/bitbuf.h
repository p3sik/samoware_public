
#pragma once

#include <cstdint>
#include <assert.h>
#include <string.h>

namespace bitbuf {
	extern uint32_t littleBits[32];
	extern uint32_t bitWriteMasks[32][33];
	extern uint32_t extraMasks[33];
}

class bf_write {
public:
	bf_write() {
		m_pData = 0;
		m_nDataBytes = 0;
		m_nDataBits = -1; // set to -1 so we generate overflow on any operation
		m_iCurBit = 0;
		m_bOverflow = false;
		m_bAssertOnOverflow = true;
		m_pDebugName = 0;
	}

	void StartWriting(void* pData, int nBytes, int iStartBit = 0, int nBits = -1) {
		// The writing code will overrun the end of the buffer if it isn't dword aligned, so truncate to force alignment
		nBytes &= ~3;

		m_pData = (uint32_t*)pData;
		m_nDataBytes = nBytes;

		if (nBits == -1) {
			m_nDataBits = nBytes << 3;
		} else {
			m_nDataBits = nBits;
		}

		m_iCurBit = iStartBit;
		m_bOverflow = false;
	}

	uint32_t GetNumBitsLeft() {
		return m_nDataBits - m_iCurBit;
	}

	void WriteOneBit(bool bit) {
		if (bit) {
			m_pData[m_iCurBit >> 5] |= bitbuf::littleBits[m_iCurBit & 31];
		} else {
			m_pData[m_iCurBit >> 5] &= ~bitbuf::littleBits[m_iCurBit & 31];
		}

		++m_iCurBit;
	}

	void WriteUInt(uint32_t val, uint32_t numBits) {
		if (GetNumBitsLeft() < numBits) {
			assert("Bitbuf storage is full");
			return;
		}

		int32_t iCurBitMasked = m_iCurBit & 31;
		int32_t iDWord = m_iCurBit >> 5;
		m_iCurBit += numBits;

		// Mask in a dword.
		uint32_t* pOut = &m_pData[iDWord];

		// Rotate number into dword alignment
		val = (val << iCurBitMasked) | (val >> (32 - iCurBitMasked));

		// Calculate bitmasks for first and second word
		uint32_t temp = 1 << (numBits - 1);
		uint32_t mask1 = (temp * 2 - 1) << iCurBitMasked;
		uint32_t mask2 = (temp - 1) >> (31 - iCurBitMasked);

		// Only look beyond current word if necessary (avoid access violation)
		int32_t i = mask2 & 1;
		uint32_t dword1 = pOut[0];
		uint32_t dword2 = pOut[i];

		// Drop bits into place
		dword1 ^= (mask1 & (val ^ dword1));
		dword2 ^= (mask2 & (val ^ dword2));

		// Note reversed order of writes so that dword1 wins if mask2 == 0 && i == 0
		pOut[i] = dword2;
		pOut[0] = dword1;
	}

	void WriteSignedInt(int32_t val, uint32_t nBits) {
		// Force the sign-extension bit to be correct even in the case of overflow.
		int32_t nValue = val;
		int32_t nPreserveBits = (0x7FFFFFFF >> (32 - nBits));
		int32_t nSignExtension = (nValue >> 31) & ~nPreserveBits;
		nValue &= nPreserveBits;
		nValue |= nSignExtension;

		WriteUInt(nValue, nBits);
	}

	void WriteBits(void* pIn, uint32_t nBits) {
		uint8_t* pOut = (uint8_t*)pIn;
		uint32_t nBitsLeft = nBits;

		// Align output to dword boundary
		while (((std::uintptr_t)pOut & 3) != 0 && nBitsLeft >= 8) {
			WriteUInt(*pOut, 8);
			++pOut;
			nBitsLeft -= 8;
		}

		if ((nBitsLeft >= 32) && (m_iCurBit & 7) == 0) {
			// current bit is byte aligned, do block copy
			int32_t numbytes = nBitsLeft >> 3;
			int32_t numbits = numbytes << 3;

			memcpy((char*)m_pData + (m_iCurBit >> 3), pOut, numbytes);
			pOut += numbytes;
			nBitsLeft -= numbits;
			m_iCurBit += numbits;
		}

		// X360TBD: Can't write dwords in WriteBits because they'll get swapped
		if (nBitsLeft >= 32) {
			uint32_t iBitsRight = (m_iCurBit & 31);
			uint32_t iBitsLeft = 32 - iBitsRight;
			uint32_t bitMaskLeft = bitbuf::bitWriteMasks[iBitsRight][32];
			uint32_t bitMaskRight = bitbuf::bitWriteMasks[0][iBitsRight];

			uint32_t* pData = &m_pData[m_iCurBit >> 5];

			// Read dwords.
			while (nBitsLeft >= 32) {
				uint32_t curData = *(uint32_t*)pOut;
				pOut += sizeof(unsigned long);

				*pData &= bitMaskLeft;
				*pData |= curData << iBitsRight;

				pData++;

				if (iBitsLeft < 32) {
					curData >>= iBitsLeft;
					*pData &= bitMaskRight;
					*pData |= curData;
				}

				nBitsLeft -= 32;
				m_iCurBit += 32;
			}
		}

		// write remaining bytes
		while (nBitsLeft >= 8) {
			WriteUInt(*pOut, 8);
			++pOut;
			nBitsLeft -= 8;
		}

		// write remaining bits
		if (nBitsLeft) {
			WriteUInt(*pOut, nBitsLeft);
		}
	}

	void WriteBytes(void* pIn, uint32_t nBytes) {
		WriteBits(pIn, nBytes * 8);
	}

	void WriteChar(char val) {
		WriteSignedInt(val, 8);
	}

	void WriteByte(uint8_t val) {
		WriteUInt(val, 8);
	}

	void WriteShort(int16_t val) {
		WriteSignedInt(val, 16);
	}

	void WriteUnsignedShort(uint16_t val) {
		WriteUInt(val, 16);
	}

	void WriteLong(int32_t val) {
		WriteSignedInt(val, 32);
	}

	void WriteUnsignedLong(uint32_t val) {
		WriteUInt(val, 32);
	}

	void WriteString(const char* pStr) {
		if (!pStr) {
			WriteByte(0);
			return;
		}

		do {
			WriteChar(*pStr++);
		} while (*(pStr - 1) != 0);
	}

	// The current buffer.
	uint32_t* __restrict m_pData;
	int				m_nDataBytes;
	int				m_nDataBits;

	// Where we are in the buffer.
	int				m_iCurBit;

private:
	// Errors?
	bool			m_bOverflow;

	// For debugging..
	bool			m_bAssertOnOverflow;

	const char* m_pDebugName;
};

class bf_read {
public:
	bf_read() {
		m_pData = 0;
		m_nDataBytes = 0;
		m_nDataBits = -1; // set to -1 so we overflow on any operation
		m_iCurBit = 0;
		m_bOverflow = false;
		m_bAssertOnOverflow = true;
		m_pDebugName = 0;
	}

	uint32_t GetNumBitsLeft() {
		return m_nDataBits - m_iCurBit;
	}

	uint8_t ReadOneBit() {
		unsigned int value = ((uint32_t*)m_pData)[m_iCurBit >> 5] >> (m_iCurBit & 31);
		++m_iCurBit;

		return value & 1;
	}

	uint32_t ReadUInt(uint32_t nBits) {
		if (GetNumBitsLeft() < nBits) {
			return 0;
		}

		uint32_t iStartBit = m_iCurBit & 31u;
		int32_t iLastBit = m_iCurBit + nBits - 1;
		uint32_t iWordOffset1 = m_iCurBit >> 5;
		uint32_t iWordOffset2 = iLastBit >> 5;
		m_iCurBit += nBits;

		uint32_t bitmask = (2 << (nBits - 1)) - 1;

		uint32_t dw1 = m_pData[iWordOffset1] >> iStartBit;
		uint32_t dw2 = m_pData[iWordOffset2] << (32 - iStartBit);

		return (dw1 | dw2) & bitmask;
	}

	int32_t ReadSignedInt(uint32_t nBits) {
		uint32_t r = ReadUInt(nBits);
		uint32_t s = 1 << (nBits - 1);

		if (r >= s) {
			r = r - s * 2;
		}

		return r;
	}

	uint32_t ReadVarInt32();

	void ReadBits(void* pOutData, uint32_t nBits) {
		uint8_t* pOut = (uint8_t*)pOutData;
		uint32_t nBitsLeft = nBits;

		// align output to dword boundary
		while (((std::uintptr_t)pOut & 3) != 0 && nBitsLeft >= 8) {
			*pOut = (uint8_t)ReadUInt(8);
			++pOut;
			nBitsLeft -= 8;
		}

		while (nBitsLeft >= 32) {
			*((uint32_t*)pOut) = ReadUInt(32);
			pOut += sizeof(uint32_t);
			nBitsLeft -= 32;
		}

		// read remaining bytes
		while (nBitsLeft >= 8) {
			*pOut = ReadUInt(8);
			++pOut;
			nBitsLeft -= 8;
		}

		// read remaining bits
		if (nBitsLeft) {
			*pOut = ReadUInt(nBitsLeft);
		}
	}

	void ReadBytes(void* pOutData, uint32_t nBytes) {
		ReadBits(pOutData, nBytes * 8);
	}

	char ReadChar() {
		return ReadSignedInt(8);
	}

	uint8_t ReadByte() {
		return ReadUInt(8);
	}

	int16_t ReadShort() {
		return ReadSignedInt(16);
	}

	uint16_t ReadUnsignedShort() {
		return ReadUInt(16);
	}

	int32_t ReadLong() {
		return ReadSignedInt(32);
	}

	uint32_t ReadUnsignedLong() {
		return ReadUInt(32);
	}

	bool ReadString(char* pStr, uint32_t bufLen);

	// The current buffer.
	const uint32_t* __restrict m_pData;
	int						m_nDataBytes;
	int						m_nDataBits;

	// Where we are in the buffer.
	int				m_iCurBit;

private:
	// Errors?
	bool			m_bOverflow;

	// For debugging..
	bool			m_bAssertOnOverflow;

	const char* m_pDebugName;
};
