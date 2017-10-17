#include <assert.h>

#include "bitmap_patricia.h"
#include "component_byte_patricia.h"


unsigned int SDBMHash(char* str, unsigned int len)
{
	unsigned int hash = 0;
	unsigned int i    = 0;
	for(i = 0; i < len; str++, i++) {
		hash = (*str) + (hash << 6) + (hash << 16) - hash;
	}
	return hash%BUCKET_LEN;
}

void get_domain_url(char *str_input, char* domain_name, int* len_domain, char* url_name, int * len_url){
	int input_len = strlen(str_input);
	int i = 0, j = 0, k = 0;

	for (i = 0; i < input_len; i++){
		if (str_input[i] == '/'){
			break;
		}
		domain_name[j++] = str_input[i];
	}
	domain_name[j] = '\0';
	*len_domain = j;

	while (++j < input_len){
		url_name[k++] = str_input[j];
	}
	url_name[k] = '\0';
	*len_url = k;
	return;
}

void component_byte_patricia_insert(struct component_byte_patricia_node ** root_bucket, char *str_input, int nInputPort)
{
	// defind parameter
	char str_domain_name[BUFSIZE];
	char str_url[BUFSIZE];

	int domain_length=0;
	int url_length=0;

	unsigned int bucket_index=0;
	struct component_byte_patricia_node *new_node = NULL, *p_node = NULL;
	struct bitmap_patricia_node *pt_node=NULL;

	get_domain_url(str_input, str_domain_name, &domain_length, str_url, &url_length);

	// insert list
	bucket_index = SDBMHash(str_domain_name, domain_length);

	if (root_bucket[bucket_index] == NULL){
		// 1) not exist before
		new_node = (struct component_byte_patricia_node*) calloc(1, sizeof(struct component_byte_patricia_node));
		new_node->domain_name = (char *)calloc(domain_length + 1, sizeof(char));
		strcpy(new_node->domain_name, str_domain_name);
		new_node->next_node = NULL;

		if (url_length == 0){
			new_node->next_hop = nInputPort;
		}
		else{
			new_node->next_hop = -1;
			new_node->pt_root = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
			bitmap_patricia_initial(new_node->pt_root);
			// insert to patricia trie
			bitmap_patricia_insert_fib(new_node->pt_root, str_url, 0, nInputPort);
		}
		root_bucket[bucket_index] = new_node;
	}
	else {
		// 2) maybe comflict need compare

		for (p_node = root_bucket[bucket_index]; p_node != NULL;){
			if (strcmp(str_domain_name, p_node->domain_name) == 0) {
				// 2.1) no comflict
				if (url_length == 0){
					p_node->next_hop = nInputPort;
				}
				else{
					// insert to patricia trie
					if (p_node->pt_root == NULL){
						p_node->pt_root = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
						bitmap_patricia_initial(p_node->pt_root);
					}
					bitmap_patricia_insert_fib(p_node->pt_root, str_url, 0, nInputPort);
				}
				break;
			}
			else if (p_node->next_node == NULL){
				new_node = (struct component_byte_patricia_node *) calloc(1, sizeof(struct component_byte_patricia_node));
				new_node->domain_name = (char *)calloc(domain_length + 1, sizeof(char));
				strcpy(new_node->domain_name, str_domain_name);
				new_node->next_node = NULL;
				if (url_length == 0){
					new_node->next_hop = nInputPort;
				}
				else{
					new_node->next_hop = -1;
					new_node->pt_root = (struct bitmap_patricia_node *) calloc(1, sizeof(struct bitmap_patricia_node));
					bitmap_patricia_initial(new_node->pt_root);
					// insert to patricia trie
					bitmap_patricia_insert_fib(new_node->pt_root, str_url, 0, nInputPort);
				}
				p_node->next_node = new_node;
				break;
			}
			else{
				p_node = p_node->next_node;
			}
		}
	} //if(root_bucket[bucket_index] == NULL)
}

