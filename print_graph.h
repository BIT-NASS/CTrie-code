#ifndef __PRINT_GRAPH__
#define __PRINT_GRAPH__

#include "bitmap_patricia.h"
#include "component_patricia.h"
#include "component_byte_patricia.h"
#include "patricia_statistic.h"

static int counter = 0;

void create_bitmap_patricia_graphviz(char * filename, struct bitmap_patricia_node *node);
void create_component_patricia_graphviz(char * filename, struct component_patricia_node *node);
void create_component_byte_grapgviz(char *filename, struct component_byte_patricia_node **node);

#endif

