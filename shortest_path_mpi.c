#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <mpi.h>

/* The unsigned integer type used for node ID numbers
 * and number of nodes */
typedef unsigned int node_id_t;

/* The unsigned integer type used for edge ID numbers
 * and number of edges for edges emanating from a single node. */
typedef unsigned int edge_id_t;

/* The unsigned integer type used for storing the total
 * number of edges in the graph, also for the file
 * line number.  It should be a big unsigned int. */
typedef unsigned long int bigint_t;

/* The maximum length of a word in the graph input file */
#define MAXL 100

/* An integer bigger than any possible distance. */
#define INFINITY ULONG_MAX

/* An integer bigger than any possible node ID. */
#define UNDEFINED UINT_MAX

/* The maximum number of nodes we can handle */
#define NODE_MAX (UINT_MAX - 1)

/* The maximum number of edges we can handle */
#define EDGE_MAX (ULONG_MAX - 1)

/* The function to convert a string to a bigint_t */
bigint_t string_to_bigint(const char *str) {
	return strtoul(str, NULL, 10);
}



// ************************  MPI  *************************
int rank;
int nprocs;

// ************************  Edge  ************************

/* An edge in the graph */
typedef struct edge_s {
	node_id_t dest; // the ID of the destination node, 0-indexed
	int weight; // the weight of this edge
} * Edge;

/* Creates an edge.  dest is the destination node ID,
 * which should be 0-indexed */
Edge edge_create(node_id_t dest, int weight) {
	assert(weight>0);

	Edge result = malloc(sizeof(struct edge_s));

	assert(result);
	result->dest = dest;
	result->weight = weight;
	return result;
}

/* Destroys an edge that was created by edge_create */
void edge_destroy(Edge edge) {
	free(edge);
}

/* Prints an edge.  Our nodes are 0-indexed, but in the file
 * format, they are 1-indexed, so we add 1 before printing
 * to be consistent with the file format. */
void edge_print(Edge edge) {
	printf("Edge[dest=%d  weight=%d]", 1+edge->dest, edge->weight);
}

/* A comparison function for edges emanating from a single
 * node.   Used to sort the edges by increasing destination ID.
 * This function consumes two pointer-to-Edge arguments. */
int edge_compare(const void *p1, const void *p2) {
	Edge edge1 = *((Edge*)p1), edge2 = *((Edge*)p2);
	node_id_t n1 = edge1->dest, n2 = edge2->dest;

	return n1<n2 ? -1 : n1==n2 ? 0 : 1;
}

// ************************  Node  ************************

/* A node in the graph with fields that are used by the Dijkstra
 * shortest path algorithm */
typedef struct node_s {
	node_id_t id; // 0-indexed ID number
	edge_id_t num_edges; // number of edges emanating from this node
	Edge * edges; // the edges emanating from this node
	bigint_t dist; // the distance field used in Dijkstra algorithm
	node_id_t prev; // previous node in shortest path
	bool visited; // has this node been visited in the Dijkstra alg.?
} * Node;

/* Creates a new node.   ID number is 0-indexed. */
Node node_create(node_id_t id) {
	Node result = malloc(sizeof(struct node_s));

	assert(result);
	result->id = id;
	result->num_edges = 0;
	result->edges = NULL;
	result->dist = INFINITY;
	result->prev = UNDEFINED;
	result->visited = false;
	return result;
}

/* Destroys the node.  Only frees the data allocated in node_create.
 * Does not destroy the Edges associated to this node. */
void node_destroy(Node node) {
	assert(node);
	free(node);
}

/* Prints a node.  We add 1 to the ID number to be consistent
 * with the file format. */
void node_print(Node node) {
	printf("Node %d:\n", node->id+1);
	printf("  num_edges = %u\n", node->num_edges);
	printf("  dist = %lu\n", node->dist);
	printf("  visited = %d\n", node->visited);
	printf("  edges:\n");
	for (edge_id_t j=0; j<node->num_edges; j++) {
		printf("    ");
		edge_print(node->edges[j]);
		printf("\n");
	}
}

