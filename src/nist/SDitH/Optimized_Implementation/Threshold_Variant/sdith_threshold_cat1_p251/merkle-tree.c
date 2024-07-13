#include "merkle-tree.h"
#include "hash.h"
#include "parameters.h"
#include <stdlib.h>

static uint32_t nlz(uint32_t x)
{
    uint32_t n;

    if (x == 0) return (32);
    n = 1;
    if((x >> 16) == 0) {n = n + 16; x = x << 16;}
    if((x >> 24) == 0) {n = n + 8;  x = x << 8;}
    if((x >> 28) == 0) {n = n + 4;  x = x << 4;}
    if((x >> 30) == 0) {n = n + 2;  x = x << 2;}
    n = n - (x >> 31);

    return n;
}

static uint32_t ceil_log2(uint32_t x)
{
    if (x == 0) {
        return 0;
    }
    return 32 - nlz(x - 1);
}

#define get_node(tree,idx) tree->nodes[(idx)-1]
merkle_tree_t* malloc_merkle_tree(uint16_t nb_leaves)
{
    merkle_tree_t* tree = (merkle_tree_t*) malloc(sizeof(merkle_tree_t));

    uint8_t height = (uint8_t) ceil_log2(nb_leaves);
    // Even if the number of leaves is smaller than 2^16,
    //   the total number of nodes can be larger
    uint32_t nb_nodes = (1 << (height)) + nb_leaves - 1;

    tree->height = height;
    tree->nb_leaves = nb_leaves;
    tree->nb_nodes = nb_nodes;
    tree->nodes = (uint8_t**) malloc(nb_nodes * sizeof(uint8_t*));

    uint8_t* slab = malloc(nb_nodes*PARAM_DIGEST_SIZE);
    for (uint32_t i=1; i <= nb_nodes; i++) {
        get_node(tree, i) = slab;
        slab += PARAM_DIGEST_SIZE;
    }

    return tree;
}

void free_merkle_tree(merkle_tree_t* tree)
{
    if (tree != NULL) {
        free(tree->nodes[0]);
        free(tree->nodes);
        free(tree);
    }
}

/* Create a Merkle tree by hashing up all nodes.
 * leaf_data must have length tree->numNodes. */
void build_merkle_tree(merkle_tree_t* tree, uint8_t** leaf_data, const uint8_t* salt)
{
    uint32_t first_index = tree->nb_nodes - tree->nb_leaves + 1;
    uint32_t last_index = tree->nb_nodes;

    // Initialize the leaves with leaf_data
    for (uint16_t i = 0; i < tree->nb_leaves; i++)
        memcpy(get_node(tree,first_index+i), leaf_data[i], PARAM_DIGEST_SIZE);

    int h=tree->height-1;
    for(; h>= 2; h--) {
        // last_is_isolated:
        //   indicates if the last node has a single child
        int last_is_isolated = 1-(last_index & 0x1);
        // Current floor:
        //    {first_index, ..., last_index}
        first_index >>= 1;
        last_index >>= 1;
        // For each node
        uint32_t parent_index=first_index;
        for(; parent_index+4<=last_index; parent_index+=4) {
            hash_context_x4 ctx_x4;
            hash_init_prefix_x4(&ctx_x4, HASH_PREFIX_MERKLE_TREE);
            if(salt != NULL)
                hash_update_x4_1(&ctx_x4, salt, PARAM_SALT_SIZE);
            const uint16_t nodes[4] = {
                (uint16_t)(parent_index+0), (uint16_t)(parent_index+1),
                (uint16_t)(parent_index+2), (uint16_t)(parent_index+3)
            };
            hash_update_x4_uint16s_le(&ctx_x4, nodes);
            uint8_t const* left_childs[4] = {
                get_node(tree, 2*parent_index+0), get_node(tree, 2*parent_index+2),
                get_node(tree, 2*parent_index+4), get_node(tree, 2*parent_index+6)
            };
            uint8_t const* right_childs[4] = {
                get_node(tree, 2*parent_index+1), get_node(tree, 2*parent_index+3),
                get_node(tree, 2*parent_index+5), get_node(tree, 2*parent_index+7)
            };
            hash_update_x4(&ctx_x4, left_childs, PARAM_DIGEST_SIZE);
            hash_update_x4(&ctx_x4, right_childs, PARAM_DIGEST_SIZE);

            uint8_t* parents[4] = {
                get_node(tree, parent_index+0), get_node(tree, parent_index+1),
                get_node(tree, parent_index+2), get_node(tree, parent_index+3)
            };
            hash_final_x4(&ctx_x4, parents);
        }
        for(; parent_index<=last_index; parent_index++) {
            hash_context ctx;
            hash_init_prefix(&ctx, HASH_PREFIX_MERKLE_TREE);
            if(salt != NULL)
                hash_update(&ctx, salt, PARAM_SALT_SIZE);
            hash_update_uint16_le(&ctx, (uint16_t)parent_index);
            hash_update(&ctx, get_node(tree, 2*parent_index), PARAM_DIGEST_SIZE);
            if((parent_index<last_index) || (!last_is_isolated))
                hash_update(&ctx, get_node(tree, 2*parent_index+1), PARAM_DIGEST_SIZE);
            hash_final(&ctx, get_node(tree, parent_index));
        }
    }
    for(; h>= 0; h--) {
        // last_is_isolated:
        //   indicates if the last node has a single child
        int last_is_isolated = 1-(last_index & 0x1);
        // Current floor:
        //    {first_index, ..., last_index}
        first_index >>= 1;
        last_index >>= 1;
        // For each node
        for(uint32_t parent_index=first_index; parent_index<=last_index; parent_index++) {
            hash_context ctx;
            hash_init_prefix(&ctx, HASH_PREFIX_MERKLE_TREE);
            if(salt != NULL)
                hash_update(&ctx, salt, PARAM_SALT_SIZE);
            hash_update_uint16_le(&ctx, (uint16_t) parent_index);
            hash_update(&ctx, get_node(tree, 2*parent_index), PARAM_DIGEST_SIZE);
            if((parent_index<last_index) || (!last_is_isolated))
                hash_update(&ctx, get_node(tree, 2*parent_index+1), PARAM_DIGEST_SIZE);
            hash_final(&ctx, get_node(tree, parent_index));
        }
    }
}

