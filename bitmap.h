#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define BMP_LENGTH	8
#define SHIFT 5
#define MASK 0x1F //16进制下的31

uint32_t bit_mask[33];

void init_bit_mask();
void bitmap_set(uint32_t * bitmap, uint32_t i);
uint32_t bitmap_get(uint32_t * bitmap, uint32_t i);
void bitmap_clear(uint32_t * bitmap, uint32_t i);
int Count1ofNumber(uint32_t n);
int Count1ofBitmap(uint32_t *bm, uint32_t n);

#endif