int component_byte_patricia_lookup(struct component_byte_patricia_node **bucket_list, char *str_input)
{
	// defind parameter
	char str_domain_name[BUFSIZE];
	char *str_url;

	int i,j;
	int start_position=0;
	int input_length=0;
	int domain_length=0;
	int url_length=0;

	// -- about bucket_list
	unsigned int bucket_index=0;
	struct component_byte_patricia_node *p_list = NULL;
	// -- aout patricia trie
	struct bitmap_patricia_node *pt_node=NULL;
	// -- return value
	int nSearchPort=-1;
	// end defind parameter //

	// parameter bitmap_patricia_initial
	input_length = strlen(str_input);
	// end bitmap_patricia_initial //


	// get domain name
	for (i = 0, j = 0; i < input_length; i++, j++){
		if (str_input[i] == '/'){
			start_position = i+1;
			break;
		}
		str_domain_name[j] = str_input[i];
	}
	str_domain_name[j] = '\0';
	domain_length = j;
	if (i == input_length){
		start_position = i;
	}

	str_url = &str_input[start_position];
	url_length = strlen(str_url);

	// search list
	bucket_index = SDBMHash(str_domain_name, domain_length);

	if (bucket_list[bucket_index] == NULL ){
		// 1) no find
		return -1;
	} else {
		// 2) maybe comflict need compare
		for (p_list = bucket_list[bucket_index]; p_list != NULL; ){
			if ( strcmp(str_domain_name, p_list->domain_name) == 0) {
				// 2.1) no comflict
				// insert to patricia trie
				if (url_length == 0)
					nSearchPort = p_list->next_hop;
				else
					nSearchPort = bitmap_patricia_lookup(p_list->pt_root, str_url, 0);
				assert(nSearchPort > 0);
				break;
			} else if (p_list->next_node == NULL) {
				// 2.2) comflict
				nSearchPort = -1;
				break;
			} else {
				p_list = p_list->next_node;
			}
		} // end for

		return nSearchPort;

	} // if (bucket_list[bucket_index] == NULL)
}


void component_byte_patricia_insert_fib(struct component_byte_patricia_node **root_bucket, char *str_input, int next_hop){
	component_byte_patricia_insert(root_bucket, str_input, next_hop);
}

void component_byte_patricia_insert_pit(struct component_byte_patricia_node **root_bucket, char *str_input, int nInputPort){
	char str_domain_name[BUFSIZE];
	char str_url[BUFSIZE];

	int domain_length = 0;
	int url_length = 0;
	int flag = 0;

	unsigned int bucket_index = 0;
	struct component_byte_patricia_node *new_node = NULL, *p_node = NULL;
	struct bitmap_patricia_node *pt_node = NULL;

	get_domain_url(str_input, str_domain_name, &domain_length, str_url, &url_length);

	bucket_index = SDBMHash(str_domain_name, domain_length);
	if (root_bucket[bucket_index] == NULL){
		// 1) not exist in FIB
		return; //插入到 default
	}
	else {
		for (p_node = root_bucket[bucket_index]; p_node != NULL;){
			if (strcmp(str_domain_name, p_node->domain_name) == 0) {
				// find match
				if (url_length == 0){
					p_node->input_port = nInputPort; //加入Interest Interface
				}
				else{
					// insert to patricia trie
					if (p_node->next_hop > 0){
						flag = 1;
						if (p_node->pt_root == NULL){ // 匹配第一个component的leaf
							p_node->pt_root = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
							bitmap_patricia_initial(p_node->pt_root);
						}
					}
					else if (p_node->pt_root == NULL){
						return; // 未匹配到FIB leaf
					}
					bitmap_patricia_insert_pit(p_node->pt_root, str_url, 0, nInputPort, flag);
				}
				break;
			}
			else if (p_node->next_node == NULL){
				return; //未匹配到FIB节点
			}
			else{
				p_node = p_node->next_node;
			}
		}
	} //if(root_bucket[bucket_index] == NULL)
}

void component_byte_patricia_insert_cs(struct component_byte_patricia_node **root_bucket, char *str_input, char * data){
	char str_domain_name[BUFSIZE];
	char str_url[BUFSIZE];

	int domain_length = 0;
	int url_length = 0;
	int flag = 0;

	unsigned int bucket_index = 0;
	struct component_byte_patricia_node *new_node = NULL, *p_node = NULL;
	struct bitmap_patricia_node *pt_node = NULL;

	get_domain_url(str_input, str_domain_name, &domain_length, str_url, &url_length);

	bucket_index = SDBMHash(str_domain_name, domain_length);
	if (root_bucket[bucket_index] == NULL){
		// 1) not exist in FIB
		return; //插入到 default
	}
	else {
		for (p_node = root_bucket[bucket_index]; p_node != NULL;){
			if (strcmp(str_domain_name, p_node->domain_name) == 0) {
				// find match
				if (p_node->input_port > 0){
					p_node->input_port = -1;
				}
				if (url_length == 0){
					p_node->pdata = data; //加入Interest Interface
				}
				else{
					// insert to patricia trie
					if (p_node->next_hop > 0){
						flag = 1;
						if (p_node->pt_root == NULL){ // 匹配第一个component的leaf
							p_node->pt_root = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
							bitmap_patricia_initial(p_node->pt_root);
						}
					}
					else if (p_node->pt_root == NULL){
						return; // 未匹配到FIB leaf
					}
					bitmap_patricia_insert_CS(p_node->pt_root, str_url, 0, data, flag);
				}
				break;
			}
			else if (p_node->next_node == NULL){
				return; //未匹配到FIB节点
			}
			else{
				p_node = p_node->next_node;
			}
		}
	} //if(root_bucket[bucket_index] == NULL)
}

