#include <stdint.h>

#include "bitmap_patricia.h"

#include "time.h"

#ifdef linux
/*将数字转换成字符串*/
static void _itoa(unsigned long val, char *buf, unsigned radix) 
{ 
    char *p;           /* pointer to traverse string */ 
    char *firstdig;       /* pointer to first digit */ 
    char temp;           /* temp char */ 
    unsigned digval;       /* value of digit */ 

    p = buf; 
    firstdig = p;         /* save pointer to first digit */ 

    do { 
    digval = (unsigned) (val % radix); 
    val /= radix;     /* get next digit */ 

    /* convert to ascii and store */ 
    if (digval > 9) 
        *p++ = (char ) (digval - 10 + 'a');   /* a letter */ 
    else 
        *p++ = (char ) (digval + '0');     /* a digit */ 
    } while (val > 0); 

    /* We now have the digit of the number in the buffer, but in reverse 
    order.   Thus we reverse them now. */ 

    *p-- = '\0';         /* terminate string; p points to last digit */ 

	//交换字符串的顺序
    do { 
    temp = *p; 
    *p = *firstdig; 
    *firstdig = temp;   /* swap *p and *firstdig */ 
    --p; 
    ++firstdig;       /* advance to next two digits */ 
    } while (firstdig < p); /* repeat until halfway */ 
}

#endif

static char *str_2_binary(char *line)
{
	char binary[9], *str_binary=NULL, ch_tmp[2];
	int line_length=0, i=0, j=0, tmp_str=0;

	line_length = strlen (line);
	str_binary = (char *) malloc ( (line_length*8+1)*sizeof(char) );
	memset(str_binary, 0, line_length*8+1);
	
	for (i=0; i < line_length; i++){
		tmp_str = line[i];
		binary[8] = 0;
		// change char to binary
		for (j=7; j >=0; j-- ){
			_itoa(tmp_str%2, ch_tmp, 10);
			binary[j] = ch_tmp[0];
			tmp_str = tmp_str/2;
		}
		printf ("binary = %s \t\t", binary);

		strcat (str_binary, binary);
		printf ("str_binary = %s\n", str_binary);
	}

	return str_binary;
}

// -- basic operation -- //
//
// node assignment
static void NodeAssignment(
struct bitmap_patricia_node *pNode,
	int next_hop,							// if = FIB LEAF pNode,assign hop >=0
	int input_port,							// if = PIT LEAF,
	char* pdata,					// store the CS entry
	char *token,							// the pNode context
	uint32_t *bitmap,						// bitmap 32 bytes = 256 bits
struct bitmap_patricia_node **ppChild)		// pointer to pointer array
{
	if (pNode == NULL)
	{
		printf ("\n#error: pNode assign error! pNode == NULL\n");
		return ;
	}
	pNode->next_hop = next_hop;
	pNode->input_port = input_port;
	pNode->pdata = pdata;
	pNode->token = token;
	if (bitmap == NULL)
		memset(pNode->bitmap, 0, BMP_LENGTH);
	else
		memcpy(pNode->bitmap, bitmap, BMP_LENGTH);
	// strcpy( pNode->bitmap, bitmap);
	pNode->pchild = ppChild;
}

// find the child pointer position to insert
static struct bitmap_patricia_node* FindPath2Insert(struct bitmap_patricia_node *ParentNode, unsigned char ch)
{
	uint32_t tmp_bmap;
	int pointer_index=0;

	tmp_bmap = bitmap_get(ParentNode->bitmap, ch);

	if (tmp_bmap){
		// 1) input ch has sub_str, return index ParentNode pointer
		pointer_index = Count1ofBitmap(ParentNode->bitmap, (unsigned char)ch);
		return (ParentNode->pchild[pointer_index]);

	} else {
		// 2) no common prefix, then return NULL
		return NULL;
	}
}

// add pChild to pParent, pParent->ppChild number +1
static void InsertChild2Parent ( struct bitmap_patricia_node *pParent, struct bitmap_patricia_node *pChild)
{
	int nChildNum, nChildPosition;
	struct bitmap_patricia_node **relloc_child;
	int i, j;

	if (pParent == NULL){
		return;
	}
	// calculate the newest pChild's ppChild number & malloc the memory
	bitmap_set(pParent->bitmap, (unsigned char)pChild->token[0]);
	nChildPosition = Count1ofBitmap(pParent->bitmap, (unsigned char)pChild->token[0]);
	nChildNum = Count1ofBitmap(pParent->bitmap, 256);
	
	relloc_child = (struct bitmap_patricia_node **) calloc(nChildNum, sizeof(struct bitmap_patricia_node *));

	for (i =0, j=0; i < nChildNum; i++)
	{

		if (i == nChildPosition)
			relloc_child[i] = pChild;					// insert ppChild pChild pointer to pParent
		else
			relloc_child[i] = pParent->pchild[j++];	// copy pParent to relloc_child
	}

	free(pParent->pchild);
	pParent->pchild = relloc_child;

}

// update ppChild to pParent, ppChild number = old ppChild number
static void UpdateChild2Parent ( struct bitmap_patricia_node *pParent, struct bitmap_patricia_node *pChild)
{
	int nChildNum, nChildPosition;
	int i;

	// calculate the newest pChild's ppChild number & position
	nChildNum = Count1ofBitmap(pParent->bitmap, 256);
	nChildPosition = Count1ofBitmap(pParent->bitmap, (unsigned char)pChild->token[0]);

	for (i =0; i < nChildNum; i++)
	{
		if (i == nChildPosition)
			pParent->pchild[i] = pChild;					// update ppChild pChild pointer to pParent
	}
}

// long prefix the token & strInput
static char * LongPrefix(char *token, char *strInput)
{
	int i=0, min_len=0;
	char *sub_str=NULL;

	if (strlen(token) < strlen(strInput))
		min_len = strlen(token);
	else
		min_len = strlen(strInput);

	sub_str = (char*)malloc(sizeof(char)* (min_len + 1));

	for ( i=0; i < min_len; i++)
	{
		if (token[i] == strInput[i])
			sub_str[i] = token[i];
		else 
			break;
	}
	sub_str[i] = 0;
	return sub_str;
}

int find_longest_prefix_len_str(char * token, int token_len, char* str, int str_len){
	int prefix_len = token_len > str_len ? str_len : token_len;
	int i = 0;
	for (i = 0; i != prefix_len; i++){
		if (token[i] != str[i]){
			prefix_len = i;
			break;
		}
	}
	return prefix_len;
}
// -- end basic operation -- //

// initial patricia root node
void bitmap_patricia_initial(struct bitmap_patricia_node *pRoot)
{
	//	pRoot = (v *)calloc(1, sizeof (struct bitmap_struct_ptnode));
	NodeAssignment (
		pRoot,			//  patricia trie PATRICIA_NODE_STRU point
		-1,				// if = FIB LEAF PATRICIA_NODE_STRU , than assign port >=0
		-1,				// if = PIT LEAF
		NULL,			// CS data
		NULL,			// the PATRICIA_NODE_STRU context
		0,				// bitmap 32 bytes = 256 bits
		NULL);			// pointer to pointer array
}

