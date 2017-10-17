
#include "siphash.h"

/* SipHash-2-4 */
uint64_t
	siphash_2_4(const unsigned char *in, size_t inlen, const unsigned char *k)
{
	/* "somepseudorandomlygeneratedbytes" */
	uint64_t v0 = 0x736f6d6570736575ULL;
	uint64_t v1 = 0x646f72616e646f6dULL;
	uint64_t v2 = 0x6c7967656e657261ULL;
	uint64_t v3 = 0x7465646279746573ULL;
	uint64_t b;
	uint64_t k0 = U8TO64_LE(k);
	uint64_t k1 = U8TO64_LE(k + 8);
	uint64_t m;
	const uint8_t *end = in + inlen - (inlen % sizeof(uint64_t));
	const int left = inlen & 7;
	b = ((uint64_t)inlen) << 56;
	v3 ^= k1;
	v2 ^= k0;
	v1 ^= k1;
	v0 ^= k0;

	for (; in != end; in += 8) {
		m = U8TO64_LE(in);
#ifdef DEBUG
		printf("(%3d) v0 %08x %08x\n", (int)inlen, (uint32_t)(v0 >> 32), (uint32_t)v0);
		printf("(%3d) v1 %08x %08x\n", (int)inlen, (uint32_t)(v1 >> 32), (uint32_t)v1);
		printf("(%3d) v2 %08x %08x\n", (int)inlen, (uint32_t)(v2 >> 32), (uint32_t)v2);
		printf("(%3d) v3 %08x %08x\n", (int)inlen, (uint32_t)(v3 >> 32), (uint32_t)v3);
		printf("(%3d) compress %08x %08x\n", (int)inlen, (uint32_t)(m >> 32), (uint32_t)m);
#endif
		v3 ^= m;
		SIPROUND;
		SIPROUND;
		v0 ^= m;
	}

	switch (left) {
	case 7: b |= ((uint64_t)in[6])  << 48;

	case 6: b |= ((uint64_t)in[5])  << 40;

	case 5: b |= ((uint64_t)in[4])  << 32;

	case 4: b |= ((uint64_t)in[3])  << 24;

	case 3: b |= ((uint64_t)in[2])  << 16;

	case 2: b |= ((uint64_t)in[1])  <<  8;

	case 1: b |= ((uint64_t)in[0]); break;

	case 0: break;
	}

#ifdef DEBUG
	printf("(%3d) v0 %08x %08x\n", (int)inlen, (uint32_t)(v0 >> 32), (uint32_t)v0);
	printf("(%3d) v1 %08x %08x\n", (int)inlen, (uint32_t)(v1 >> 32), (uint32_t)v1);
	printf("(%3d) v2 %08x %08x\n", (int)inlen, (uint32_t)(v2 >> 32), (uint32_t)v2);
	printf("(%3d) v3 %08x %08x\n", (int)inlen, (uint32_t)(v3 >> 32), (uint32_t)v3);
	printf("(%3d) padding   %08x %08x\n", (int)inlen, (uint32_t)(b >> 32), (uint32_t)b);
#endif
	v3 ^= b;
	SIPROUND;
	SIPROUND;
	v0 ^= b;
#ifdef DEBUG
	printf("(%3d) v0 %08x %08x\n", (int)inlen, (uint32_t)(v0 >> 32), (uint32_t)v0);
	printf("(%3d) v1 %08x %08x\n", (int)inlen, (uint32_t)(v1 >> 32), (uint32_t)v1);
	printf("(%3d) v2 %08x %08x\n", (int)inlen, (uint32_t)(v2 >> 32), (uint32_t)v2);
	printf("(%3d) v3 %08x %08x\n", (int)inlen, (uint32_t)(v3 >> 32), (uint32_t)v3);
#endif
	v2 ^= 0xff;
	SIPROUND;
	SIPROUND;
	SIPROUND;
	SIPROUND;
	b = v0 ^ v1 ^ v2  ^ v3;
	return b;
}