
#include <samoware/sdk/bitbuf.h>

namespace bitbuf {
	uint32_t littleBits[32];
	uint32_t bitWriteMasks[32][33];
	uint32_t extraMasks[33];
}

inline int GetBitForBitnum(int bitNum) {
	static int bitsForBitnum[] = {
		(1 << 0),
		(1 << 1),
		(1 << 2),
		(1 << 3),
		(1 << 4),
		(1 << 5),
		(1 << 6),
		(1 << 7),
		(1 << 8),
		(1 << 9),
		(1 << 10),
		(1 << 11),
		(1 << 12),
		(1 << 13),
		(1 << 14),
		(1 << 15),
		(1 << 16),
		(1 << 17),
		(1 << 18),
		(1 << 19),
		(1 << 20),
		(1 << 21),
		(1 << 22),
		(1 << 23),
		(1 << 24),
		(1 << 25),
		(1 << 26),
		(1 << 27),
		(1 << 28),
		(1 << 29),
		(1 << 30),
		(1 << 31)
	};

	return bitsForBitnum[(bitNum) & (32 - 1)];
}

class InitBitMask {
public:
	InitBitMask() {
		for (uint32_t startbit = 0; startbit < 32; startbit++) {
			for (uint32_t nBitsLeft = 0; nBitsLeft < 33; nBitsLeft++) {
				uint32_t endbit = startbit + nBitsLeft;
				bitbuf::bitWriteMasks[startbit][nBitsLeft] = GetBitForBitnum(startbit) - 1;

				if (endbit < 32) {
					bitbuf::bitWriteMasks[startbit][nBitsLeft] |= ~(GetBitForBitnum(endbit) - 1);
				}
			}
		}

		for (uint32_t maskBit = 0; maskBit < 32; maskBit++) {
			bitbuf::extraMasks[maskBit] = GetBitForBitnum(maskBit) - 1;
		}

		bitbuf::extraMasks[32] = ~0ul;

		for (uint32_t littleBit = 0; littleBit < 32; littleBit++) {
			(&bitbuf::littleBits[littleBit])[0] = 1u << littleBit;
		}
	}
};

static InitBitMask bitMasks;
