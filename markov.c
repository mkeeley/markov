#include "hash.h"

static NODE *pick_first_word(HASH_TABLE *ht) {
	NODE *node;
	unsigned choices = 0;
	unsigned sentences = 0;
	unsigned i = 0;

	printf("num sentences: %u\n", sentences = get_sentences(ht));

	while((node = get_next_node(ht))) {
		if(node->first)
			choices++;
	}

	NODE *picks[choices];
	while((node = get_next_node(ht))) {
		if(node->first)
			picks[i++] = node;
	}
	
	printf("all choices for first word of sentence:\n");
	for(i = 0; i < choices; i++) {
		printf("\t%s freq:\t%u, prob:\t%.3f\n", picks[i]->word, picks[i]->first, (float) picks[i]->first/sentences);
	}
	return node;
}
	
int main() {
	HASH_TABLE *ht;
	NODE 	*node = NULL;
	FILE	*fp;

	ht = create_table();
	// test parser
	fp = fopen("test.txt", "r");
	insert_words(ht, fp);
	print_all_nodes(ht);
	
	while((node = get_next_node(ht))) {
		printf("node->word: %s\n", node->word);
	}
	
	pick_first_word(ht);
	return 1;
}
