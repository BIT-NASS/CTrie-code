#ifndef PATRICIA_STATISTIC_H
#define PATRICIA_STATISTIC_H


#include <stdint.h>

#include "bitmap_patricia.h"
#include "component_byte_patricia.h"
#include "component_patricia.h"



struct statistic_info {
	double total_depth;
	uint32_t leaf_count;
	uint64_t mem_count;
	uint32_t node_count;
	uint32_t max_depth;

	double avg_depth;
};


struct statistic_info bitmap_patricia_statistic(struct bitmap_patricia_node *root, size_t depth);

struct statistic_info component_byte_patricia_statistic(struct component_byte_patricia_node **root);

struct statistic_info component_patricia_statistic(struct component_patricia_node *root);

int count_set_bit(uint32_t i);

#endif