// ************************  Global vars  ************************

/* The number of nodes in the graph */
node_id_t total_num_nodes;

/* The number of edges in the graph.  This is the number of
 * directed edges, so for an undirected graph, divide this by
 * 2 to get the number of undirected edges. */
bigint_t total_num_edges;

/* The current line number; used when parsing the input file */
bigint_t lineno;

/* The nodes in the graph.  A sequence of length total_num_nodes. */
Node *the_nodes;

// ************** Utility functions on global vars **************


/* Prints the graph, including the state data for the
 * Dijkstra algorithm */
void print_graph() {
	for (node_id_t i=0; i<total_num_nodes; i++)
		node_print(the_nodes[i]);
}

/* Quit due to some error */
void bail(char * msg) {
	fprintf(stderr, "Error reading line %lu: %s\n", lineno, msg);
	exit(1);
}

// ************************  Parsing  ************************

/* Moves to the next line by reading until newline character.
 * Increments lineno (unless no \n is encountered).
 * Returns false if EOF is encountered first, else true */
bool next_line(FILE * file) {
	int c;
	do {
		c = fgetc(file);
		if (c == EOF)
			return false;
	} while (c != (unsigned char)'\n');
	lineno++;
	return true;
}

/* Parses file through the problem description line, which
 * specifies number of nodes and number of edges.
 * Returns those numbers in the given out parameters.
 * An out parameter can also be NULL.
 * Upon return, the file position will be at the beginning
 * of the line immediately after the problem description line.
 */
void read_header(FILE *file, node_id_t *num_nodes, bigint_t *num_edges) {
	char word[MAXL], num_nodes_str[MAXL], num_edges_str[MAXL];
	int c;

	assert(file);
	for (c=fgetc(file); c!=(unsigned char)'p'; c=fgetc(file)) {
		if (c==EOF) {
			bail("File ended with no line beginning with 'p'");
		} else if (c==(unsigned char)'c') {
			if (!next_line(file))
				bail("File ended with no line beginning with 'p'");
		} else if (c==(unsigned char)'\n') {
			lineno++;  // a blank line is OK, just ignore it
		} else
			bail("Expected line beginning with p or c");
	}

	int nread = fscanf(file, "%s", word);

	if (nread != 1 || strcmp(word, "sp")!=0) bail("Expected 'sp'");
	nread = fscanf(file, "%s%s", num_nodes_str, num_edges_str);
	if (nread != 2)
		bail("Expected 2 unsigned integers after 'sp'");

	bigint_t num_nodes_big = string_to_bigint(num_nodes_str);

	if (num_nodes_big == 0 || num_nodes_big > NODE_MAX)
		bail("Illegal value for num_nodes");

	bigint_t num_edges_big = string_to_bigint(num_edges_str);

	if (num_edges_big > EDGE_MAX) bail("Illegal value for num_edges");
	if (num_nodes != NULL) *num_nodes = (node_id_t)num_nodes_big;
	if (num_edges != NULL) *num_edges = num_edges_big;
	next_line(file);
}

/* Reads the next edge.  Will pass through comment and blank lines.
 * Precondition: file position is at beginning of some line.
 * Postcondition: file position will be at beginning of line just
 * after the first encountered edge line.
 * The node IDs returned are 0-indexed.  */
bool read_edge(FILE *file, node_id_t *source, node_id_t *dest,
		int *weight) {
	int c;

	while (true) {
		c = fgetc(file);
		if (c == EOF) {
			return false;
		} else if (c == (unsigned char)'c') {
			next_line(file);
		} else if (c == (unsigned char)'\n') {
			lineno++;
		} else
			break;
	}
	if (c != (unsigned char)'a') bail("Unknown line type");

	// this assumes node_id_t is unsigned int:
	int nread = fscanf(file, "%u%u%d", source, dest, weight);

	if (nread != 3)
		bail("Expected 3 ints following 'a'");
	// note: in the file, nodes are indexed from 1.  We want
	// them indexed from 0...
	assert(*source >= 1 && *source <= total_num_nodes);
	assert(*dest >= 1 && *dest <= total_num_nodes);
	assert(*weight > 0);
	(*source)--;
	(*dest)--;
	if (!next_line(file))
		bail("File ended without a newline character");
	return true;
}

