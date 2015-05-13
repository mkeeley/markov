#include "hash.h"
#include "markov.h"

// bad random number, but eh

static double get_rand() {
	time_t	t;
	srand((unsigned) time(&t));

	unsigned r = rand() % 1000;
	return (double)r/1000;
}
	
static NODE *pick_first_word(HASH_TABLE *ht) {
	NODE *node;
	unsigned i = 0,
		sentences = 0,
		choices = 0;

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
	printf("cumulative sum distr: %.3lf\n", get_rand());
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