// insert item to patricia trie, entry_flag 表示FIB(0x01)、PIT(0x02)、CS(0x04)的一种
// 节点token的第一位为区分字节，为节点添加孩子时，需重新分配bitmap对应的链表数组，访问时O(1)，更新时较麻烦
void bitmap_patricia_insert(struct bitmap_patricia_node *pParent, char *strInput, size_t nStartPos, int nInputPort)
{
	// parameter define here
	struct bitmap_patricia_node *p_i=NULL;
	struct bitmap_patricia_node *pNode=NULL, *node_internal=NULL;
	char *sub_str=NULL, *malloc_str, *for_free; // tmp use
	int pointer_index = 0;
	// end parameter

	if (nStartPos >= strlen(strInput))
		return ;

	// find sub_str ppChild 
	if (bitmap_get(pParent->bitmap, strInput[nStartPos])){
		// 1) input ch has sub_str, return index ParentNode pointer
		pointer_index = Count1ofBitmap(pParent->bitmap, (unsigned char)strInput[nStartPos]);
		p_i = pParent->pchild[pointer_index];
	}
	else {
		// 2) no common prefix, then return NULL
		p_i =  NULL;
	}

	//p_i = FindPath2Insert(pParent, strInput[nStartPos]);


	// 1) no common prefix, insert strInput
	if (p_i == NULL) {
		pNode = (struct bitmap_patricia_node *)calloc(1,  sizeof(struct bitmap_patricia_node) );
		NodeAssignment (
			pNode,			//  patricia trie pNode point
			nInputPort,		// next hop, if =LEAF pNode , than assign port >=0
			-1,				// PIT input port
			NULL,			// CS data
			NULL,			// token, the pNode context
			NULL,			// *bitmap 32 bytes = 256 bits
			NULL);			// **ppChild
		// assign the token context, strInput - sub_str
		pNode->token = (char *) calloc(1, (strlen(strInput + nStartPos)+1)*sizeof(char) ); // allocate new memory for split pNode
		strcpy(pNode->token, strInput + nStartPos);
		InsertChild2Parent ( pParent, pNode);

		return ;

	} 

	// 2) has the common prefix
	sub_str = LongPrefix(p_i->token, strInput + nStartPos);	//long prefix

	if (strcmp (sub_str, p_i->token) == 0 && strcmp (sub_str, strInput + nStartPos) == 0) {
		// a) face the same string, do nothing, must be LEAF
		p_i->next_hop = nInputPort;
		free(sub_str);
		sub_str = NULL;
		return ;
	} else if (strcmp (sub_str, strInput + nStartPos) == 0){
		// b) sub_str = strInput != p_i->token, split the p_i->token, and add a leaf pNode
		//   insert pNode as sub_str
		pNode = (struct bitmap_patricia_node *)calloc(1,  sizeof(struct bitmap_patricia_node) );
		NodeAssignment (
			pNode,			//  patricia trie pNode point
			nInputPort,		// port, if =LEAF pNode , than assign port >=0
			-1,
			NULL,			// cs entry
			sub_str,		// token, the pNode context
			NULL,			// *bitmap 32 bytes = 256 bits
			NULL);			// **ppChild

		// cut context to &(p_i->token[strlen(sub_str)])
		malloc_str = (char *) calloc(1, (strlen(p_i->token)-strlen(sub_str)+1)*sizeof(char) ); // allocate new memory for split pNode
		strcpy (malloc_str, &(p_i->token[strlen(sub_str)]) );	// p_i->context - sub_str
		for_free = p_i->token;
		p_i->token = malloc_str;

		free(for_free);

		// insert the pNode, make pParent->p_i => pParent-> pNode ->p_i
		UpdateChild2Parent (pParent, pNode);
		InsertChild2Parent (pNode, p_i);

		return ;

	} else if (strcmp(sub_str, p_i->token) == 0){
		bitmap_patricia_insert(p_i, strInput, nStartPos + strlen(sub_str), nInputPort);
		free(sub_str);
		sub_str = NULL;
		return ;

	} else {
		// 子串与输入和节点都只有部分重合，需要加入一个叶子节点和一个内部节点
		//   insert leaf pNode (strInput-sub_str )
		pNode = (struct bitmap_patricia_node *)calloc(1,  sizeof(struct bitmap_patricia_node) );
		NodeAssignment (
			pNode,			//  patricia trie pNode point
			nInputPort,		// port, if =LEAF pNode , than assign port >=0
			-1,
			NULL,
			NULL,			// token, the pNode context
			NULL,			// *bitmap 32 bytes = 256 bits
			NULL);			// **ppChild
		// pNode->token = strInput-sub_str
		pNode->token = (char *) calloc(1, strlen(strInput + nStartPos)-strlen(sub_str)+1);
		strcpy(pNode->token, &(strInput[nStartPos + strlen(sub_str)]));

		//    insert internal pNode
		node_internal = (struct bitmap_patricia_node *)calloc(1,  sizeof(struct bitmap_patricia_node) );
		NodeAssignment (
			node_internal,	//  patricia trie pNode point
			-1,		// port, if =LEAF pNode , than assign port >=0
			-1,
			NULL,
			sub_str,		// token, the pNode context
			NULL,			// *bitmap 32 bytes = 256 bits
			NULL);			// **ppChild

		// cut token to &(p_i->token[strlen(sub_str)])
		malloc_str = (char *) calloc(1, (strlen(p_i->token)-strlen(sub_str)+1)*sizeof(char) ); // allocate new memory for split pNode
		strcpy (malloc_str, &(p_i->token[strlen(sub_str)]) );	// p_i->context - sub_str
		for_free = p_i->token;
		p_i->token = malloc_str;
		free(for_free);
		for_free = NULL;

		// change pNode relationship, From pParent->p_i To pParent->node_internal, node_internal->p_i & node_internal->pNode
		UpdateChild2Parent(pParent, node_internal);
		InsertChild2Parent(node_internal, p_i);
		InsertChild2Parent(node_internal, pNode);

		return ;
	}

}
//
// -- end insert module -- //

//static int CountSetBit(struct bitmap_patricia_node *pNode, int ch)
//{
//	int counter=0;
//	uint32_t bmap_byte=0, bmap_bit=0;
//	uint32_t tmp_bmap;
//	bmap_byte = ch / 32;	// start from 0, so do not need +1
//	bmap_bit = ch % 32;
//
//	tmp_bmap = pNode->bitmap[bmap_byte] & bit_mask[bmap_bit];
//	counter += NumberOfSetBits(tmp_bmap);
//
//	// bit 1 in per byte 
//	while (bmap_byte--) {
//		tmp_bmap = pNode->bitmap[bmap_byte];
//		counter += NumberOfSetBits(tmp_bmap);
//	}
//
//	return counter;
//}