uint8_t* get_root(merkle_tree_t* tree) {
    return get_node(tree, 1);
}

static uint16_t get_revealed_nodes(uint32_t* revealed_nodes, uint8_t tree_depth, uint16_t nb_leaves, uint16_t nb_revealed_leaves, const uint16_t* leaves_indexes) {
    uint16_t nb_revealed_nodes = 0; 

    // Initialisation
    uint32_t first_index = (uint32_t)(1<<tree_depth);
    uint32_t last_index = first_index + nb_leaves - 1;
    // We use "leaves" as a circular queue, so it destroys the input data.
    uint16_t queue_start = 0;
    uint16_t queue_stop = 0;
    uint32_t* queue_indexes = (uint32_t*) malloc(nb_revealed_leaves*sizeof(uint32_t));
    for(int i=0; i<nb_revealed_leaves; i++)
        queue_indexes[i] = first_index + leaves_indexes[i];
    
    // While the queue head does not corresponds to the root of the Merkle tree
    while(queue_indexes[queue_start] != 1) {
        // Get the first node in the queue
        uint32_t index = queue_indexes[queue_start];
        queue_start++;
        if(queue_start == nb_revealed_leaves)
            queue_start = 0;
        // Detect if we change of tree height
        if(index < first_index) {
            first_index >>= 1;
            last_index >>= 1;
        }
        // Get the sibling node
        int is_left_child = (index % 2 == 0);
        if(is_left_child && (index == last_index)) {
            // The node has no sibling node
        } else {
            // The node HAS a sibling node
            uint32_t candidate_index = queue_indexes[queue_start];
            int queue_is_empty = (queue_start == queue_stop);
            if(is_left_child && (candidate_index==index+1) && !queue_is_empty) {
                queue_start++;
                if(queue_start == nb_revealed_leaves)
                    queue_start = 0;
            } else {
                // The sibling node is given in the authentication paths
                if(revealed_nodes != NULL) {
                    if(is_left_child)
                            revealed_nodes[nb_revealed_nodes] = index+1;
                    else    revealed_nodes[nb_revealed_nodes] = index-1;
                }
                nb_revealed_nodes++;
            }
        }

        // Compute the parent node, and push it in the queue
        uint32_t parent_index = index >> 1;
        queue_indexes[queue_stop] = parent_index;
        queue_stop++;
        if(queue_stop == nb_revealed_leaves)
            queue_stop = 0;
    }

    // Free memory
    free(queue_indexes);
    return nb_revealed_nodes;
}

uint8_t* open_merkle_tree(merkle_tree_t* tree, uint16_t* open_leaves, uint16_t nb_open_leaves, uint32_t* auth_size)
{
    uint32_t* revealed = malloc(tree->height*nb_open_leaves*sizeof(uint32_t));
    uint16_t nb_revealed = get_revealed_nodes(revealed, tree->height, tree->nb_leaves, nb_open_leaves, open_leaves);

    *auth_size = nb_revealed * PARAM_DIGEST_SIZE;
    uint8_t* output = malloc(*auth_size);
    uint8_t* outputBase = output;

    for (uint16_t i = 0; i < nb_revealed; i++) {
        memcpy(output, get_node(tree,revealed[i]), PARAM_DIGEST_SIZE);
        output += PARAM_DIGEST_SIZE;
    }

    free(revealed);

    return outputBase;
}

