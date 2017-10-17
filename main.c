#include <time.h>
#include <assert.h>

#include "bitmap_patricia.h"
#include "component_byte_patricia.h"
#include "component_patricia.h"

#include "patricia_statistic.h"
#include "print_graph.h"


#define LOOKUP_TIMES 5

void bitmap_patricia_test(char *input_file_path)
{
	FILE *input_file;
	char input_buffer[BUFSIZE];
	struct bitmap_patricia_node trie;
	int ret = 0;
	double start,finish;
	int i;
	int port_count = 0;
	struct statistic_info info;

	input_file = fopen(input_file_path, "r");
	if (input_file == NULL) {
		printf("File not exist.\n");
		assert(input_file != NULL);
	}

	printf ("This is bitmap_patricia Test Case!\n\n");

	memset(&trie, 0, sizeof(trie));
	bitmap_patricia_initial(&trie);

	while (fgets(input_buffer, BUFSIZE, input_file)) {
		// delete the '\n' in strLine
		if (input_buffer[strlen(input_buffer) - 1] == '\n')
			input_buffer[strlen(input_buffer) - 1] = 0;

		port_count++;
		bitmap_patricia_insert(&trie, input_buffer, 0, port_count);
	}


	//////////////////////////////////////////////////////////////////////////

	fseek(input_file, 0, SEEK_SET);
	start = clock();
	for (i = 0; i != LOOKUP_TIMES; ++i) {

		while (fgets(input_buffer, BUFSIZE, input_file)) {
			// delete the '\n' in strLine
			if (input_buffer[strlen(input_buffer) - 1] == '\n')
				input_buffer[strlen(input_buffer) - 1] = 0;

			ret = bitmap_patricia_lookup(&trie, input_buffer, 0);
			assert(ret > 0);
		}
	}
	finish = clock();
	printf ("time use %f\n", (finish-start)/LOOKUP_TIMES);


	//////////////////////////////////////////////////////////////////////////

	fclose(input_file);

	info = bitmap_patricia_statistic(&trie, 0);

	printf("Total Memory: %d bytes\n", info.mem_count);
	printf("Max Depth:  %d\n", info.max_depth);
	printf("Avg Depth:    %lf\n", info.avg_depth);
	printf("Leaf Number:  %d (insert: %d)\n", info.leaf_count, port_count);
	printf("Node Number:  %d\n", info.node_count);

	/*char filename[50] = "./output/bitmap_patricia.txt";
	create_bitmap_patricia_graphviz(filename, &trie);*/

	return;
}

void component_patricia_test(char *input_file_path)
{
	FILE *input_file;
	char input_buffer[BUFSIZE];
	struct component_patricia_node *trie;
	int ret = 0;
	double start, finish;
	int i;
	int port_count = 0;

	struct statistic_info info;

	input_file = fopen(input_file_path, "r");
	if (input_file == NULL) {
		printf("File not exist.\n");
		assert(input_file != NULL);
	}

	printf("This is component_patricia Test Case!\n\n");

	trie = (struct component_patricia_node *)calloc(1, sizeof (struct component_patricia_node));

	while (fgets(input_buffer, BUFSIZE, input_file)) {
		// delete the '\n' in strLine
		if (input_buffer[strlen(input_buffer) - 1] == '\n')
			input_buffer[strlen(input_buffer) - 1] = 0;

		port_count++;
		component_patricia_insert(trie, input_buffer, port_count);
	}

	//////////////////////////////////////////////////////////////////////////

	fseek(input_file, 0, SEEK_SET);
	start = clock();
	for (i = 0; i != LOOKUP_TIMES; ++i) {

		while (fgets(input_buffer, BUFSIZE, input_file)) {
			// delete the '\n' in strLine
			if (input_buffer[strlen(input_buffer) - 1] == '\n')
				input_buffer[strlen(input_buffer) - 1] = 0;

			ret = component_patricia_lookup(trie, input_buffer, strlen(input_buffer));
			assert(ret > 0);
		}
	}
	finish = clock();
	printf("time use %f\n", (finish - start) / LOOKUP_TIMES);

	//////////////////////////////////////////////////////////////////////////

	fclose(input_file);

	// 	output_file = fopen(statistic_file_path, "w");
	// 	if (output_file == NULL) {
	// 		printf("File not exist.\n");
	// 		assert(output_file != NULL);
	// 	}

	info = component_patricia_statistic(trie);

	printf("Total Memory: %d bytes\n", info.mem_count);
	printf("Max Depth:  %d\n", info.max_depth);
	printf("Avg Depth:    %lf\n", info.avg_depth);
	printf("Leaf Number:  %d (insert: %d)\n", info.leaf_count, port_count);
	printf("Node Number:  %d\n", info.node_count);

	/*char filename[50] = "./output/component_patricia.txt";
	create_component_patricia_graphviz(filename, trie);*/
	return;
}