/* First parsing pass: get the number of edges for each node
 * and allocate the_nodes array. */
void parse1(char * filename) {
	FILE * file = fopen(filename, "r");
	bigint_t edge_count = 0;
	node_id_t source, dest;
	int weight;

	lineno=1;
	read_header(file, &total_num_nodes, &total_num_edges);
	printf("Parsing graph with %u nodes, %lu edges.\n",
			total_num_nodes, total_num_edges);
	the_nodes = malloc(total_num_nodes*sizeof(struct node_s));
	assert(the_nodes);
	for (node_id_t i=0; i<total_num_nodes; i++)
		the_nodes[i] = node_create(i);
	while (read_edge(file, &source, &dest, &weight)) {
		the_nodes[source]->num_edges++;
		edge_count++;
	}
	if (edge_count != total_num_edges) {
		fprintf(stderr, "Expected %lu edges, but saw %lu\n",
				total_num_edges, edge_count);
		bail("Bailing...");
	}
	for (node_id_t i=0; i<total_num_nodes; i++) {
		Node node = the_nodes[i];

		node->edges = malloc(node->num_edges * sizeof(Edge));
		assert(node->edges);
	}
	fclose(file);
}

/* Second parsing pass: populate the edges fields */
void parse2(char * filename) {
	int edge_counts[total_num_nodes];
	FILE * file = fopen(filename, "r");
	node_id_t source, dest;
	int weight;

	for (node_id_t i=0; i<total_num_nodes; i++) edge_counts[i] = 0;
	lineno=1;
	read_header(file, NULL, NULL);
	while (read_edge(file, &source, &dest, &weight)) {
		Node node = the_nodes[source];
		edge_id_t edge_id = edge_counts[source];

		node->edges[edge_id] = edge_create(dest, weight);
		edge_counts[source] = edge_id+1;
	}
	fclose(file);
	for (node_id_t i=0; i<total_num_nodes; i++)
		assert(edge_counts[i] == the_nodes[i]->num_edges);
}

/* Sorts the edges for each node in the graph.
 * Precondition: build_graph has been called.
 * For all i,j, there is at most one edge from node i to node j.
 * Postcondition: the sequence of edges emanating from
 * a node will be ordered by increasing destination ID.
 *
 * Note: the syntax for stdlib's quick-sort function is:
 *  void qsort(void *base, size_t nel, size_t width,
 *             int (*compar)(const void *, const void *));
 */
void sort_edges() {
	int width = sizeof(Edge);

	assert(the_nodes);
	for (int i=0; i<total_num_nodes; i++) {
		Edge *edges = the_nodes[i]->edges;
		int n_edges = the_nodes[i]->num_edges;

		qsort(edges, n_edges, width, edge_compare);
	}
}

/* Parses input file and builds the graph */
void parse(char *filename) {
	parse1(filename);
	parse2(filename);
	sort_edges();  
}


// ******************** Dijkstra's algorithm ********************

/* Prints the shortest path.
 * Precondition: the shortest path has been found, so the prev
 * fields are defined for the path starting from last following
 * prev fields, until the source node is reached.
 */
void print_path(Node last) {
	bigint_t len = 0; // number of edges in path

	for (node_id_t i=last->prev; i!=UNDEFINED; i=the_nodes[i]->prev)
		len++;

	Edge edges[len];
	node_id_t currID = last->id;
	node_id_t prevID = last->prev;
	bigint_t count = 0;

	while (prevID!=UNDEFINED) {
		Node prev = the_nodes[prevID];
		edge_id_t num_edges = prev->num_edges, i=0;

		while (i<num_edges && prev->edges[i]->dest!=currID)
			i++;
		assert(i<num_edges);
		edges[len-count-1] = prev->edges[i];
		count++;
		currID = prevID;
		prevID = prev->prev;
	}
	assert(count==len);
	for (bigint_t i=0; i<len; i++) {
		Edge edge = edges[i];

		printf("%6lu. %u -> %u   [%u]\n", i+1, currID+1,
				edge->dest+1, edge->weight);
		currID = edge->dest;
	}
}