uint32_t get_auth_size(uint8_t tree_depth, uint16_t nb_leaves, uint16_t nb_revealed_leaves, const uint16_t* leaves_indexes) {
    uint16_t nb_revealed_nodes = get_revealed_nodes(NULL, tree_depth, nb_leaves, nb_revealed_leaves, leaves_indexes);
    return PARAM_DIGEST_SIZE*nb_revealed_nodes;
}

//  - dataSize: size of the digest (in bytes)
//  - tree_depth: depth of the Merkle tree (for example, 8 for a tree with 256 leaves)
//  - nb_leaves: Number of leaves in the Merkle Tree
//  - nb_revealed_leaves: Number of open leaves
//  - leaves_indexes: the index of the open leaves: 0 for the first leaf, ... Must be in the increasing order
//  - leaves: the open leaves in the same order than leaves_indexes
//  - auth: the authentication paths
//  - authSize: size (in bytes) of auth
//  - salt: the salt
// Warning: it destroy the value in leaves
int get_merkle_root_from_auth(uint8_t* root, uint8_t tree_depth, uint16_t nb_leaves, uint16_t nb_revealed_leaves, const uint16_t* leaves_indexes, uint8_t* leaves, const uint8_t* auth, uint32_t auth_size, const uint8_t* salt) {
    // Initialisation
    uint32_t first_index = (uint16_t)(1<<tree_depth);
    uint32_t last_index = first_index + nb_leaves - 1;
    // We use "leaves" as a circular queue, so it destroy the input data.
    uint8_t* queue = leaves;
    uint16_t queue_start = 0;
    uint16_t queue_stop = 0;
    uint32_t* queue_indexes = (uint32_t*) malloc(nb_revealed_leaves*sizeof(uint32_t));
    for(uint16_t i=0; i<nb_revealed_leaves; i++)
        queue_indexes[i] = first_index + leaves_indexes[i];
    
    // While the queue head does not corresponds to the root of the Merkle tree
    while(queue_indexes[queue_start] != 1) {
        // Get the first node in the queue
        const uint8_t* node = &queue[PARAM_DIGEST_SIZE*queue_start];
        uint32_t index = queue_indexes[queue_start];
        queue_start++;
        if(queue_start == nb_revealed_leaves)
            queue_start = 0;
        // Detect if we change of tree height
        if(index < first_index) {
            first_index >>= 1;
            last_index >>= 1;
        }
        // Get the sibling node
        const uint8_t* node2;
        int is_left_child = (index % 2 == 0);
        if(is_left_child && (index == last_index)) {
            // The node has no sibling node
            node2 = NULL;
        } else {
            // The node HAS a sibling node
            uint32_t candidate_index = queue_indexes[queue_start];
            int queue_is_empty = (queue_start == queue_stop);
            if(is_left_child && (candidate_index==index+1) && !queue_is_empty) {
                // The sibling node is already in the queue
                // Since the nodes in the queue are sorted,
                //   the sibling node is necessarily at the top of the queue
                // So, we pop this node from the queue
                node2 = &queue[PARAM_DIGEST_SIZE*queue_start];
                queue_start++;
                if(queue_start == nb_revealed_leaves)
                    queue_start = 0;
            } else {
                // The sibling node is given in the authentication paths
                if(auth_size >= PARAM_DIGEST_SIZE) {
                    node2 = &auth[0];
                    auth += PARAM_DIGEST_SIZE;
                    auth_size -= PARAM_DIGEST_SIZE;
                } else {
                    // Failure: the authentication paths are not long enough
                    return -1;
                }
                // Swap the both nodes
                if(!is_left_child) {
                    const uint8_t* tmp = node;
                    node = node2;
                    node2 = tmp;
                }
            }
        }

        // Compute the parent node, and push it in the queue
        uint32_t parent_index = index >> 1;
        hash_context ctx;
        hash_init_prefix(&ctx, HASH_PREFIX_MERKLE_TREE);
        if(salt != NULL)
            hash_update(&ctx, salt, PARAM_SALT_SIZE);
        hash_update_uint16_le(&ctx, (uint16_t) parent_index);
        hash_update(&ctx, node, PARAM_DIGEST_SIZE);
        if(node2 != NULL)
            hash_update(&ctx, node2, PARAM_DIGEST_SIZE);
        hash_final(&ctx, &queue[PARAM_DIGEST_SIZE*queue_stop]);

        queue_indexes[queue_stop] = parent_index;
        queue_stop++;
        if(queue_stop == nb_revealed_leaves)
            queue_stop = 0;
    }

    memcpy(root, &queue[PARAM_DIGEST_SIZE*queue_start], PARAM_DIGEST_SIZE);

    // Free memory
    free(queue_indexes);
    return 0;
}

