#ifndef BITMAP_PATRICIA_H
#define BITMAP_PATRICIA_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"




// this header define patricia_node structure & define the constant
#define	MAX_DEPTH	300
#define BUFSIZE		2048
#define BMP_LENGTH	8

struct face_list {
	int length;			// list length
	int *list;			// port list
};

//struct CSdata{
//	char* pData;
//};

struct bitmap_patricia_node {
	int next_hop;								// store the FIB entry
	int input_port;								// store the PIT entry
	char* pdata;						//store the CS entry
	char *token;								// the PATRICIA_NODE_STRU context
	uint32_t bitmap[BMP_LENGTH];				// bitmap 32 bytes = 256 bits
	struct bitmap_patricia_node **pchild;		// pointer to pointer array
};

struct ptnode_queue {
	int index;
	struct bitmap_patricia_node *patricia_node;
	struct ptnode_queue *next_node;
};

struct CSdataList{
	char * data;
	struct CSdataList* next;
};

struct InterestResult{
	int hop;
	struct CSdataList * datalist;
};

struct PortList{
	int port;
	struct PortList* next;
};

struct DataPkgResult{
	struct PortList* portList;
	struct CSdataList * data;
};

void bitmap_patricia_initial(struct bitmap_patricia_node *root);

void bitmap_patricia_insert(struct bitmap_patricia_node *pParent, char *strInput, size_t nStartPos, int nInputPort);

int bitmap_patricia_lookup(struct bitmap_patricia_node *pParent, char *strInput, size_t nStartPos);

// three insert operation

void bitmap_patricia_insert_fib(struct bitmap_patricia_node *pParent, char *strInput, size_t nStartPos, int nextHop);

void bitmap_patricia_insert_pit(struct bitmap_patricia_node *pParent, char *strInput, size_t nStartPos, int nInputPort, int flag);

void bitmap_patricia_insert_CS(struct bitmap_patricia_node *pParent, char *strInput, size_t nStartPos, char * data, int flag);

void free_bitmap_patricia_node(struct bitmap_patricia_node * node);

void free_CSdataList(struct CSdataList* node);

int Is_internal_node(struct bitmap_patricia_node * node);

// the match operation
struct bitmap_patricia_node * find_exact_match_Node(struct bitmap_patricia_node **pParent, char *strInput, uint32_t* index);

struct bitmap_patricia_node * find_one_match_PitNode(struct bitmap_patricia_node **pParent, char *strInput, uint32_t* index);

int bitmap_patricia_longest_match_fib(struct bitmap_patricia_node *pParent, char *strInput);

int bitmap_patricia_exact_match_pit(struct bitmap_patricia_node *pParent, char *strInput);

struct CSdataList* bitmap_patricia_deepest_match_cs(struct bitmap_patricia_node *pParent, char *strInput);

void getSubTreeCSDataSet(struct bitmap_patricia_node *pParent, struct CSdataList* dataset);

void consume_bitmap_patricia_pitNode(struct bitmap_patricia_node *pParent, char* strInput);

void consume_bitmap_patricia_PitWithNode(struct bitmap_patricia_node* pParent, struct bitmap_patricia_node **node, int index);

// the delete operation
void del_bitmap_patricia_node(struct bitmap_patricia_node *pParent, char* strInput);

void del_bitmap_patricia_fib(struct bitmap_patricia_node *pParent, char* strInput); //undo

void del_bitmap_patricia_pit(struct bitmap_patricia_node *pParent, char* strInput);

void del_bitmap_patricia_cs(struct bitmap_patricia_node *pParent, char* strInput);

// unified operation for three tables
// Interest到达后，查询树结构，可能返回数据或下一跳信息
struct InterestResult* deal_Interest_in_bitmap_patricia(struct bitmap_patricia_node *pParent, char* strInput, int nInputPort, int hop);

// Data 到达后，会返回数据以及Interest进入的接口
struct PortList* deal_Data_in_bitmap_patricia(struct bitmap_patricia_node *pParent, char* strInput, char* data, int port);

#endif