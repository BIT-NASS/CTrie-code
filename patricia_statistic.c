#include <assert.h>
#include "patricia_statistic.h"


int count_set_bit(uint32_t i)
{
	i = i - ((i >> 1) & 0x55555555);
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
	return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

static void component_patricia_traversal(struct component_patricia_node *node,
										 struct statistic_info *info, size_t depth)
{
	int bucket_size = 0;
	int i = 0;
	if (node == NULL){
		return;
	}
	info->mem_count += sizeof(*node);
	info->node_count++;
	if (node->token != NULL) {
		info->mem_count += strlen(node->token) + 1;
	}
	if (node->port > 0) { // assume that is leaf
		assert(node->token != NULL);
		info->leaf_count++;
		info->total_depth += depth;
		if (depth > info->max_depth) {
			info->max_depth = depth;
		}

	}
	if (node->bucket != NULL) {
		struct component_patricia_node * tmpnode = NULL;
		if(depth == 0) bucket_size = BUCKET_SIZE_FIRST;
		else bucket_size = BUCKET_SIZE_SECOND;

		info->mem_count += sizeof(struct component_patricia_node *) * bucket_size; //加上哈希表的大小

		for (i = 0; i != bucket_size; ++i) {
			tmpnode = node->bucket[i];
			while (tmpnode != NULL){
				component_patricia_traversal(tmpnode, info, depth + 1);
				//info->mem_count += sizeof(struct component_patricia_node *); //加上冲突指针
				tmpnode = tmpnode->next;
			}
		}
	}
}

struct statistic_info component_patricia_statistic(struct component_patricia_node *root)
{
	struct statistic_info info = {0};

	component_patricia_traversal(root, &info, 0);

	info.avg_depth = info.total_depth / info.leaf_count;

	return info;
}



static void bitmap_patricia_traversal(struct bitmap_patricia_node *node,
									  struct statistic_info *info, size_t depth)
{
	int bucket_size = 0;
	int i = 0;
	int child_count = 0;

	if (node->token != NULL) {  //root节点的token为空，不计算？
		info->mem_count += sizeof(*node);
		info->node_count++;
		info->mem_count += strlen(node->token) + 1;
	}
	else{
		info->mem_count += sizeof(node->bitmap);
	}

	if (!Is_internal_node(node)) {
		info->leaf_count++;
		info->total_depth += depth;
		if (depth > info->max_depth) {
			info->max_depth = depth;
		}
	}

	for (i = 0; i != BMP_LENGTH; ++i) {
		child_count += count_set_bit(node->bitmap[i]);
	}

	info->mem_count += sizeof(struct bitmap_patricia_node *) * child_count; //存储孩子节点的指针

	for (i = 0; i != child_count; ++i) {
		bitmap_patricia_traversal(node->pchild[i], info, depth+1);
	}
}

struct statistic_info bitmap_patricia_statistic(struct bitmap_patricia_node *root, size_t depth )
{
	struct statistic_info info = {0};

	bitmap_patricia_traversal(root, &info, depth);

	info.avg_depth = info.total_depth / info.leaf_count;

	return info;
}



struct statistic_info component_byte_patricia_statistic(struct component_byte_patricia_node **root)
{
	struct statistic_info info = {0};
	struct statistic_info tmp  = {0};
	struct component_byte_patricia_node *node = NULL;
	int i = 0;

	info.mem_count += sizeof(*root);
	info.mem_count += sizeof(struct component_byte_patricia_node *) * BUCKET_LEN;

	for (i = 0; i != BUCKET_LEN; ++i) {
		node = root[i];
		while (node != NULL) {
			info.mem_count += sizeof(*node);
			info.node_count += 1;
			if (node->domain_name != NULL) {
				info.mem_count += strlen(node->domain_name) + 1;
			}
			if (node->next_hop > 0 || node->input_port > 0 || node->pdata){
				info.leaf_count += 1;
				if (info.max_depth < 1) 
					info.max_depth = 1;
				info.total_depth += 1;
			}
			if (node->pt_root != NULL) {
				tmp = bitmap_patricia_statistic(node->pt_root, 1);
				info.mem_count += tmp.mem_count;
				info.leaf_count += tmp.leaf_count;
				info.node_count += tmp.node_count;
				info.total_depth += tmp.total_depth;
				if (tmp.max_depth > info.max_depth) info.max_depth = tmp.max_depth;
				memset(&tmp, 0, sizeof(tmp));
			}
			node = node->next_node;
		}
	}

	info.avg_depth = info.total_depth / info.leaf_count;

	return info;
}

