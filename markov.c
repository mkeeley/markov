#include "hash.h"

int main() {
	HASH_TABLE *ht;
	NODE 	*node = NULL;
	FILE	*fp;

	ht = create_table();
	node = NULL;
	// test parser
	fp = fopen("test.txt", "r");
	insert_words(ht, fp);
	print_all_nodes(ht);
	
	while((node = get_next_node(ht))) {
		printf("node->word: %s\n", node->word);
	}
	return 1;
}