/* Find a shortest path from start to stop.  Node IDs are 0-indexed.
 * Prints the path (if one exists) and the total distance. */
void find_shortest_path(node_id_t start, node_id_t stop) {
	the_nodes[start]->dist = 0;
	while (true) {
		// find unvisited node with minimal dist...
		Node u = NULL;
		unsigned int *u_indices;
		unsigned int u_index_local = NULL;
		bigint_t u_dist = INFINITY;
		bigint_t u_dist_local = INFINITY;
		//processors finds the node with minimal distance
		for (node_id_t j=rank; j<total_num_nodes; j+=nprocs) {
			Node v = the_nodes[j];

			if (!v->visited) {


				if (v->dist < u_dist_local) {
					u_index_local = j;
					u_dist_local = v->dist;
				}
			}
		}
		//gather all the processors minimums
		MPI_Allgather(&u_index_local, 1, MPI_UNSIGNED, u_indices,
				1, MPI_UNSIGNED, MPI_COMM_WORLD);
	
		//Find the minimum of all the processors
		for (int k =0; k<nprocs; k++){
			unsigned int j = u_indices[k];
			if (j<total_num_nodes){
				Node v = the_nodes[j];
				if (!v->visited)
				if (v->dist < u_dist) {
					u = v;
					u_dist = v->dist;
				}
			}
		}

		if (u_dist == INFINITY) {
			printf("There is no path.\n");
			return;
		}
		fflush(stdout);
		if (u->id==stop) {
			if (rank==0){
				print_path(u);
				printf("The shortest path has length %lu.\n", u_dist);
			}
			return;
		}
		u->visited = true;

		int num_edges = u->num_edges;
		Edge *edges = u->edges;
		//not parrallelized because each node has only 4 edges at most. 
		//Not worth the overhead
		for (edge_id_t j=0; j<num_edges; j++) {
			Edge e = edges[j];
			Node v = the_nodes[e->dest];

			if (!v->visited) {
				bigint_t alt = u_dist + e->weight;

				if (alt<v->dist) {
					v->dist = alt;
					v->prev = u->id;
				}
			}
		}
	}
}

// ************************  Main function  ************************

/* Main function: finds the shortest path between two nodes in a graph.
 * Arg1: filename for the graph file
 * Arg2: node number for source node (1-indexed)
 * Arg3: node number for target node (1-indexed)
 * The result is printed to stdout.
 */
int main(int argc, char * argv[]) {
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);


	double time0 = MPI_Wtime();

	if (argc != 4)
		bail("Expected 3 arguments: filename, source, and target");
	parse(argv[1]);

	double time1 = MPI_Wtime();

	if (rank==0)
		printf("Parsing complete (time = %lf seconds).\n",
				time1-time0);
	fflush(stdout);
#ifdef DEBUG
	printf("\nThe graph:\n\n");
	print_graph();
#endif

	bigint_t source_big = string_to_bigint(argv[2]);

	if (source_big<1 || source_big>total_num_nodes) {
		if (rank ==0)
			fprintf(stderr, "Illegal node: %lu\n", source_big);
		exit(1);
	}

	bigint_t target_big = string_to_bigint(argv[3]);

	if (target_big<1 || target_big>total_num_nodes) {
		if (rank ==0)
			fprintf(stderr, "Illegal node: %lu\n", target_big);
		exit(1);
	}
	find_shortest_path((node_id_t)source_big - 1,
			(node_id_t)target_big - 1);

	double time2 = MPI_Wtime();

	if (rank ==0)
		printf("Total time: %lf seconds.\n", time2-time0);
	MPI_Finalize();
}