// Search the input line in patricia trie, if match then return the port
// return -1, mean do not match the strInput
int bitmap_patricia_lookup(struct bitmap_patricia_node *pParent, char *strInput, size_t nStartPos)
{
	struct bitmap_patricia_node *p_i=NULL;
	int nRetPort=-1;

	int prefixLen = 0;
	int inputLen = 0;
	int tokenLen = 0;

	int res = 0;
	int i = 0;
	uint32_t tmp_bmap;

	if (nStartPos >= strlen(strInput))
		return -1;

	if (pParent == NULL){
		return -1;
	}

	while (1) {
		// find sub_str ppChild 		
		tmp_bmap = bitmap_get(pParent->bitmap, (unsigned char)strInput[nStartPos]);
		if (tmp_bmap){
			// 1) input ch has sub_str, return index ParentNode pointer
			// p_i = pParent->ppChild[CountSetBit(ParentNode, strInput[nStartPos])]);
			i = Count1ofBitmap(pParent->bitmap, (unsigned char)strInput[nStartPos]);
			p_i = pParent->pchild[i];

		} else {
			return -1;
		}
		tokenLen = strlen(p_i->token);
		inputLen = strlen(strInput + nStartPos);
		prefixLen = find_longest_prefix_len_str(p_i->token, tokenLen, strInput + nStartPos, inputLen);

		if (prefixLen == inputLen) {
			if (prefixLen == tokenLen) {
				return p_i->next_hop;
			}
			return -1;
		}
		else if (prefixLen == tokenLen) {
			if (p_i->pchild != NULL){

				nStartPos += prefixLen;
				pParent = p_i;
				continue;
			} else {
				// free(sub_str);
				return -1;
			}
		}
		else {
			return -1;
		}

	}
}


//--20170722--three insert functions
void bitmap_patricia_insert_fib(struct bitmap_patricia_node *pParent, char *strInput, size_t nStartPos, int nextHop){
	// parameter define here
	struct bitmap_patricia_node *p_i = NULL;
	struct bitmap_patricia_node *pNode = NULL, *node_internal = NULL;
	char *sub_str = NULL, *malloc_str, *for_free; // tmp use
	int pointer_index = 0;
	// end parameter

	if (nStartPos >= strlen(strInput))
		return;

	// find sub_str ppChild 
	if (bitmap_get(pParent->bitmap, strInput[nStartPos])){
		// 1) input ch has sub_str, return index ParentNode pointer
		pointer_index = Count1ofBitmap(pParent->bitmap, (unsigned char)strInput[nStartPos]);
		p_i = pParent->pchild[pointer_index];
	}
	else {
		// 2) no common prefix, then return NULL
		p_i = NULL;
	}
	//p_i = FindPath2Insert(pParent, strInput[nStartPos]);

	// 1) no common prefix, insert strInput
	if (p_i == NULL) {
		pNode = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
		NodeAssignment(pNode, nextHop, -1, NULL, NULL, NULL, NULL);
		// assign the token context, strInput - sub_str
		pNode->token = (char *)calloc(1, (strlen(strInput + nStartPos) + 1)*sizeof(char)); // allocate new memory for split pNode
		strcpy(pNode->token, strInput + nStartPos);
		InsertChild2Parent(pParent, pNode);

		return;

	}

	// 2) has the common prefix
	sub_str = LongPrefix(p_i->token, strInput + nStartPos);	//long prefix

	if (strcmp(sub_str, p_i->token) == 0 && strcmp(sub_str, strInput + nStartPos) == 0) {
		// a) face the same string, do nothing, must be LEAF
		p_i->next_hop = nextHop;
		free(sub_str);
		sub_str = NULL;
		return;
	}
	else if (strcmp(sub_str, strInput + nStartPos) == 0){
		// b) sub_str = strInput != p_i->token, split the p_i->token, and add a leaf pNode
		//   insert pNode as sub_str
		pNode = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
		NodeAssignment( pNode, nextHop, -1, NULL, sub_str, NULL, NULL);

		// cut context to &(p_i->token[strlen(sub_str)])
		malloc_str = (char *)calloc(1, (strlen(p_i->token) - strlen(sub_str) + 1)*sizeof(char)); // allocate new memory for split pNode
		strcpy(malloc_str, &(p_i->token[strlen(sub_str)]));	// p_i->context - sub_str
		for_free = p_i->token;
		p_i->token = malloc_str;

		free(for_free);

		// insert the pNode, make pParent->p_i => pParent-> pNode ->p_i
		pParent->pchild[pointer_index] = pNode;
		//UpdateChild2Parent(pParent, pNode);
		InsertChild2Parent(pNode, p_i);

		return;

	}
	else if (strcmp(sub_str, p_i->token) == 0){
		bitmap_patricia_insert_fib(p_i, strInput, nStartPos+strlen(sub_str), nextHop);
		free(sub_str);
		sub_str = NULL;
		return;

	}
	else {
		// 子串与输入和节点都只有部分重合，需要加入一个叶子节点和一个内部节点
		//   insert leaf pNode (strInput-sub_str )
		pNode = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
		NodeAssignment( pNode, nextHop, -1, NULL, NULL, NULL, NULL);
		// pNode->token = strInput-sub_str
		pNode->token = (char *)calloc(1, strlen(strInput + nStartPos) - strlen(sub_str) + 1);
		strcpy(pNode->token, &(strInput[nStartPos+ strlen(sub_str)]));

		//    insert internal pNode
		node_internal = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
		NodeAssignment( node_internal, -1, -1, NULL, sub_str, NULL,	NULL);

		// cut token to &(p_i->token[strlen(sub_str)])
		malloc_str = (char *)calloc(1, (strlen(p_i->token) - strlen(sub_str) + 1)*sizeof(char)); // allocate new memory for split pNode
		strcpy(malloc_str, &(p_i->token[strlen(sub_str)]));	// p_i->context - sub_str
		for_free = p_i->token;
		p_i->token = malloc_str;
		free(for_free);
		for_free = NULL;

		// change pNode relationship, From pParent->p_i To pParent->node_internal, node_internal->p_i & node_internal->pNode
		pParent->pchild[pointer_index] = node_internal;
		//UpdateChild2Parent(pParent, node_internal);
		InsertChild2Parent(node_internal, p_i);
		InsertChild2Parent(node_internal, pNode);
		return;
	}
}

