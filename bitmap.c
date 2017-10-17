#include "bitmap.h"

void init_bit_mask(){
	int i = 0;
	bit_mask[0] = 0;
	bit_mask[1] = 1;
	for (i = 2; i != 32; ++i) {
		bit_mask[i] = (bit_mask[i - 1] << 1) + 1;
	}
}

void bitmap_set(uint32_t * bitmap, uint32_t i){
	bitmap[i >> SHIFT] |= (1 << (i & MASK));
}

uint32_t bitmap_get(uint32_t * bitmap, uint32_t i){
	return bitmap[i >> SHIFT] & (1 << (i & MASK));
}

void bitmap_clear(uint32_t * bitmap, uint32_t i){
	bitmap[i >> SHIFT] &= ~(1 << (i & MASK));
}

int Count1ofNumber(uint32_t n){
	n -= ((n >> 1) & 0x55555555);
	n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
	n = ((n + (n >> 4)) & 0x0f0f0f0f);
	return (n * 0x01010101) >> 24;
}

//计算n位之前的bitmap种有多少个1
int Count1ofBitmap(uint32_t *bm, uint32_t n){
	int counter = 0;
	uint32_t bmap_byte = 0, bmap_bit = 0, i = 0;
	uint32_t tmp_bmap = 0;

	bmap_byte = n >> SHIFT;
	bmap_bit = n & MASK;
	for (i = 0; i < bmap_byte; i++){
		counter += Count1ofNumber(bm[i]);
	}
	if (i != BMP_LENGTH){
		//tmp_bmap = bm[bmap_byte] << (32 - bmap_bit); //当bmap_bit 等于0时，tmp_bmap移位过大为1
		tmp_bmap = bm[bmap_byte] & bit_mask[bmap_bit];
		counter += Count1ofNumber(tmp_bmap);
	}
	return counter;

}