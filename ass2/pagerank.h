// Rotem Beker 217386598

#ifndef PAGERANK_H
#define PAGERANK_H

#include "graph.h"

/**
 * Computes the PageRank of all vertices in the graph using a parallel
 * thread-pool-based implementation.
 *
 * @param g        Pointer to the input graph.
 * @param n_iters  Number of iterations to run the PageRank algorithm.
 * @param rank     Output array of length g->numVertices. On return,
 *                 rank[i] holds the PageRank score of vertex i.
 *                 The caller is responsible for allocating this array.
 */
void PageRank(Graph *g, int n_iters, float *rank);

#endif /* PAGERANK_H */