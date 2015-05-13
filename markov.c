#include "hash.h"
#include "markov.h"

static NODE *pick_first_word(HASH_TABLE *);
static double get_rand();

static void quicksort(NODE **nodes, unsigned n) {
	unsigned i,
		 j,
		 p;
	NODE 	*t;

	if(n < 2)
		return;
	p = nodes[n/2]->first;
	for(i = 0, j = n - 1; ; i++, j--) {
		while(nodes[i]->first > p)
			i++;
		while(p > nodes[j]->first)
			j--;
		if(i >= j)
			break;
		t = nodes[i];
		nodes[i] = nodes[j];
		nodes[j] = t;
	}
	quicksort(nodes, i);
	quicksort(nodes + i, n - i);
}

// bad random number, but eh

static double get_rand() {
	time_t	t;
	srand((unsigned) time(&t));

	unsigned r = rand() % 1000;
	return (double)r/1000;
}
	
/* Function:	pick_first_word()
 * Description:	Pick the first word of the sentence to construct.  Do so by
 *		creating a list of all possible words to start the sentence,
 *		sort it, and then, based on cumulative distrubition function,
 *		pick a word.
 */

static NODE *pick_first_word(HASH_TABLE *ht) {
	NODE *node;
	unsigned i = 0,
		sentences = 0,
		choices = 0,
		sum = 0;
	double 	sum_dist = 0;

	sentences = get_sentences(ht);
	printf("num sentences: %u\n", sentences);

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
	quicksort(picks, choices);
	for(i = 0; i < choices; i++) {
		printf("%s:\n\tfreq: %u,\tprob: %.5lf\n", picks[i]->word, picks[i]->first, (double) picks[i]->first/sentences);
		sum += picks[i]->first;
		sum_dist += (double)picks[i]->first/sentences;
	}

	printf("sentences:\t%u\n", sentences);
	printf("occurences:\t%u\n", sum);
	printf("cumulative sum distr: %.3lf\n", get_rand());
	printf("sum dist: %lf\n", sum_dist);
	// sort list
	// then choose word based on distribution

	return node;
}
	
int main() {
	HASH_TABLE *ht;
	NODE 	*node = NULL;
	FILE	*fp;

	ht = create_table();
	// test parser
	fp = fopen("test2.txt", "r");
	insert_words(ht, fp);
	//print_all_nodes(ht);
	
	pick_first_word(ht);
	return 1;
}