//返回匹配到的component节点
struct component_byte_patricia_node* find_match_component_node(struct component_byte_patricia_node **root_bucket, char *domain_name, int domain_length){
	unsigned int bucket_index = 0;
	struct component_byte_patricia_node *new_node = NULL, *p_node = NULL;
	struct bitmap_patricia_node *pt_node = NULL;

	if (root_bucket == NULL || domain_name == NULL || domain_length == 0){
		return NULL;
	}

	bucket_index = SDBMHash(domain_name, domain_length);
	if (root_bucket[bucket_index] == NULL){
		return NULL;
	}
	else {
		for (p_node = root_bucket[bucket_index]; p_node != NULL;){
			if (strcmp(domain_name, p_node->domain_name) == 0) {
				return p_node;
			}
			else if (p_node->next_node != NULL){
				p_node = p_node->next_node;
			}
		}
	} //if(root_bucket[bucket_index] == NULL)
	return NULL;
}

void del_component_byte_patricia_cs(struct component_byte_patricia_node **root_bucket, char *str_input){
	struct component_byte_patricia_node* cp_node = NULL;
	char str_domain_name[BUFSIZE];
	char str_url[BUFSIZE];

	int domain_length = 0;
	int url_length = 0;

	get_domain_url(str_input, str_domain_name, &domain_length, str_url, &url_length);

	cp_node = find_match_component_node(root_bucket, str_domain_name, domain_length);
	if (cp_node == NULL){
		return;
	}
	if (url_length == 0){
		if (cp_node->pdata){
			free(cp_node->pdata);
			cp_node->pdata = NULL;
		}
	}
	else if(cp_node->pt_root != NULL){
		del_bitmap_patricia_cs(cp_node->pt_root, str_url);
		if (cp_node->pt_root->pchild == NULL){ //无孩子节点
			free_bitmap_patricia_node(cp_node->pt_root);
			cp_node->pt_root = NULL;
		}
	}
	return;
}

void del_component_byte_patricia_pit(struct component_byte_patricia_node **root_bucket, char *str_input){
	struct component_byte_patricia_node* cp_node = NULL;
	char str_domain_name[BUFSIZE];
	char str_url[BUFSIZE];

	int domain_length = 0;
	int url_length = 0;

	get_domain_url(str_input, str_domain_name, &domain_length, str_url, &url_length);

	cp_node = find_match_component_node(root_bucket, str_domain_name, domain_length);
	if (cp_node == NULL){
		return;
	}
	if (url_length == 0){
		cp_node->input_port = -1;
	}
	else if (cp_node->pt_root != NULL){
		del_bitmap_patricia_pit(cp_node->pt_root, str_url);
		if (cp_node->pt_root->pchild == NULL){ //无孩子节点
			free_bitmap_patricia_node(cp_node->pt_root);
			cp_node->pt_root = NULL;
		}
	}
	return;
}

// FIB 的最长前缀匹配
int cb_patricia_longest_match_fib(struct component_byte_patricia_node **root_bucket, char *str_input){
	struct component_byte_patricia_node* cp_node = NULL;
	char str_domain_name[BUFSIZE];
	char str_url[BUFSIZE];
	int hop = -1;

	int domain_length = 0;
	int url_length = 0;

	get_domain_url(str_input, str_domain_name, &domain_length, str_url, &url_length);

	cp_node = find_match_component_node(root_bucket, str_domain_name, domain_length);
	if (cp_node == NULL){
		return hop;
	}
	if (url_length == 0){
		if (cp_node->next_hop > 0)
			hop = cp_node->next_hop;
	}
	else if (cp_node->pt_root != NULL){
		hop = bitmap_patricia_longest_match_fib(cp_node->pt_root, str_url);
		if (hop == -1 && cp_node->next_hop > 0){
			hop = cp_node->next_hop;
		}
	}
	return hop;
}

struct CSdataList* cb_patricia_deepest_match_cs(struct component_byte_patricia_node **root_bucket, char *str_input){
	struct component_byte_patricia_node* cp_node = NULL;
	char str_domain_name[BUFSIZE];
	char str_url[BUFSIZE];
	struct CSdataList * dataset = NULL, *tmpset = NULL;

	int domain_length = 0;
	int url_length = 0;
	int datalen = 0;

	get_domain_url(str_input, str_domain_name, &domain_length, str_url, &url_length);

