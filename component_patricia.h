#ifndef COMPONENT_PATRICIA_H
#define COMPONENT_PATRICIA_H


#include <stdlib.h>
#include <string.h>

#include <stdint.h>

// #define BUCKET_SIZE 16
#define BUCKET_SIZE_FIRST  64000
#define BUCKET_SIZE_SECOND 16
#define BUCKET_SIZE_THIRD  4
#define BUCKET_SIZE_FOURTH 2


// struct component_patricia_node;

struct component_patricia_node {
	int port;
	char *token;
	struct component_patricia_node *next; //ͬһbucket�е���һ���ڵ�
	struct component_patricia_node **bucket; //bucket����һ�������ݵ�ͷ��㣬ͷ����next��Ϊ�գ�����Ϊ�ա�
};

uint64_t calcmem(struct component_patricia_node *root);

int calcdepth(struct component_patricia_node *root, char *input, int inputlen);

int component_patricia_insert(struct component_patricia_node *root, char *input, int strlen);

int component_patricia_lookup(struct component_patricia_node *root, char *input, int strlen);

int calc_collapse(struct component_patricia_node *hashtb);

#endif