void component_byte_patricia_test(char *input_file_path) {
	FILE *input_file;
	char input_buffer[BUFSIZE];
	struct component_byte_patricia_node **trie;
	int ret = 0;
	double start,finish;
	int i;
	int port_count = 0;
	struct statistic_info info;

	input_file = fopen(input_file_path, "r");
	if (input_file == NULL) {
		printf("File not exist.\n");
		assert(input_file != NULL);
	}

	printf ("This is component_byte_patricia Test Case!\n\n");

	trie = (struct component_byte_patricia_node **)calloc(
		1, sizeof (struct component_byte_patricia_node*) * BUCKET_LEN);

	while (fgets(input_buffer, BUFSIZE, input_file)) {
		// delete the '\n' in strLine
		if (input_buffer[strlen(input_buffer) - 1] == '\n')
			input_buffer[strlen(input_buffer) - 1] = 0;

		port_count++;
		component_byte_patricia_insert(trie, input_buffer, port_count);
	}


	//////////////////////////////////////////////////////////////////////////

	fseek(input_file, 0, SEEK_SET);
	start = clock();
	for (i = 0; i != LOOKUP_TIMES; ++i) {

		while (fgets(input_buffer, BUFSIZE, input_file)) {
			// delete the '\n' in strLine
			if (input_buffer[strlen(input_buffer) - 1] == '\n')
				input_buffer[strlen(input_buffer) - 1] = 0;

			ret = component_byte_patricia_lookup(trie, input_buffer);
			assert(ret > 0);
		}
	}
	finish = clock();
	printf ("time use %f\n", (finish-start)/LOOKUP_TIMES);

	//////////////////////////////////////////////////////////////////////////

	fclose(input_file);

	info = component_byte_patricia_statistic(trie);

	printf("Total Memory: %d bytes\n", info.mem_count);
	printf("Max Depth:  %d\n", info.max_depth);
	printf("Avg Depth:    %lf\n", info.avg_depth);
	printf("Leaf Number:  %d (insert: %d)\n", info.leaf_count, port_count);
	printf("Node Number:  %d\n", info.node_count);

	char filename[50] = "./output/component_byte_patricia.txt";
	create_component_byte_grapgviz(filename, trie);
	return;

}

int readfile(FILE * fp, char** rule_set){
	char input_buffer[BUFSIZE];
	int i = 0;
	while (fgets(input_buffer, BUFSIZE, fp)) {
		// delete the '\n' in strLine
		while (input_buffer[strlen(input_buffer) - 1] == '\n' || input_buffer[strlen(input_buffer) - 1] == '\r'){
			input_buffer[strlen(input_buffer) - 1] = 0;
		}
		rule_set[i] = (char*)malloc(sizeof(char)* (strlen(input_buffer) + 1));
		strcpy(rule_set[i], input_buffer);
		i++;
	}
	return i;
}

/* 
* interest的格式 I:baidu.com/news/school
* data的格式	 D:baidu.com/news/school/v1
*/
char* parseTrace(char * str){
	char * name = NULL;
	int len = strlen(str);
	 
	if (len <= 2){
		return NULL;
	}
	name = (char*)malloc(sizeof(char)*(len-1));
	strcpy(name, str + 2);
	return name;
}

void printInterestRes(struct InterestResult* interestRes, FILE* fp){
	struct CSdataList* p = NULL;
	int count = 0;
	if (interestRes){
		if (interestRes->datalist){
			p = interestRes->datalist;
			fprintf(fp,"Get data:");
			while (p){
				fprintf(fp, " %s", p->data);
				p = p->next;
				count++;
			}
			fprintf(fp, "  total %d data\n", count);
			free_CSdataList(interestRes->datalist);
			interestRes->datalist = NULL;
		}
		else if (interestRes->hop != -1){
			fprintf(fp, "Next-hop is %d\n", interestRes->hop);
		}
	}
	else{
		fprintf(fp, "Drop.\n");
	}
	return;
}

void printPortlist(struct PortList* portlist, FILE* fp){
	struct PortList* p = portlist,*tmp = NULL;
	int count = 0;
	if (!p){
		fprintf(fp, "Drop the Data.\n");
		return;
	}
	fprintf(fp, "Incomming Port:");
	while (p){
		tmp = p;
		fprintf(fp, " %d", p->port);
		p = p->next;
		count++;
		free(tmp);
		tmp = NULL;
	}
	fprintf(fp, "\n");
	return;
}