void bitmap_patricia_insert_pitnode(struct bitmap_patricia_node *pParent, char *strInput, size_t nStartPos, int nInputPort){
	struct bitmap_patricia_node *p_i = NULL;
	struct bitmap_patricia_node *pNode = NULL, *node_internal = NULL;
	char *sub_str = NULL, *malloc_str, *for_free; // tmp use
	int pointer_index = 0;
	// end parameter

	if (nStartPos >= strlen(strInput))
		return;

	// find sub_str ppChild
	if (bitmap_get(pParent->bitmap, strInput[nStartPos])){
		// 1) input ch has sub_str, return index ParentNode pointer
		pointer_index = Count1ofBitmap(pParent->bitmap, (unsigned char)strInput[nStartPos]);
		p_i = pParent->pchild[pointer_index];
	}
	else {
		// 2) no common prefix, then return NULL
		p_i = NULL;
	}
	//p_i = FindPath2Insert(pParent, strInput[nStartPos]);


	// 1) no common prefix, insert strInput
	if (p_i == NULL) {
		pNode = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
		NodeAssignment(pNode, -1, nInputPort, NULL, NULL, NULL, NULL);
		// assign the token context, strInput - sub_str
		pNode->token = (char *)calloc(1, (strlen(strInput + nStartPos) + 1)*sizeof(char)); // allocate new memory for split pNode
		strcpy(pNode->token, strInput+nStartPos);

		InsertChild2Parent(pParent, pNode);
		return;
	}

	// 2) has the common prefix
	sub_str = LongPrefix(p_i->token, strInput+nStartPos);	//long prefix

	if (strcmp(sub_str, p_i->token) == 0 && strcmp(sub_str, strInput+nStartPos) == 0) {
		// a) face the same string, do nothing, must be LEAF
		p_i->input_port = nInputPort;
		free(sub_str);
		sub_str = NULL;
		return;
	}
	else if (strcmp(sub_str, strInput+nStartPos) == 0){
		// b) sub_str = strInput != p_i->token, split the p_i->token, and add a leaf pNode
		//   insert pNode as sub_str
		pNode = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
		NodeAssignment(pNode, -1, nInputPort, NULL, sub_str, NULL, NULL);

		// cut context to &(p_i->token[strlen(sub_str)])
		malloc_str = (char *)calloc(1, (strlen(p_i->token) - strlen(sub_str) + 1)*sizeof(char)); // allocate new memory for split pNode
		strcpy(malloc_str, &(p_i->token[strlen(sub_str)]));	// p_i->context - sub_str
		for_free = p_i->token;
		p_i->token = malloc_str;

		free(for_free);
		for_free = NULL;

		// insert the pNode, make pParent->p_i => pParent-> pNode ->p_i
		//UpdateChild2Parent(pParent, pNode);
		pParent->pchild[pointer_index] = pNode;
		InsertChild2Parent(pNode, p_i);
		return;

	}
	else if (strcmp(sub_str, p_i->token) == 0){
		bitmap_patricia_insert_pitnode(p_i, strInput, nStartPos + strlen(sub_str), nInputPort);
		free(sub_str);
		sub_str = NULL;
		return;

	}
	else {
		// 子串与输入和节点都只有部分重合，需要加入一个叶子节点和一个内部节点
		//   insert leaf pNode (strInput-sub_str )
		pNode = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
		NodeAssignment(pNode, -1, nInputPort, NULL, NULL, NULL, NULL);
		// pNode->token = strInput-sub_str
		pNode->token = (char *)calloc(1, strlen(strInput + nStartPos) - strlen(sub_str) + 1);
		strcpy(pNode->token, strInput + nStartPos +strlen(sub_str));

		//    insert internal pNode
		node_internal = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
		NodeAssignment( node_internal, -1, -1, NULL, sub_str, NULL, NULL);

		// cut token to &(p_i->token[strlen(sub_str)])
		malloc_str = (char *)calloc(1, (strlen(p_i->token) - strlen(sub_str) + 1)*sizeof(char)); // allocate new memory for split pNode
		strcpy(malloc_str, &(p_i->token[strlen(sub_str)]));	// p_i->context - sub_str
		for_free = p_i->token;
		p_i->token = malloc_str;
		free(for_free);
		for_free = NULL;

		// change pNode relationship, From pParent->p_i To pParent->node_internal, node_internal->p_i & node_internal->pNode
		//UpdateChild2Parent(pParent, node_internal);
		pParent->pchild[pointer_index] = node_internal;
		InsertChild2Parent(node_internal, p_i);
		InsertChild2Parent(node_internal, pNode);
		return;
	}
}

void bitmap_patricia_insert_CSnode(struct bitmap_patricia_node *pParent, char *strInput, size_t nStartPos, char * data){
	// parameter define here
	struct bitmap_patricia_node *p_i = NULL;
	struct bitmap_patricia_node *pNode = NULL, *node_internal = NULL;
	char *sub_str = NULL, *malloc_str, *for_free; // tmp use
	int pointer_index = 0;
	// end parameter

	if (pParent == NULL || strInput == NULL || nStartPos > strlen(strInput))
		return;

	if (nStartPos == strlen(strInput) && data){ // 传入的字符串匹配到父节点  待验证
		pParent->pdata = data;
	}

	// find sub_str ppChild 
	if (bitmap_get(pParent->bitmap, strInput[nStartPos])){
		// 1) input ch has sub_str, return index ParentNode pointer
		pointer_index = Count1ofBitmap(pParent->bitmap, (unsigned char)strInput[nStartPos]);
		p_i = pParent->pchild[pointer_index];
	}
	else {
		// 2) no common prefix, then return NULL
		p_i = NULL;
	}
	//p_i = FindPath2Insert(pParent, strInput[nStartPos]);


	// 1) no common prefix, insert strInput
	if (p_i == NULL) {
		pNode = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
		NodeAssignment(pNode, -1, -1, data, NULL, NULL, NULL);
		// assign the token context, strInput - sub_str
		pNode->token = (char *)calloc(1, (strlen(strInput + nStartPos) + 1)*sizeof(char)); // allocate new memory for split pNode
		strcpy(pNode->token, strInput + nStartPos);
		InsertChild2Parent(pParent, pNode);
		return;

	}

	// 2) has the common prefix
	sub_str = LongPrefix(p_i->token, strInput + nStartPos);	//long prefix

	if (strcmp(sub_str, p_i->token) == 0 && strcmp(sub_str, strInput + nStartPos) == 0) {
		// a) face the same string, do nothing, must be LEAF
		p_i->pdata = data;
		free(sub_str);
		sub_str = NULL;
		return;
	}
	else if (strcmp(sub_str, strInput + nStartPos) == 0){
		// b) sub_str = strInput != p_i->token, split the p_i->token, and add a leaf pNode
		//   insert pNode as sub_str
		pNode = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
		NodeAssignment(pNode, -1, -1, data, sub_str, NULL, NULL);

		// cut context to &(p_i->token[strlen(sub_str)])
		malloc_str = (char *)calloc(1, (strlen(p_i->token) - strlen(sub_str) + 1)*sizeof(char)); // allocate new memory for split pNode
		strcpy(malloc_str, &(p_i->token[strlen(sub_str)]));	// p_i->context - sub_str
		for_free = p_i->token;
		p_i->token = malloc_str;

		free(for_free);

		// insert the pNode, make pParent->p_i => pParent-> pNode ->p_i
		//UpdateChild2Parent(pParent, pNode);
		pParent->pchild[pointer_index] = pNode;
		InsertChild2Parent(pNode, p_i);
		return;

	}
	else if (strcmp(sub_str, p_i->token) == 0){
		bitmap_patricia_insert_CSnode(p_i, strInput, nStartPos + strlen(sub_str), data);
		free(sub_str);
		sub_str = NULL;
		return;

	}
	else {
		// 子串与输入和节点都只有部分重合，需要加入一个叶子节点和一个内部节点
		//   insert leaf pNode (strInput-sub_str )
		pNode = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
		NodeAssignment(pNode, -1, -1, data, NULL, NULL, NULL);
		// pNode->token = strInput-sub_str
		pNode->token = (char *)calloc(1, strlen(strInput + nStartPos) - strlen(sub_str) + 1);
		strcpy(pNode->token, &(strInput[nStartPos + strlen(sub_str)]));

		//    insert internal pNode
		node_internal = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
		NodeAssignment(node_internal, -1, -1, NULL, sub_str, NULL, NULL);

		// cut token to &(p_i->token[strlen(sub_str)])
		malloc_str = (char *)calloc(1, (strlen(p_i->token) - strlen(sub_str) + 1)*sizeof(char)); // allocate new memory for split pNode
		strcpy(malloc_str, &(p_i->token[strlen(sub_str)]));	// p_i->context - sub_str
		for_free = p_i->token;
		p_i->token = malloc_str;
		free(for_free);
		for_free = NULL;

		// change pNode relationship, From pParent->p_i To pParent->node_internal, node_internal->p_i & node_internal->pNode
		//UpdateChild2Parent(pParent, node_internal);
		pParent->pchild[pointer_index] = node_internal;
		InsertChild2Parent(node_internal, p_i);
		InsertChild2Parent(node_internal, pNode);
		return;
	}
}

