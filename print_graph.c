#include "print_graph.h"

//输出小规模数据集的示例图
// bitmap_patricia node token[0] 是区分位 
void bitmap_patricia_graphviz_dfs(struct bitmap_patricia_node *node, FILE *fp, int name){
	int child_count = 0, i = 0;
	char ch;
	char data[10] = "";

	if (node == NULL){
		return;
	}

	if (node->pdata){
		strcpy(data, node->pdata);
	}

	if (node->pchild == NULL){ //叶子节点
		fprintf(fp, "%d [shape=doublecircle, label=\"%s %d %d %s\"];\n", name, node->token + 1, node->next_hop, node->input_port, data);
	}
	else{
		if (node->next_hop >= 0){
			fprintf(fp, "%d [shape=doublecircle, label = \"%s %d %d %s\"];\n", name, node->token + 1, node->next_hop, node->input_port, data);
		}
		else if(node->token != NULL){
			fprintf(fp, "%d [label = \"%s %d %d %s\"];\n", name, node->token + 1, node->next_hop, node->input_port, data);
		}
		child_count = Count1ofBitmap(node->bitmap, 256);

		for (i = 0; i != child_count; ++i) {
			ch = node->pchild[i]->token[0];
			fprintf(fp, "%d -> %d [label=\"%c\"];\n", name, ++counter, ch);
			bitmap_patricia_graphviz_dfs(node->pchild[i], fp, counter);
		}
	}
	return;
}

void component_patricia_graphviz_dfs(struct component_patricia_node *node, FILE *fp, int name, int depth){
	int bucket_size = 0, i = 0;
	if (node == NULL){
		return;
	}

	if (node->bucket == NULL){ //叶子节点
		fprintf(fp, "%d [shape=doublecircle, label=\"%s %d\"];\n", name, node->token, node->port);
	}
	else{
		if (node->port > 0){ //含有叶子信息
			fprintf(fp, "%d [label = \"%s %d\"];\n", name, node->token, node->port);
		}
		else{
			fprintf(fp, "%d [label = \"%s\"];\n", name, node->token);
		}
		if (depth == 0){
			bucket_size = BUCKET_SIZE_FIRST;
		}
		else{
			bucket_size = BUCKET_SIZE_SECOND;
		}
		struct component_patricia_node* tmpnode = NULL;
		for (i = 0; i != bucket_size; ++i){
			tmpnode = node->bucket[i];
			while (tmpnode != NULL){
				fprintf(fp, "%d -> %d;\n", name, ++counter);
				component_patricia_graphviz_dfs(tmpnode, fp, counter, depth + 1);
				tmpnode = tmpnode->next;
			}
		}
	}
	return;
}

void component_byte_patricia_graphviz_dfs(struct component_byte_patricia_node **node, FILE *fp, int name){
	struct component_byte_patricia_node* pnode = NULL;
	int i = 0, tmp_name;

	fprintf(fp, "%d [label = \"root\"];\n", name);
	for (i = 0; i != BUCKET_LEN; ++i){
		pnode = node[i];
		while (pnode != NULL){
			fprintf(fp, "%d -> %d;\n", name, ++counter);
			tmp_name = counter;
			if (pnode->pt_root == NULL || pnode->pt_root->pchild == NULL){ //pt_root不可能为NULL吧？byte_patricia根节点为空时
				if (pnode->domain_name != NULL && pnode->next_hop >= 0)
					fprintf(fp, "%d [shape=doublecircle, label=\"%s %d\"];\n", tmp_name, pnode->domain_name, pnode->next_hop);
				else if (pnode->next_hop >= 0){
					fprintf(fp, "%d [shape=doublecircle, label=\"%d\"];\n", tmp_name, pnode->next_hop);
				}
				else if (pnode->domain_name != NULL){
					fprintf(fp, "%d [shape=doublecircle, label=\"%s\"];\n", tmp_name, pnode->domain_name);
				}
				else{
					fprintf(fp, "%d [shape=doublecircle, label=\"ERR\"];\n", tmp_name); //error
				}
			}
			else{
				if (pnode->domain_name != NULL && pnode->next_hop >= 0){
					fprintf(fp, "%d [label=\"%s %d\"];\n", tmp_name, pnode->domain_name, pnode->next_hop);
				}
				else if (pnode->domain_name != NULL){
					fprintf(fp, "%d [label=\"%s\"];\n", tmp_name, pnode->domain_name);
				}
				else{
					fprintf(fp, "%d [label=\"\"];\n", tmp_name);
				}
				fprintf(fp, "%d -> %d [label=\"/\"];\n", tmp_name, ++counter);
				bitmap_patricia_graphviz_dfs(pnode->pt_root, fp, counter);
			}
			pnode = pnode->next_node;
		}
	}
	return;
}

void create_bitmap_patricia_graphviz(char * filename, struct bitmap_patricia_node *node){
	FILE * fp;
	char cmd[100], out[50];

	counter = 0;

	strcpy(out, filename);
	out[strlen(filename) - 4] = '\0';

	fp = fopen(filename, "w");
	fprintf(fp, "digraph G{\n");
	bitmap_patricia_graphviz_dfs(node, fp, 0);
	fprintf(fp, "}");
	fclose(fp);
	sprintf(cmd, "dot %s -Tpng -o %s.png\n", filename, out);
	printf("%s", cmd);
	system(cmd);
	//sprintf(cmd, "del %s", filename);
	//system(cmd);
}

void create_component_patricia_graphviz(char * filename, struct component_patricia_node *node){
	FILE * fp;
	char cmd[100], out[50];

	counter = 0;

	strcpy(out, filename);
	out[strlen(filename) - 4] = '\0';

	fp = fopen(filename, "w");
	fprintf(fp, "digraph G{\n");
	component_patricia_graphviz_dfs(node, fp, 0, 0);
	fprintf(fp, "}");
	fclose(fp);
	sprintf(cmd, "dot %s -Tpng -o %s.png\n", filename, out);
	printf("%s", cmd);
	system(cmd);
	//sprintf(cmd, "del %s", filename);
	//system(cmd);
}

void create_component_byte_grapgviz(char *filename, struct component_byte_patricia_node **node){
	FILE * fp;
	char cmd[100], out[50];
	unsigned int i = 0;

	counter = 0;

	strcpy(out, filename);
	out[strlen(filename) - 4] = '\0';

	fp = fopen(filename, "w");
	fprintf(fp, "digraph G{\n");
	component_byte_patricia_graphviz_dfs(node, fp, 0);
	fprintf(fp, "}");
	fclose(fp);
	sprintf(cmd, "dot %s -Tpng -o %s.png\n", filename, out);
	printf("%s", cmd);
	system(cmd);
	for (i = 0; i < strlen(filename); i++){
		if (filename[i] == '/'){
			filename[i] = '\\';
		}
	}
	sprintf(cmd, "del %s", filename);
	system(cmd);
}