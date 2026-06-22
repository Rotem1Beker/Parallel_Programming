//Rotem Beker 217386598


#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include <stdbool.h>
#include <omp.h>


typedef struct syncBlock {
    unsigned int count;       /* Counter for active readers */
    omp_lock_t readers_lock;  /* Protects the count field */
    omp_lock_t vertex_lock;   /* Exclusive writer / shared reader group lock */
} syncBlock;

/*
 * TreeNode — Standard BST node structure
 */
typedef struct TreeNode {
    syncBlock sync;           /* Initialized for tree structure root coordination */
    int data;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

/* Allocate and initialize a new node. */
TreeNode* createNode(int data);

/* Insert data into the BST. Returns the (possibly new) root. */
TreeNode* insertNode(TreeNode* root, int data);

/* Delete data from the BST if present. Returns the (possibly new) root. */
TreeNode* deleteNode(TreeNode* root, int data);

/* Return true iff data is in the BST. */
bool searchNode(TreeNode* root, int data);

/* Return a pointer to the node with the smallest key, or NULL if empty. */
TreeNode* findMin(TreeNode* root);

/* Print all values in sorted order, each followed by a space. */
void inorderTraversal(TreeNode* root);

/* Print values in pre-order (root, left, right). */
void preorderTraversal(TreeNode* root);

/* Print values in post-order (left, right, root). */
void postorderTraversal(TreeNode* root);

/* Free every node and destroy all locks. Does not print. */
void freeTree(TreeNode* root);

#endif // BINARY_TREE_H