// 将一个节点根据strInput的输入分割成两个节点，返回父节点。针对token="/abc/b",str="/abc","/abc/d","/abc/b/c"
void devide_bitmap_patricia_node(struct bitmap_patricia_node *node, char *strInput, size_t *nStartPos){
	char* sub_str = NULL, *malloc_str = NULL, *for_free = NULL;
	char* input = NULL;
	struct bitmap_patricia_node* p_node = NULL, *copy_node = NULL, *new_node = NULL;

	if (node == NULL || strInput == NULL || (*nStartPos) >= strlen(strInput)){
		return;
	}
	if (!node->token){ // 到达root节点，没有token信息
		return;
	}
	input = strInput + (*nStartPos);
	sub_str = LongPrefix(node->token, input);	//long prefix
	(*nStartPos) += strlen(sub_str);

	if (strcmp(sub_str, node->token) == 0 && strcmp(sub_str, input) == 0) { //匹配到当前节点
		free(sub_str);
		sub_str = NULL;
		return;
	}
	else if (strcmp(sub_str, input) == 0){
		copy_node = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
		memcpy(copy_node, node, sizeof(struct bitmap_patricia_node));

		memset(node, 0, sizeof(struct bitmap_patricia_node));
		NodeAssignment(node, -1, -1, NULL, sub_str, NULL, NULL);

		malloc_str = (char *)calloc(1, (strlen(copy_node->token) - strlen(sub_str) + 1)*sizeof(char)); // allocate new memory for split pNode
		strcpy(malloc_str, copy_node->token + strlen(sub_str));
		for_free = copy_node->token;
		copy_node->token = malloc_str;
		free(for_free);

		InsertChild2Parent(node, copy_node);
		return;

	}
	else if (strcmp(sub_str, node->token) == 0){
		free(sub_str);
		sub_str = NULL;
		return;

	}
	else { //仅分割节点，不插入data name
		copy_node = (struct bitmap_patricia_node *)calloc(1, sizeof(struct bitmap_patricia_node));
		memcpy(copy_node, node, sizeof(struct bitmap_patricia_node));
		
		memset(node, 0, sizeof(struct bitmap_patricia_node));
		NodeAssignment(node, -1, -1, NULL, sub_str, NULL, NULL);

		// cut token to &(p_i->token[strlen(sub_str)])
		malloc_str = (char *)calloc(1, (strlen(copy_node->token) - strlen(sub_str) + 1)*sizeof(char)); // allocate new memory for split pNode
		strcpy(malloc_str, copy_node->token + strlen(sub_str));	// p_i->context - sub_str
		for_free = copy_node->token;
		copy_node->token = malloc_str;
		free(for_free);
		for_free = NULL;

		InsertChild2Parent(node, copy_node);
		return;
	}
}


struct bitmap_patricia_node* bitmap_patricia_find_node(struct bitmap_patricia_node *pParent, char *strInput, size_t *nStartPos, int *flag){
	struct bitmap_patricia_node *p_i = NULL;

	int prefixLen = 0;
	int inputLen = 0;
	int tokenLen = 0;

	int i = 0;
	char *input;
	uint32_t tmp_bmap;

	if (*nStartPos >= strlen(strInput))
		return pParent;

	if (pParent == NULL){
		return pParent;
	}

	while (1) {
		// find sub_str ppChild 
		tmp_bmap = bitmap_get(pParent->bitmap, (unsigned char)strInput[*nStartPos]);
		if (tmp_bmap){
			// 1) input ch has sub_str, return index ParentNode pointer
			// p_i = pParent->ppChild[CountSetBit(ParentNode, strInput[nStartPos])]);
			i = Count1ofBitmap(pParent->bitmap, (unsigned char)strInput[*nStartPos]);
			p_i = pParent->pchild[i];

		}
		else {
			return pParent;
		}
		input = strInput + *nStartPos;
		tokenLen = strlen(p_i->token);
		inputLen = strlen(input);


		prefixLen = tokenLen > inputLen ? inputLen : tokenLen;
		for (i = 0; i != prefixLen; ++i) {
			if (p_i->token[i] != input[i]) {
				prefixLen = i;
				break;
			}
		}

		if (prefixLen == inputLen) {
			if (prefixLen == tokenLen) {
				if (p_i->next_hop > 0){ //路径上存在FIB匹配项
					*flag = 1;
				}
				(*nStartPos) += prefixLen;
				return p_i;
			}
			return pParent;
		}
		else if (prefixLen == tokenLen) {
			(*nStartPos) += prefixLen;
			pParent = p_i;
			if (pParent->next_hop > 0){ //路径上存在FIB匹配项
				*flag = 1;
			}
			if (p_i->pchild != NULL){
				continue;
			}
			else {
				return pParent;
			}
		}
		else {
			return pParent;
		}

	}
}

// match the FIB leaf, FIB internal node, PIT leaf, PIT internal node.
void bitmap_patricia_insert_pit(struct bitmap_patricia_node *pParent, char *strInput, size_t nStartPos, int nInputPort, int flag){
	struct bitmap_patricia_node* match_node = NULL; //匹配到的父节点
	size_t start_pos = 0, len = strlen(strInput);
	int insert_flag = flag;

	if (pParent == NULL || strInput == NULL || nStartPos >= strlen(strInput)){
		return;
	}
	match_node = bitmap_patricia_find_node(pParent, strInput, &start_pos, &insert_flag);

	if (match_node->next_hop > 0){
		insert_flag = 1;
	}
	if (!insert_flag){ //匹配到FIB internal node不存在相应的PIT entry
		return; //store in default path，未完成
	}
	else{
		if (start_pos == len){  //matched
			match_node->input_port = nInputPort; // 将Input port 加入到节点中，未完成
		}
		else{
			bitmap_patricia_insert_pitnode(match_node, strInput, start_pos, nInputPort);
		}
	}
	return;
}

void bitmap_patricia_insert_CS(struct bitmap_patricia_node *pParent, char *strInput, size_t nStartPos, char* data, int flag){
	struct bitmap_patricia_node* match_node = NULL; //匹配到的父节点
	size_t start_pos = 0, len = strlen(strInput);
	int insert_flag = flag;

	if (pParent == NULL || strInput == NULL || nStartPos >= strlen(strInput)){
		return;
	}
	consume_bitmap_patricia_pitNode(pParent, strInput); //先删除对应的PIT表项（这里是一个）

	match_node = bitmap_patricia_find_node(pParent, strInput, &start_pos, &insert_flag);

	if (match_node->next_hop > 0){
		insert_flag = 1;
	}
	if (!insert_flag){ //匹配到FIB internal node不存在相应的CS entry
		return; //store in default path，未完成
	}
	else{
		if (start_pos == len){  //matched
			match_node->pdata = data; // 将数据更新到节点中
		}
		else{
			bitmap_patricia_insert_CSnode(match_node, strInput, start_pos, data);
		}
	}
	return;
}


