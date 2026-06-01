// Ophir Finchelstein 216639542
// Yoav Haze 329409965

#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "thr_pool.h"

#define DAMPING_FACTOR 0.15

// Structure to pass arguments to worker threads
typedef struct {
    int start_node;      
    int end_node;        
    int num_vertices;    
    float *current_ranks; // Array of ranks from the previous iteration
    float *new_ranks;    // Array to write the new calculated ranks
    int *out_degree;     // Array containing out-degree of each node
    node **inverse_adj;  // Inverse graph: inverse_adj[v] contains list of nodes pointing to v
    float dangling_sum;  // Sum of ranks of nodes with no out-links
} ThreadArgs;

// Worker function to calculate PageRank for a specific range of nodes
void compute_pagerank_range(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int N = args->num_vertices;
    float d = DAMPING_FACTOR;

    // Iterate over the assigned range of vertices
    for (int i = args->start_node; i < args->end_node; i++) {
        double incoming_sum = 0.0;
        
        // Traverse the inverse graph to find nodes pointing to 'i'
        node *current = args->inverse_adj[i];
        while (current != NULL) {
            int u = current->v; // Node u points to i
            // Add contribution: Rank(u) / OutDegree(u)
            if (args->out_degree[u] > 0) {
                incoming_sum += args->current_ranks[u] / (double)args->out_degree[u];
            }
            current = current->next;
        }

        // Calculate the contribution from dangling nodes (distributed evenly)
        double dangling_contribution = args->dangling_sum / (double)N;
        
        // Apply the PageRank formula
        args->new_ranks[i] = (d / N) + (1.0 - d) * (incoming_sum + dangling_contribution);
    }
    
    free(args);
}

// Helper function to free the inverse graph memory
void free_inverse_graph(node **inverse_adj, int n) {
    for (int i = 0; i < n; i++) {
        node *curr = inverse_adj[i];
        while (curr != NULL) {
            node *temp = curr;
            curr = curr->next;
            free(temp);
        }
    }
    free(inverse_adj);
}

// Main PageRank function
void PageRank(Graph *g, int n_iters, float *rank) {
    int N = g->numVertices;
    
    // Initialize ranks to 1/N
    for (int i = 0; i < N; i++) {
        rank[i] = 1.0f / N;
    }
    
    // Allocate memory for out-degrees and inverse graph
    int *out_degree = (int *)calloc(N, sizeof(int));
    node **inverse_adj = (node **)malloc(N * sizeof(node *));
    
    // Initialize inverse adjacency list
    for (int i = 0; i < N; i++) {
        inverse_adj[i] = NULL;
    }

    // Build the inverse graph and calculate out-degrees
    // Iterate over the original graph
    for (int i = 0; i < N; i++) {
        node *curr = g->adjacencyLists[i];
        while (curr != NULL) {
            out_degree[i]++; // Increment out-degree for node i
            
            // Add edge to inverse graph: curr->v is pointed to by i
            node *new_node = (node *)malloc(sizeof(node));
            new_node->v = i;
            new_node->next = inverse_adj[curr->v];
            inverse_adj[curr->v] = new_node;
            
            curr = curr->next;
        }
    }

    // Create thread pool
    int num_threads = 8; // Number of threads to use
    thr_pool_t *pool = thr_pool_create(num_threads, num_threads, 60, NULL);
    float *new_ranks = (float *)malloc(N * sizeof(float));

    // Main iteration loop
    for (int iter = 0; iter < n_iters; iter++) {
        
        // Calculate sum of ranks for dangling nodes
        double dangling_sum = 0.0;
        for (int i = 0; i < N; i++) {
            if (out_degree[i] == 0) {
                dangling_sum += rank[i];
            }
        }

        // Distribute work to threads
        int chunk_size = (N + num_threads - 1) / num_threads; // Ceiling division

        for (int t = 0; t < num_threads; t++) {
            int start = t * chunk_size;
            int end = start + chunk_size;
            if (start >= N) break;
            if (end > N) end = N;

            ThreadArgs *args = (ThreadArgs *)malloc(sizeof(ThreadArgs));
            args->start_node = start;
            args->end_node = end;
            args->num_vertices = N;
            args->current_ranks = rank;
            args->new_ranks = new_ranks;
            args->out_degree = out_degree;
            args->inverse_adj = inverse_adj;
            args->dangling_sum = (float)dangling_sum;

            thr_pool_queue(pool, (void *(*)(void *))compute_pagerank_range, args);
        }

        // Wait for all threads to complete the current iteration
        thr_pool_wait(pool);

        // Update ranks for the next iteration
        for (int i = 0; i < N; i++) {
            rank[i] = new_ranks[i];
        }
    }

    // Clean up resources
    thr_pool_destroy(pool);
    free_inverse_graph(inverse_adj, N);
    free(out_degree);
    free(new_ranks);
}