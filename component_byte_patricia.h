#ifndef _HASH_AND_INSERT_PT_H
#define _HASH_AND_INSERT_PT_H

#define BUCKET_LEN	64000	// for hash, 1K or 10K or 100K ...

// for hash
struct component_byte_patricia_node {
	int next_hop;								// store the FIB entry
	int input_port;								// store the PIT entry
	char* pdata;						//store the CS entry
	char *domain_name;
	struct bitmap_patricia_node *pt_root;
	struct component_byte_patricia_node *next_node;
};

unsigned int SDBMHash(char* str, unsigned int len);

void component_byte_patricia_insert(struct component_byte_patricia_node **bucket_list, char *str_input, int nInputPort);

int component_byte_patricia_lookup(struct component_byte_patricia_node **bucket_list, char *str_input);

// operate three type nodes
void component_byte_patricia_insert_fib(struct component_byte_patricia_node **root_bucket, char *str_input, int next_hop);

void component_byte_patricia_insert_pit(struct component_byte_patricia_node **root_bucket, char *str_input, int nInputPort);

void component_byte_patricia_insert_cs(struct component_byte_patricia_node **root_bucket, char *str_input, char* data);

// delete operations
void del_component_byte_patricia_cs(struct component_byte_patricia_node **root_bucket, char *str_input);

void del_component_byte_patricia_pit(struct component_byte_patricia_node **root_bucket, char *str_input);

// match prefix

int cb_patricia_longest_match_fib(struct component_byte_patricia_node **root_bucket, char *str_input);

struct CSdataList* cb_patricia_deepest_match_cs(struct component_byte_patricia_node **root_bucket, char *str_input);

// deal with incoming Interest packets
struct InterestResult* deal_Interest_in_cb_patricia(struct component_byte_patricia_node **root_bucket, char *str_input, int next_hop);

struct PortList* deal_Data_in_cb_patricia(struct component_byte_patricia_node **root_bucket, char *str_input, char* data);

#endif