//--20170724--three delete functions
struct bitmap_patricia_node * find_exact_match_Node(struct bitmap_patricia_node **pParent, char *strInput, uint32_t* index){
	struct bitmap_patricia_node *p_i = NULL;
	int prefixLen = 0, inputLen = 0, tokenLen = 0;
	uint32_t tmp_bmap = 0, i = 0;
	uint32_t start_pos = 0;

	if (pParent == NULL || strInput == NULL || strlen(strInput) == 0){
		return NULL;
	}

	while (1){
		//find sub_str ppChild
		tmp_bmap = bitmap_get((*pParent)->bitmap, (unsigned char)strInput[start_pos]);
		if (tmp_bmap){
			i = Count1ofBitmap((*pParent)->bitmap, (unsigned char)strInput[start_pos]);
			p_i = (*pParent)->pchild[i];
			(*index) = i;
		}
		else{ // don't exist
			break;
		}
		tokenLen = strlen(p_i->token);
		inputLen = strlen(strInput + start_pos);
		prefixLen = find_longest_prefix_len_str(p_i->token, tokenLen, strInput + start_pos, inputLen);

		if (prefixLen == inputLen && prefixLen == tokenLen){ //exact match the node return the parent node
			return p_i;
		}
		else if (prefixLen == tokenLen && p_i->pchild){
			start_pos += prefixLen;
			(*pParent) = p_i;
			continue;
		}
		else{
			break;
		}

	}
	return NULL;
}

struct bitmap_patricia_node * find_one_match_PitNode(struct bitmap_patricia_node **pParent, char *strInput, uint32_t* index){
	struct bitmap_patricia_node *p_i = NULL;
	int prefixLen = 0, inputLen = 0, tokenLen = 0;
	uint32_t tmp_bmap = 0, i = 0;
	uint32_t start_pos = 0;

	if (pParent == NULL || strInput == NULL || strlen(strInput) == 0){
		return NULL;
	}

	while (1){
		//find sub_str ppChild
		tmp_bmap = bitmap_get((*pParent)->bitmap, (unsigned char)strInput[start_pos]);
		if (tmp_bmap){
			i = Count1ofBitmap((*pParent)->bitmap, (unsigned char)strInput[start_pos]);
			p_i = (*pParent)->pchild[i];
			(*index) = i;
		}
		else{ // don't exist
			break;
		}
		tokenLen = strlen(p_i->token);
		inputLen = strlen(strInput + start_pos);
		prefixLen = find_longest_prefix_len_str(p_i->token, tokenLen, strInput + start_pos, inputLen);

		if (prefixLen == tokenLen){
			if (p_i->input_port > 0){ //该节点是一个PIT leaf，不一定是最长匹配或完全匹配，若是需要全匹配可以建一个列表
				return p_i;
			}
			else if (p_i->pchild){
				start_pos += prefixLen;
				(*pParent) = p_i;
				continue;
			}
			break;
		}
		else{
			break;
		}
	}
	return NULL;
}

int Is_internal_node(struct bitmap_patricia_node * node){
	if (node->input_port < 0 && node->next_hop < 0 && node->pdata == NULL){
		return 1;
	}
	return 0;
}

//free node
void free_bitmap_patricia_node(struct bitmap_patricia_node * node){
	struct bitmap_patricia_node* p = NULL;
	int num = 0, i;
	if (node == NULL){
		return;
	}

	if (node->token){
		free(node->token);
		node->token = NULL;
	}
	if (node->pdata){
		free(node->pdata);
		node->pdata = NULL;
	}
	if (node->pchild){
		num = Count1ofBitmap(node->bitmap, 256);
	}
	for (i = 0; i < num; i++){
		if (node->pchild[i]){
			free_bitmap_patricia_node(node->pchild[i]);
			node->pchild[i] = NULL;
		}
	}
	free(node->pchild);
	node->pchild = NULL;
	free(node);
	node = NULL;
	return;
}

void free_CSdataList(struct CSdataList* node){
	struct CSdataList * tmp = NULL;
	if (node == NULL){
		return;
	}
	while (node){
		tmp = node;
		node = node->next;
		if (tmp->data){
			free(tmp->data);
			tmp->data = NULL;
		}
		free(tmp);
		tmp = NULL;
	}
	return;
}

//delete the specific child node of parent node
void remove_node_from_childList(struct bitmap_patricia_node * node, uint32_t n, uint32_t index){
	struct bitmap_patricia_node **relloc_child = NULL;
	int i = 0, j = 0, total_num;
	total_num = Count1ofBitmap(node->bitmap, 256);
	if (total_num == 0 || node->pchild == NULL || bitmap_get(node->bitmap, n) == 0)
		return;

	bitmap_clear(node->bitmap, n); //clear bitmap
	total_num -= 1;
	if (total_num != 0){
		relloc_child = (struct bitmap_patricia_node**) calloc(1, sizeof(struct bitmap_patricia_node *) * total_num);
		for (i = 0, j = 0; i <= total_num; i++){
			if (i == index){
				continue;
			}
			relloc_child[j++] = node->pchild[i];
		}
	}
	
	free(node->pchild);
	node->pchild = relloc_child;
	return;
}

void merge_oneNode_withParent(struct bitmap_patricia_node ** pParent){
	struct bitmap_patricia_node *one_node = NULL, *new_node = NULL;
	char * new_token = NULL;

	one_node = (*pParent)->pchild[0];
	remove_node_from_childList((*pParent), (unsigned char)one_node->token[0], 0);
	new_token = (char*)malloc(strlen((*pParent)->token) + strlen(one_node->token) + 1);
	strcpy(new_token, (*pParent)->token);
	strcat(new_token, one_node->token);
	new_node = (struct bitmap_patricia_node*) calloc(1, sizeof(struct bitmap_patricia_node));
	memcpy(new_node, (*pParent), sizeof(struct bitmap_patricia_node));
	memcpy((*pParent), one_node, sizeof(struct bitmap_patricia_node));
	free_bitmap_patricia_node(new_node);
	new_node = NULL;
	free((*pParent)->token);
	(*pParent)->token = new_token;
	free(one_node);
	one_node = NULL;
	return;
}

void consume_bitmap_patricia_pitNode(struct bitmap_patricia_node *pParent, char* strInput){
	struct bitmap_patricia_node * parent_node = pParent, *match_node = NULL;

	int index = 0, num = 0, num1 = 0;

	match_node = find_one_match_PitNode(&parent_node, strInput, &index);
	if (match_node == NULL){
		printf("the pit name do not exist.\n");
		return;
	}

	// match the node, delete it
	num = Count1ofBitmap(match_node->bitmap, 256);
	if (num == 0){//Leaf node
		//delete parent children list and return the existing node number
		remove_node_from_childList(parent_node, (unsigned char)match_node->token[0], index);
		free_bitmap_patricia_node(match_node); //delete leaf node
		match_node = NULL;
	}
	else{
		match_node->input_port = -1;
		if (num == 1 && Is_internal_node(match_node)){ //当前节点变为 不含叶子信息的中间节点
			merge_oneNode_withParent(&match_node);
		}
	}
	num1 = Count1ofBitmap(parent_node->bitmap, 256);
	if (num1 == 1 && Is_internal_node(parent_node)){ //只剩一个孩子节点，且父节点不含叶子信息，合并节点
		merge_oneNode_withParent(&parent_node);
	}
	return;
}

