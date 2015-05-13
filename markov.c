#include "hash.h"
#include "markov.h"

static NODE *pick_first_word(HASH_TABLE *);
static double get_rand();
static void quicksort(NODE **, unsigned);

static void quicksort(NODE **nodes, unsigned n) {
	unsigned i, j, p;
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
	
/* Function:	build_density()
 * Description:	Build density array.  Each element's value is the previous sum 
 *		probabilities plus current element's probability. Cumulative
 *		distribution.
 */

static void build_density(NODE **nodes, double *density, unsigned n, unsigned total) {
	unsigned i;
	double sum = 0;

	for(i = 0; i < n; i++) {
		sum += (double) nodes[i]->first/total;
		density[i] = sum;
	}
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
	double 	sum_dist = 0,
		prob = 0;

	sentences = get_sentences(ht);
	printf("num sentences: %u\n", sentences);

	while((node = get_next_node(ht))) {
		if(node->first)
			choices++;
	}

	NODE 	*picks[choices];
	double	density[choices];
	while((node = get_next_node(ht))) {
		if(node->first)
			picks[i++] = node;
	}
	
	printf("all choices for first word of sentence:\n");
	quicksort(picks, choices);
	build_density(picks, density, choices, sentences);
	for(i = 0; i < choices; i++) {
		printf("freq: %3u, prob: %.5lf, dist: %.5lf, word: %s\n", picks[i]->first, (double) picks[i]->first/sentences, density[i], picks[i]->word);
		sum += picks[i]->first;
		sum_dist += (double)picks[i]->first/sentences;
	}

	printf("sentences:\t%u\n", sentences);
	printf("occurences:\t%u\n", sum);
	printf("cumulative sum dist: %.3lf\n", prob = get_rand());
	printf("total sum dist: %lf\n", sum_dist);

	for(i = 0; i < choices; i++) {
		if(prob < density[i]) {
			node = picks[i];
			break;
		}
	}
	printf("node: chosen: %s, chances: %.5lf\n", node->word, density[i]);
	return node;
}
	
int main() {
	HASH_TABLE *ht;
	NODE 	*node = NULL;
	FILE	*fp;

	ht = create_table();
	// test parser
	fp = fopen("test3.txt", "r");
	insert_words(ht, fp);
	//print_all_nodes(ht);
	
	pick_first_word(ht);
	return 1;
}