void printInputInfo(){
	struct component_byte_patricia_node **trie;
	int select_flag = 1, len = 0, i = 0;
	uint32_t rules_num = 0;

	char file_name[100];
	char out_fname[100] = "./output/res_lookup.txt";
	char infoFile[150];
	char tmp_fname[100];
	FILE * infofp;
	FILE * pfile;
	FILE * out_fp;
	char ** rules = NULL;

	char* input_buffer;
	double start, finish;

	int port_count = 0;
	int hop_count = 0;
	int cs_count = 0;
	char* pdata = NULL;
	char * name = NULL;
	struct InterestResult* interestRes = NULL;
	struct PortList* portlist = NULL;
	struct statistic_info info;

	out_fp = fopen(out_fname, "w");
	init_bit_mask();
	//build the initial trie
	trie = (struct component_byte_patricia_node **)calloc(
		1, sizeof (struct component_byte_patricia_node*) * BUCKET_LEN);

	printf("Input the scale of rules.\n");
	scanf("%d", &rules_num);
	rules = (char**)calloc(rules_num, sizeof(char*));

	while (1){
		if (select_flag == 1){
			printf("Please Input FIB Filename:\n");
		}
		else if (select_flag == 2){
			printf("Please Input PIT Filename:\n");
		}
		else if (select_flag == 3){
			printf("Please Input CS Filename:\n");
		}
		else if (select_flag == 4){
			printf("Please Input Delete PIT Filename:\n");
		}
		else if (select_flag == 5){
			printf("Please Input Delete CS Filename:\n");
		}
		else if (select_flag == 6){
			printf("Please Input the Trace Filename:\n");
		}
		else if (select_flag == -1){
			break;
		}
		else{
			printf("\nPlease select the following number:\n[ 1:] input FIB filename.    [ 2:] input PIT filename.\n[ 3:] input CS filename.     [-1:] return.\n");
			printf("[ 4:] input delete pit filename.    [ 5:] input delete cs filename.\n[6] input the trace file.\n");
			scanf("%d", &select_flag);
			if (select_flag > 10){
				break;
			}
			continue;
		}

		scanf("%s", file_name);
		pfile = fopen(file_name, "r");
		if (pfile == NULL) {
			printf("File not exist.\n");
			assert(pfile != NULL);
		}

		memset(rules, 0, sizeof(char*)* rules_num);
		len = readfile(pfile, rules);
		fclose(pfile);
		i = 0;

		printf("Begin building trie.\n");
		start = clock();
		while (i < len) {
			input_buffer = rules[i++];

			switch (select_flag)
			{
			case 1:
				hop_count++;
				component_byte_patricia_insert_fib(trie, input_buffer, hop_count);
				break;
			case 2:
				port_count++;
				component_byte_patricia_insert_pit(trie, input_buffer, port_count);
				break;
			case 3:
				pdata = (char*)malloc(sizeof(char)* 15);
				sprintf(pdata, "d%d", ++cs_count);
				component_byte_patricia_insert_cs(trie, input_buffer, pdata);
				break;
			case 4:
				del_component_byte_patricia_pit(trie, input_buffer);
				break;
			case 5:
				del_component_byte_patricia_cs(trie, input_buffer);
				break;
			case 6:
				name = parseTrace(input_buffer);
				if (name){
					if (input_buffer[0] == 'I'){
						port_count++;
						interestRes = deal_Interest_in_cb_patricia(trie, name, port_count);
						printInterestRes(interestRes, out_fp);
					}
					else if (input_buffer[0] == 'D'){
						pdata = (char*)malloc(sizeof(char)* 15);
						sprintf(pdata, "d%d", ++cs_count);
						portlist = deal_Data_in_cb_patricia(trie, name, pdata);
						printPortlist(portlist, out_fp);
					}
				}
				break;
			default:
				break;
			}
		}
		finish = clock();
		printf("Finish building!\nUse time %f s\n", (finish - start) / CLOCKS_PER_SEC);//包含读取文件的时间

		strcpy(tmp_fname, file_name);
		tmp_fname[strlen(file_name) - 4] = '\0';
		sprintf(infoFile, "%s_info_out%d.txt", tmp_fname, select_flag);
		infofp = fopen(infoFile, "w");
		info = component_byte_patricia_statistic(trie);

		fprintf(infofp, "Total Memory: %d bytes\n", info.mem_count);
		fprintf(infofp, "Max Depth:  %d\n", info.max_depth);
		fprintf(infofp, "Avg Depth:    %lf\n", info.avg_depth);
		fprintf(infofp, "Leaf Number:  %d (insert: %d)\n", info.leaf_count, port_count);
		fprintf(infofp, "Node Number:  %d\n", info.node_count);
		fclose(infofp);

		/*char filename[50];
		sprintf(filename, "./output/test1_pic_%d.txt", select_flag);
		create_component_byte_grapgviz(filename, trie);*/

		select_flag = 0;
	}
	for (i = 0; i < len; i++){
		free(rules[i]);
	}
	free(rules);
	fclose(out_fp);
	return;
}

int main()
{

	// parameter define
	/*char input_file_path[] = "input.txt";
	component_byte_patricia_test(input_file_path);*/

	printInputInfo();

	printf("end!\n");
	return 0;
}
