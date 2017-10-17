#ifndef SIPHASH_H
#define SIPHASH_H

#include <stdlib.h>
#include <stdint.h>

#define ROTL(x,b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

#define U32TO8_LE(p, v)         \
	(p)[0] = (uint8_t)((v)      ); (p)[1] = (uint8_t)((v) >>  8); \
	(p)[2] = (uint8_t)((v) >> 16); (p)[3] = (uint8_t)((v) >> 24);

#define U64TO8_LE(p, v)         \
	U32TO8_LE((p),     (uint32_t)((v)     ));   \
	U32TO8_LE((p) + 4, (uint32_t)((v) >> 32));

#define U8TO64_LE(p) \
	(((uint64_t)((p)[0])      ) | \
	((uint64_t)((p)[1]) <<  8) | \
	((uint64_t)((p)[2]) << 16) | \
	((uint64_t)((p)[3]) << 24) | \
	((uint64_t)((p)[4]) << 32) | \
	((uint64_t)((p)[5]) << 40) | \
	((uint64_t)((p)[6]) << 48) | \
	((uint64_t)((p)[7]) << 56))

#define SIPROUND            \
	do {              \
	v0 += v1; v1=ROTL(v1,13); v1 ^= v0; v0=ROTL(v0,32); \
	v2 += v3; v3=ROTL(v3,16); v3 ^= v2;     \
	v0 += v3; v3=ROTL(v3,21); v3 ^= v0;     \
	v2 += v1; v1=ROTL(v1,17); v1 ^= v2; v2=ROTL(v2,32); \
	} while(0)

/* SipHash-2-4 */
uint64_t
	siphash_2_4(const unsigned char *in, size_t inlen, const unsigned char *k);
#endif