//注意可能会删除delete_node与delete_node的children
void consume_bitmap_patricia_PitWithNode(struct bitmap_patricia_node* pParent, struct bitmap_patricia_node** delete_node, int index){
	int num = 0, num1 = 0;
	// match the node, delete it
	num = Count1ofBitmap((*delete_node)->bitmap, 256);
	if (num == 0){//Leaf node
		//delete parent children list and return the existing node number
		remove_node_from_childList(pParent, (unsigned char)(*delete_node)->token[0], index);
		free_bitmap_patricia_node((*delete_node)); //delete leaf node
		(*delete_node) = NULL;
	}
	else{
		(*delete_node)->input_port = -1;
		if (num == 1 && Is_internal_node((*delete_node))){ //当前节点变为 不含叶子信息的中间节点
			merge_oneNode_withParent(delete_node);
		}
	}
	num1 = Count1ofBitmap(pParent->bitmap, 256);
	if (num1 == 1 && Is_internal_node(pParent)){ //只剩一个孩子节点，且父节点不含叶子信息，合并节点
		merge_oneNode_withParent(&pParent);
	}
}

void del_bitmap_patricia_cs(struct bitmap_patricia_node *pParent, char* strInput){
	struct bitmap_patricia_node* parent_node = pParent, *match_node = NULL;
	int num, num1;
	uint32_t index = 0;

	match_node = find_exact_match_Node(&parent_node, strInput, &index);
	if (match_node == NULL){
		printf("the url do not exist.\n");
		return;
	}

	// match the node, delete it
	num = Count1ofBitmap(match_node->bitmap, 256);
	if (num == 0){//Leaf node
		//delete parent children list and return the existing node number
		remove_node_from_childList(parent_node, (unsigned char)match_node->token[0], index);
		free_bitmap_patricia_node(match_node); //delete leaf node
		match_node = NULL;
	}
	else{
		//free(match_node->pdata->pData);
		//match_node->pdata->pData = NULL;
		free(match_node->pdata);
		match_node->pdata = NULL;
		if (num == 1 && Is_internal_node(match_node)){
			merge_oneNode_withParent(&match_node);
		}
	}
	num1 = Count1ofBitmap(parent_node->bitmap, 256);
	if (num1 == 1 && Is_internal_node(parent_node)){ //只剩一个孩子节点，且父节点不包含CS entry，合并节点
		merge_oneNode_withParent(&parent_node);
	}
	return;
}

void del_bitmap_patricia_pit(struct bitmap_patricia_node *pParent, char* strInput){
	struct bitmap_patricia_node * parent_node = pParent, *deleted_node = NULL;

	int index, num, num1;

	deleted_node = find_exact_match_Node(&parent_node, strInput, &index);
	if (deleted_node == NULL){
		printf("the url do not exist.\n");
		return;
	}

	// match the node, delete it
	num = Count1ofBitmap(deleted_node->bitmap, 256);
	if (num == 0){//Leaf node
		//delete parent children list and return the existing node number
		remove_node_from_childList(parent_node, (unsigned char)deleted_node->token[0], index);
		free_bitmap_patricia_node(deleted_node); //delete leaf node
		deleted_node = NULL;
	}
	else{
		deleted_node->input_port = -1;
		if (num == 1 && Is_internal_node(deleted_node)){
			merge_oneNode_withParent(&deleted_node);
		}
	}
	num1 = Count1ofBitmap(parent_node->bitmap, 256);
	if (num1 == 1 && Is_internal_node(parent_node)){ //只剩一个孩子节点，且父节点不含叶子信息，合并节点
		merge_oneNode_withParent(&parent_node);
	}
	return;
}


// 2017-07-26 match function
int bitmap_patricia_longest_match_fib(struct bitmap_patricia_node *pParent, char *strInput){
	int hop = -1;
	unsigned int start_pos = 0, i = 0;
	int tokenLen = 0, prefixLen = 0, inputLen = 0;
	struct bitmap_patricia_node * p_i;

	if (pParent == NULL || strInput == NULL){
		return -1;
	}

	while (1){
		if (!bitmap_get(pParent->bitmap, (unsigned char)strInput[start_pos])) //不存在对应孩子节点
			break;
		else{
			i = Count1ofBitmap(pParent->bitmap, (unsigned char)strInput[start_pos]);
			p_i = pParent->pchild[i];
		}
		tokenLen = strlen(p_i->token);
		inputLen = strlen(strInput + start_pos);
		prefixLen = find_longest_prefix_len_str(p_i->token, tokenLen, strInput + start_pos, inputLen);

		if (prefixLen == tokenLen){
			if (p_i->next_hop > 0){
				hop = p_i->next_hop; //若求最长前缀匹配的节点，hop信息换成节点指针即可
			}
			if (prefixLen != inputLen && p_i->pchild){
				start_pos += prefixLen;
				pParent = p_i;
				continue;
			}
			else{
				break;
			}
		}
		else{
			break;
		}
	}
	return hop;
}

// necessary?
int bitmap_patricia_exact_match_pit(struct bitmap_patricia_node *pParent, char *strInput){
	int port = -1;
	unsigned int start_pos = 0, i = 0;
	int tokenLen = 0, prefixLen = 0, inputLen = 0;
	struct bitmap_patricia_node * p_i;

	if (pParent == NULL || strInput == NULL){
		return -1;
	}

	while (1){
		if (!bitmap_get(pParent->bitmap, (unsigned char)strInput[start_pos])) //不存在对应孩子节点
			break;
		else{
			i = Count1ofBitmap(pParent->bitmap, (unsigned char)strInput[start_pos]);
			p_i = pParent->pchild[i];
		}
		tokenLen = strlen(p_i->token);
		inputLen = strlen(strInput + start_pos);
		prefixLen = find_longest_prefix_len_str(p_i->token, tokenLen, strInput + start_pos, inputLen);

		if (prefixLen == tokenLen && prefixLen == inputLen){ //exact match
			if (p_i->input_port > 0){
				port = p_i->input_port; //若求最长前缀匹配的节点，hop信息换成节点指针即可
			}
			break;
		}
		else if (prefixLen == tokenLen && p_i->pchild){
				start_pos += prefixLen;
				pParent = p_i;
				continue;
		}
		else{
			break;
		}
	}
	return port;
}

//返回PIT匹配的数据
struct CSdataList* bitmap_patricia_deepest_match_cs(struct bitmap_patricia_node *pParent, char *strInput){
	struct CSdataList* datalist = NULL;

	unsigned int start_pos = 0, i = 0;
	int tokenLen = 0, prefixLen = 0, inputLen = 0;
	struct bitmap_patricia_node * p_i;

	if (pParent == NULL || strInput == NULL){
		return datalist;
	}

