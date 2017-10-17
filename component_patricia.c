#include <stdlib.h>
#include <assert.h>


#include "component_patricia.h"
#include "siphash.h"

static int bucketport = 0;

int component_patricia_insert(struct component_patricia_node *root,
							  char *input, int inputlen)
{

	const unsigned char seed[16] = {0xf2, 0xdd, 0xb7, 0xcb, 0x69, 0xf0, 0xa3, 0x0c,
		0x75, 0xc5, 0xde, 0x9c, 0x9b, 0xde, 0x56, 0x24};

	uint64_t hashval;
	struct component_patricia_node *node = root, *new_node = NULL, *pre_node = NULL;
	char *token;
	int tokenlen;
	int depth1 = 1;
	int bucket_size = 0;
	
	token = strtok((char *)input, "/");

	while (1) {
		if (token != NULL) {
			tokenlen = strlen(token);

			if (depth1) {
				hashval = siphash_2_4((unsigned char *)token, tokenlen, seed) % BUCKET_SIZE_FIRST;
				depth1 = 0;
				//bucket_size = BUCKET_SIZE_SECOND; //ÎªÊ²Ã´ÊÇSecond?
				bucket_size = BUCKET_SIZE_FIRST;
			}
			else {
				hashval = siphash_2_4((unsigned char *)token, tokenlen, seed) % BUCKET_SIZE_SECOND;
				bucket_size = BUCKET_SIZE_SECOND;
			}

			if (node->bucket == NULL) {
				node->bucket = (struct component_patricia_node **)calloc( bucket_size, sizeof(struct component_patricia_node*));
			}

			if (node->bucket[hashval] == NULL) {
				new_node = (struct component_patricia_node *)calloc(1, sizeof(struct component_patricia_node));
				new_node->token = (char *)calloc(tokenlen + 1, sizeof(char));
				strcpy(new_node->token, token);
				node->bucket[hashval] = new_node;
				node = new_node;
			}
			else{
				node = node->bucket[hashval];
				pre_node = node;
				while (node != NULL) {
					if (strcmp(token, node->token) == 0) {
						break;
					}
					pre_node = node;
					node = node->next;
				}
				if (node == NULL){
					new_node = (struct component_patricia_node *)calloc(1, sizeof(struct component_patricia_node));
					new_node->token = (char *)calloc(tokenlen + 1, sizeof(char));
					strcpy(new_node->token, token);
					pre_node->next = new_node;
					node = new_node;
				}
			}
			
			
		}
		else {
			node->port = ++bucketport;
			break;
		}
		token = strtok(NULL, "/");
	}
	return 0;
}



int component_patricia_lookup(struct component_patricia_node *component_patricia_node, 
							  char *input, int inputlen)
{
	const unsigned char seed[16] = {0xf2, 0xdd, 0xb7, 0xcb, 0x69, 0xf0, 0xa3, 0x0c,
		0x75, 0xc5, 0xde, 0x9c, 0x9b, 0xde, 0x56, 0x24};
	uint64_t hashval;
	struct component_patricia_node *node = component_patricia_node;
	char *token;
	int tokenlen;
	int depth1 = 1;

	token = strtok((char *)input, "/");

	while (1) {
		if (token != NULL) {
			tokenlen = strlen(token);

			if (depth1) {
				hashval = siphash_2_4((unsigned char *)token, tokenlen, seed) % BUCKET_SIZE_FIRST;
				depth1 = 0;
			}
			else {
				hashval = siphash_2_4((unsigned char *)token, tokenlen, seed) % BUCKET_SIZE_SECOND;
			}

			node = node->bucket[hashval];

			while (node != NULL) {
				if (strcmp(token, node->token) == 0) {
					break;
				}
				node = node->next;
			}
			if (node == NULL) {
				return -1;
			}
		}
		else {
			return node->port;
		}
		token = strtok(NULL, "/");
	}
}