	cp_node = find_match_component_node(root_bucket, str_domain_name, domain_length);
	if (cp_node == NULL){
		return dataset;
	}
	if (url_length == 0){
		if (cp_node->pdata){
			datalen = strlen(cp_node->pdata);
			dataset = (struct CSdataList*) calloc(1, sizeof(struct CSdataList));
			dataset->data = (char *)malloc(sizeof(char)*(datalen + 1));
			strcpy(dataset->data, cp_node->pdata);
			dataset->next = NULL;
		}
		if (cp_node->pt_root){
			tmpset = (struct CSdataList*) calloc(1, sizeof(struct CSdataList));
			getSubTreeCSDataSet(cp_node->pt_root, tmpset);
			if (tmpset->next == NULL){
				free(tmpset);
				tmpset = NULL;
			}
			else{
				if (!dataset){
					dataset = tmpset->next;
				}
				else{
					dataset->next = tmpset->next;
				}
				free(tmpset);
				tmpset = NULL;
			}
		}
	}
	else if (cp_node->pt_root != NULL){
		dataset = bitmap_patricia_deepest_match_cs(cp_node->pt_root, str_url);
		if (dataset&& dataset->next){
			tmpset = dataset; //去掉头结点
			dataset = dataset->next;
			free(tmpset);
			tmpset = NULL;
		}
	}
	return dataset;
}

struct InterestResult* deal_Interest_in_cb_patricia(struct component_byte_patricia_node **root_bucket, char *str_input, int nInputPort){
	struct component_byte_patricia_node* cp_node = NULL;
	struct InterestResult* res = NULL;
	struct CSdataList* datalist = NULL, *tmpset = NULL;
	char str_domain_name[BUFSIZE];
	char str_url[BUFSIZE];
	int hop = -1;

	int domain_length = 0;
	int url_length = 0;
	int datalen = 0;

	get_domain_url(str_input, str_domain_name, &domain_length, str_url, &url_length);

	cp_node = find_match_component_node(root_bucket, str_domain_name, domain_length);
	if (cp_node == NULL){
		return res;
	}

	if (cp_node->next_hop > 0){
		hop = cp_node->next_hop;
	}
	if (url_length == 0){
		if (hop != -1){
			cp_node->input_port = nInputPort; //插入PIT信息
		}
		if (cp_node->pdata){
			datalen = strlen(cp_node->pdata);
			datalist = (struct CSdataList*) calloc(1, sizeof(struct CSdataList));
			datalist->data = (char *)malloc(sizeof(char)*(datalen + 1));
			strcpy(datalist->data, cp_node->pdata);
			datalist->next = NULL;
		}
		if (cp_node->pt_root){
			tmpset = bitmap_patricia_deepest_match_cs(cp_node->pt_root, str_url);
			if (tmpset && tmpset->next){
				if (!datalist)
					datalist = tmpset->next;
				else
					datalist->next = tmpset->next;
				free(tmpset);
				tmpset = NULL;
			}
		}
		if (hop != -1 || datalist){
			res = (struct InterestResult*)calloc(1, sizeof(struct InterestResult));
			res->hop = hop;
			res->datalist = datalist; //不含头结点
		}
	}
	else{
		if (hop != -1 && cp_node->pt_root == NULL){
			cp_node->pt_root = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
			bitmap_patricia_initial(cp_node->pt_root);
		}
		res = deal_Interest_in_bitmap_patricia(cp_node->pt_root, str_url, nInputPort, hop);
	}
	return res;
}

struct PortList* deal_Data_in_cb_patricia(struct component_byte_patricia_node **root_bucket, char *str_input, char* data){
	char str_domain_name[BUFSIZE];
	char str_url[BUFSIZE];

	int domain_length = 0;
	int url_length = 0;
	int port = -1;

	struct component_byte_patricia_node *cp_node = NULL;
	struct PortList* res = NULL;

	get_domain_url(str_input, str_domain_name, &domain_length, str_url, &url_length);

	cp_node = find_match_component_node(root_bucket, str_domain_name, domain_length);
	if (cp_node == NULL){ //没有匹配节点 走default path
		return res;
	}

	// find match
	if (cp_node->input_port > 0){
		port = cp_node->input_port;
		cp_node->input_port = -1; // 删除PIT 信息
	}
	if (url_length == 0){
		if (port != -1){
			res = (struct PortList*)calloc(1, sizeof(struct PortList));
			res->port = port;
			res->next = NULL; 
			cp_node->pdata = data;
		}
	}
	else{
		if (port != -1 && cp_node->pt_root == NULL){
			cp_node->pt_root = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
			bitmap_patricia_initial(cp_node->pt_root);
		}
		res = deal_Data_in_bitmap_patricia(cp_node->pt_root, str_url, data, port);
	}
	return res;
}