#include "markov.h"

static NODE *pick_first_word(HASH_TABLE *);
static double get_rand();
static void sort_by_first(NODE **, unsigned);

/* Function:	sort_by_first()
 * Description:	Quciksort array of nodes based on frequency of
 *		occurring first in a sentence
 */

static void sort_by_first(NODE **nodes, unsigned n) {
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
	sort_by_first(nodes, i);
	sort_by_first(nodes + i, n - i);
}

/* Function:	sort_by_freq()
 * Description:	Quciksort array of nodes based on frequency of
 *		occurrence.
 */

static void sort_by_freq(SUCC **nodes, unsigned n) {
	unsigned i, j, p;
	SUCC 	*t;

	if(n < 2)
		return;
	p = nodes[n/2]->freq;
	for(i = 0, j = n - 1; ; i++, j--) {
		while(nodes[i]->freq > p)
			i++;
		while(p > nodes[j]->freq)
			j--;
		if(i >= j)
			break;
		t = nodes[i];
		nodes[i] = nodes[j];
		nodes[j] = t;
	}
	sort_by_freq(nodes, i);
	sort_by_freq(nodes + i, n - i);
}

/* Function:	end_sentence() 
 * Description:	Given a node, determine if the sentence should end.
 */

// TODO: instead of bias, use average length of sentence
static unsigned end_sentence(NODE *node) {
	double end_prob = 0;
	static double bias = 0;
	double the_rand = 0;
		bias += 0.01;

	if(node->last) {
		//printf("chance of ending equal to \"of the total freq, how often is it ending a sentence\"\n");
		//printf("\tnode->last/node->freq = %lf\n", end_prob = (double)node->last/node->freq);
		end_prob = (double)node->last/node->freq;
		the_rand = get_rand();
		//printf("\nend prob: %lf\n", end_prob);
		//printf("the rand: %lf\n", the_rand);
		if(end_prob + bias > the_rand) {
			//printf("ending sentence\n");
			printf(".\n");
			return 1;
		}
	}
	if(node->num_succ == 0) {
		printf(".\n");
		return 1;
	}
	//printf("not ending sentence\n");
	return 0;
}
// bad random number, but eh

static double get_rand() {
	static time_t	t;
	srand((unsigned) time(&t));

	unsigned r = rand() % 1000;
	return (double)r/1000;
}
	
/* Function:	build_density()
 * Description:	Build density array given frequencies.  Each element's value 
 *		is the previous sum probabilities plus current
 *		element's probability. Cumulative distribution.
 */

static void build_density(double *density, unsigned n, unsigned total) {
	unsigned i;
	double sum = 0;
	double temp[n];

	for(i = 0; i < n; i++ ) {
		temp[i] = density[i];
	}

	for(i = 0; i < n; i++) {
		sum += (double) temp[i]/total;
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
	NODE 	*node;
	unsigned i = 0,
		sentences = 0,
		choices = 0;
	double 	sum_dist = 0,
		word_prob = get_rand();
	char	buf[64];

	sentences = get_sentences(ht);
	while((node = get_next_node(ht))) {
		if(node->first)
			choices++;
	}

	NODE 	*nodes[choices];
	double	density[choices];
	while((node = get_next_node(ht))) {
		if(node->first) {
			nodes[i++] = node;
		}
	}
	sort_by_first(nodes, choices);
	for(i = 0; i < choices; i++)
		density[i] = nodes[i]->first;
	
	build_density(density, choices, sentences);
	for(i = 0; i < choices; i++) {
		//printf("freq: %3u, prob: %.5lf, dist: %.5lf, word: %s\n", nodes[i]->first, (double) nodes[i]->first/sentences, density[i], nodes[i]->word);
		sum_dist += (double)nodes[i]->first/sentences;
	}

	for(i = 0; i < choices; i++) {
		if(word_prob < density[i]) {
			node = nodes[i];
			break;
		}
	}

	strcpy(buf, node->word);
	buf[0] = toupper(buf[0]);
	//printf("node chosen: %s\n", buf);
	//printf("sentences:\t%u\n", sentences);
	//printf("cumulative sum dist: %.3lf\n", word_prob);
	//printf("total sum dist: %lf\n", sum_dist);
	printf("%s", buf);

	node->traversed++;
	return node;
}
	
/* Functiin:	pick_next_word()
 * Description:	Given a node, build build array of succ nodes and pick
 *		the next node based on freq and cumulative distribution.
 */

static NODE *pick_next_word(NODE *node) {
	SUCC 	*succ = NULL;
	unsigned i = 0,
		size = 0,
		total = 0;
	double 	sum_dist = 0,
		word_prob = get_rand();

	size = node->num_succ;
	total = node->sum_succ;
	//printf("total succ words: %u\n", size);
	//printf("total freq words: %u\n", total);
	if(node->succ) {
		SUCC *succ_nodes[size];
		double density[size];

		succ = node->succ;
		while(succ) {
			succ_nodes[i++] = succ;
			sum_dist += (double)succ->freq/total;
			succ = succ->next;
		}
		sort_by_freq(succ_nodes, size);
		for(i = 0; i < size; i++) {
			density[i] = succ_nodes[i]->freq;
		}
		build_density(density, size, total);
		for(i = 0; i < size; i++) {
		//	printf("freq: %*u, distr: %.4lf, word: %s\n", 3, succ_nodes[i]->freq, density[i], succ_nodes[i]->node->word);
		}
		for(i = 0; i < size; i++) {
			if(word_prob < density[i] && !succ_nodes[i]->node->traversed) {
				node = succ_nodes[i]->node;
				break;
			}
		}
		if(i == size) {
			printf("premature END\n");
			exit(1);
		}
		//printf("next word: %s\n", node->word);
		//printf("total sum dist: %lf\n", sum_dist);
		printf(" %s", node->word);
	}
	else {
		printf("premature END\n");
		exit(1);
	}
	node->traversed++;
	return node;
}

/* Function:	build_sentence() 
* Description:	Main loop for building a sentence.
 */

void build_sentence(HASH_TABLE *ht) {
	NODE *node = NULL;
	assert(ht);
	
	node = pick_first_word(ht);
	while(!end_sentence(node)) {
		node = pick_next_word(node);
	}
}

int main() {
	HASH_TABLE *ht;
	FILE	*fp;

	ht = create_table();
	// test parser
	fp = fopen("test3.txt", "r");
	insert_words(ht, fp);
	//print_all_nodes(ht);
	
	build_sentence(ht);
	return 1;
}
