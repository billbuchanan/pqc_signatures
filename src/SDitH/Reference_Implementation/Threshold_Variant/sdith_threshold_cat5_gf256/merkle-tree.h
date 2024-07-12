#ifndef SDITH_MERKLE_TREE_H
#define SDITH_MERKLE_TREE_H

#include <stdint.h>

/* In this implementation of Merkle tree,
 *   the number of leaves is (strictly) upper bound by 2^16.
 *   Thus, when hashing the index of the internal nodes,
 *   the index is encoded on two bytes.
 */

typedef struct merkle_tree_t {
    uint8_t height;      /* The height of the tree */
    uint8_t** nodes;      /* The data for each node (1-based array) */
    uint32_t nb_nodes;    /* The total number of nodes in the tree  */
    uint16_t nb_leaves;   /* The total number of leaves in the tree */
} merkle_tree_t;

merkle_tree_t* malloc_merkle_tree(uint16_t nb_leaves);
void free_merkle_tree(merkle_tree_t* tree);
void build_merkle_tree(merkle_tree_t* tree, uint8_t** leaf_data, const uint8_t* salt);
uint8_t* get_root(merkle_tree_t* tree);
uint8_t* open_merkle_tree(merkle_tree_t* tree, uint16_t* open_leaves, uint16_t nb_open_leaves, uint32_t* auth_size);
uint32_t get_auth_size(uint8_t tree_depth, uint16_t nb_leaves, uint16_t nb_revealed_leaves, const uint16_t* leaves_indexes);
// Warning, it destroys the values in "leaves".
int get_merkle_root_from_auth(uint8_t* root, uint8_t tree_depth, uint16_t nb_leaves, uint16_t nb_revealed_leaves, const uint16_t* leaves_indexes, uint8_t* leaves, const uint8_t* auth, uint32_t auth_size, const uint8_t* salt);

#endif /* SDITH_MERKLE_TREE_H */
