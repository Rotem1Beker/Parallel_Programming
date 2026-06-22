// Rotem Beker 217386598

#include "binary_tree.h"
#include <stdio.h>
#include <stdlib.h>

void lock_as(bool is_writer, syncBlock* sync);
void unlock_as(bool is_writer, syncBlock* sync);

syncBlock init_sync() {
    syncBlock sync;
    sync.count = 0;
    omp_init_lock(&(sync.readers_lock));
    omp_init_lock(&(sync.vertex_lock));
    return sync;
}

void destroy_sync(syncBlock* sync) {
    omp_destroy_lock(&(sync->readers_lock));
    omp_destroy_lock(&(sync->vertex_lock));
}

TreeNode* createNode(int data) {
    TreeNode* node = malloc(sizeof(TreeNode));
    if (node) {
        node->sync = init_sync();
        node->data = data;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

void freeTree(TreeNode* node) {
    if (node == NULL) return;
    freeTree(node->left);
    freeTree(node->right);
    destroy_sync(&(node->sync));
    free(node);
}

/* Sequential insertion engine */
static TreeNode* insert_sequential(TreeNode* current, int data) {
    if (current == NULL) {
        return createNode(data);
    }
    if (data < current->data) {
        current->left = insert_sequential(current->left, data);
    } else if (data > current->data) {
        current->right = insert_sequential(current->right, data);
    }
    return current;
}

TreeNode* insertNode(TreeNode* root, int data) {
    if (root == NULL) { 
        return createNode(data); 
    }

    // Track locked sync blocks explicitly before pointer mutations modify the root structure address
    syncBlock* lock_root = &(root->sync);
    lock_as(true, lock_root);
    
    root = insert_sequential(root, data);
    
    unlock_as(true, lock_root);
    return root;
}

/* Sequential deletion engine */
static TreeNode* delete_sequential(TreeNode* current, int data) {
    if (current == NULL) return NULL;

    if (data < current->data) {
        current->left = delete_sequential(current->left, data);
    } else if (data > current->data) {
        current->right = delete_sequential(current->right, data);
    } else {
        // Target node located
        if (current->left == NULL) {
            TreeNode* temp = current->right;
            destroy_sync(&(current->sync));
            free(current);
            return temp;
        } else if (current->right == NULL) {
            TreeNode* temp = current->left;
            destroy_sync(&(current->sync));
            free(current);
            return temp;
        }

        // Deletion case handling for 2 active child branches
        TreeNode* successor = current->right;
        while (successor->left != NULL) {
            successor = successor->left;
        }
        current->data = successor->data;
        current->right = delete_sequential(current->right, successor->data);
    }
    return current;
}

TreeNode* deleteNode(TreeNode* root, int data) {
    if (root == NULL) return NULL;

    // Fixed: Retain structural lock block context to prevent leaked lock references or NULL escapes
    syncBlock* lock_root = &(root->sync);
    lock_as(true, lock_root);

    root = delete_sequential(root, data);

    unlock_as(true, lock_root);
    return root;
}

bool searchNode(TreeNode* root, int data) {
    if (root == NULL) return false;

    lock_as(false, &(root->sync));
    TreeNode* current = root;
    bool found = false;

    while (current != NULL) {
        if (data == current->data) {
            found = true;
            break;
        } else if (data < current->data) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    unlock_as(false, &(root->sync));
    return found;
}

TreeNode* findMin(TreeNode* root) {
    if (root == NULL) return NULL;

    lock_as(false, &(root->sync));
    TreeNode* current = root;
    while (current->left != NULL) {
        current = current->left;
    }
    TreeNode* res = current;
    unlock_as(false, &(root->sync));
    return res;
}

/* Internal explicit walk printers avoiding internal lock traps */
static void inorder_walk(TreeNode* node) {
    if (node == NULL) return;
    inorder_walk(node->left);
    printf("%d ", node->data);
    inorder_walk(node->right);
}

void inorderTraversal(TreeNode* root) {
    if (root == NULL) return;
    lock_as(false, &(root->sync));
    inorder_walk(root);
    unlock_as(false, &(root->sync));
}

static void preorder_walk(TreeNode* node) {
    if (node == NULL) return;
    printf("%d ", node->data);
    preorder_walk(node->left);
    preorder_walk(node->right);
}

void preorderTraversal(TreeNode* root) {
    if (root == NULL) return;
    lock_as(false, &(root->sync));
    preorder_walk(root);
    unlock_as(false, &(root->sync));
}

static void postorder_walk(TreeNode* node) {
    if (node == NULL) return;
    postorder_walk(node->left);
    postorder_walk(node->right);
    printf("%d ", node->data);
}

void postorderTraversal(TreeNode* root) {
    if (root == NULL) return;
    lock_as(false, &(root->sync));
    postorder_walk(root);
    unlock_as(false, &(root->sync));
}

/* ----- Reader-Writer Lock Protocol Implementation ----- */

void lock_as(bool is_writer, syncBlock* sync) {
    if (is_writer) {
        omp_set_lock(&(sync->vertex_lock));
    } else {
        omp_set_lock(&(sync->readers_lock));
        sync->count++;
        if (sync->count == 1) {
            omp_set_lock(&(sync->vertex_lock));
        }
        omp_unset_lock(&(sync->readers_lock));
    }
}

void unlock_as(bool is_writer, syncBlock* sync) {
    if (is_writer) {
        omp_unset_lock(&(sync->vertex_lock));
    } else {
        omp_set_lock(&(sync->readers_lock));
        sync->count--;
        if (sync->count == 0) {
            omp_unset_lock(&(sync->vertex_lock));
        }
        omp_unset_lock(&(sync->readers_lock));
    }
}