	while (1){
		if (!bitmap_get(pParent->bitmap, (unsigned char)strInput[start_pos])) //不存在对应孩子节点
			break;
		else{
			i = Count1ofBitmap(pParent->bitmap, (unsigned char)strInput[start_pos]);
			p_i = pParent->pchild[i];
		}
		tokenLen = strlen(p_i->token);
		inputLen = strlen(strInput + start_pos);
		prefixLen = find_longest_prefix_len_str(p_i->token, tokenLen, strInput + start_pos, inputLen);

		if (prefixLen == tokenLen && prefixLen == inputLen){
			if (datalist == NULL)
			{
				datalist = (struct CSdataList*)calloc(1, sizeof(struct CSdataList));
			}
			getSubTreeCSDataSet(p_i, datalist); //获取以该节点为根节点的子树包含的data
			if (datalist->next == NULL){
				free(datalist);
				datalist = NULL;
			}
			break;
		}
		else if (prefixLen == tokenLen && p_i->pchild){
			start_pos += prefixLen;
			pParent = p_i;
			continue;
		}
		else if (prefixLen == inputLen && p_i->token[prefixLen] == '/'){//满足一个component
			if (datalist == NULL)
			{
				datalist = (struct CSdataList*)calloc(1, sizeof(struct CSdataList));
			}
			getSubTreeCSDataSet(p_i, datalist); //获取以该节点为根节点的子树包含的data
			if (datalist->next == NULL){
				free(datalist);
				datalist = NULL;
			}
		}
		else{
			break;
		}
	}
	return datalist;
}

void getSubTreeCSDataSet(struct bitmap_patricia_node *pParent, struct CSdataList* dataset){
	struct CSdataList* new_data = NULL;
	int datalen = 0;
	uint32_t num = 0, i = 0;

	if (pParent == NULL){
		return;
	}

	if (pParent->pdata){
		datalen = strlen(pParent->pdata);
		new_data = (struct CSdataList*) calloc(1, sizeof(struct CSdataList));
		new_data->data = (char *)malloc(sizeof(char)*(datalen + 1));
		strcpy(new_data->data, pParent->pdata);
		new_data->next = dataset->next;
		dataset->next = new_data;
	}
	
	if (pParent->pchild){
		num = Count1ofBitmap(pParent->bitmap, 256);
		for (i = 0; i < num; i++){
			getSubTreeCSDataSet(pParent->pchild[i], dataset);
		}
	}

	return;
}


//Interest到达后查询树，获取查询结果 hop为第一个component的hop信息
struct InterestResult* deal_Interest_in_bitmap_patricia(struct bitmap_patricia_node *pParent, char* strInput, int nInputPort, int hop){
	struct InterestResult* res = NULL;
	struct CSdataList* datalist = NULL;
	size_t start_pos = 0, i = 0;
	int tokenLen = 0, prefixLen = 0, inputLen = 0;
	int tmpbmap = 0;
	struct bitmap_patricia_node * p_i;
	char *input = NULL;

	if (pParent == NULL || strInput == NULL){
		return res;
	}
	datalist = (struct CSdataList*)calloc(1, sizeof(struct CSdataList));

	while (1){
		tmpbmap = bitmap_get(pParent->bitmap, (unsigned char)strInput[start_pos]);
		if (!tmpbmap){ //没有匹配的孩子节点
			break;
		}
		else{
			i = Count1ofBitmap(pParent->bitmap, (unsigned char)strInput[start_pos]);
			p_i = pParent->pchild[i];
		}
		input = strInput + start_pos;
		tokenLen = strlen(p_i->token);
		inputLen = strlen(input);
		prefixLen = find_longest_prefix_len_str(p_i->token, tokenLen, strInput + start_pos, inputLen);

		if (prefixLen == tokenLen && prefixLen == inputLen){ //完全匹配到结点
			if (p_i->next_hop > 0){
				hop = p_i->next_hop;
			}
			//判断是否有CS缓存
			getSubTreeCSDataSet(p_i, datalist); //获取以该节点为根节点的子树包含的data
			break;
		}
		else if (prefixLen == tokenLen){
			if (p_i->next_hop > 0){
				hop = p_i->next_hop; //若求最长前缀匹配的节点，hop信息换成节点指针即可
			}
			if (p_i->pchild){
				start_pos += prefixLen;
				pParent = p_i;
				continue;
			}
			else{ // 没有更长的匹配前缀，完成最长前缀匹配
				break;
			}
		}
		else if (prefixLen == inputLen && p_i->token[prefixLen] == '/'){//是一个component
			getSubTreeCSDataSet(p_i, datalist); //获取以该节点为根节点的子树包含的data
			break;
		}
		else{
			break;
		}
	}
	if (!datalist->next && hop != -1){//无缓存数据且含FIB Leaf 插入Interest name
		bitmap_patricia_insert_pitnode(pParent, strInput, start_pos, nInputPort);
	}
	if (hop != -1 || datalist->next){ //含有Data或下一跳信息
		res = (struct InterestResult*)calloc(1, sizeof(struct InterestResult));
		res->hop = hop;
		res->datalist = datalist->next;
	}
	free(datalist);
	datalist = NULL;
	return res;
}

//Data到达后的查询操作，port为之前匹配到的PIT entry的port
struct PortList* deal_Data_in_bitmap_patricia(struct bitmap_patricia_node *pParent, char* strInput, char * data, int port){
	struct bitmap_patricia_node *p_i = NULL;
	size_t prefixLen = 0, inputLen = 0, tokenLen = 0, parentLen = 0;
	uint32_t tmp_bmap = 0, i = 0;
	size_t start_pos = 0;
	char * input = NULL;
	struct PortList* portList = NULL, *listHead = NULL, *tmpport = NULL;

	if (pParent == NULL || strInput == NULL || strlen(strInput) == 0){
		return NULL;
	}

	if (port != -1){
		portList = (struct PortList*) calloc(1, sizeof(struct PortList));
		portList->port = port;
		portList->next = NULL;
		listHead = portList;
	}

	while (1){
		//find sub_str ppChild
		tmp_bmap = bitmap_get(pParent->bitmap, (unsigned char)strInput[start_pos]);
		if (tmp_bmap){
			i = Count1ofBitmap(pParent->bitmap, (unsigned char)strInput[start_pos]);
			p_i = pParent->pchild[i];
		}
		else{ // don't exist
			break;
		}
		if (pParent->token)
			parentLen = strlen(pParent->token);
		input = strInput + start_pos;
		tokenLen = strlen(p_i->token);
		inputLen = strlen(input);
		prefixLen = find_longest_prefix_len_str(p_i->token, tokenLen, strInput + start_pos, inputLen);

		if (prefixLen == tokenLen){
			if (p_i->input_port > 0 && (prefixLen == inputLen || strInput[start_pos + prefixLen] == '/')){ //该节点是一个PIT leaf，不一定是最长匹配或完全匹配，若是需要全匹配可以建一个列表
				tmpport = (struct PortList*) calloc(1, sizeof(struct PortList));
				tmpport->port = p_i->input_port;
				tmpport->next = NULL;

				if (!listHead){
					listHead = tmpport;
					portList = tmpport;
				}
				else{
					portList->next = tmpport;
					portList = portList->next;
				}

				// match the node, delete pit entry,and add data name
				consume_bitmap_patricia_PitWithNode(pParent, &p_i, i);
				
				if (!p_i){
					start_pos -= parentLen;
					devide_bitmap_patricia_node(pParent, strInput, &start_pos);
					break;
				}
			}
			if (prefixLen < strlen(p_i->token)){ //合并节点后，token可能会合并, 需继续检索p_i节点
				continue;
			}
			if (p_i->pchild){
				start_pos += prefixLen;
				pParent = p_i;
				continue;
			}
			break;
		}
		else{
			break;
		}
	}
	if (listHead != NULL){ //合并后改变了parent的信息
		bitmap_patricia_insert_CSnode(pParent, strInput, start_pos, data);
	}
	return listHead; // 返回portlist
}