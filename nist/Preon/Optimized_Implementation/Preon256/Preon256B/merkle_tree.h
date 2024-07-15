#ifndef _MERKLE_TREE_H__
#define _MERKLE_TREE_H__

#include <stddef.h>
#include <stdint.h>

#include "params.h"

typedef struct
{
    size_t leaf_content_len;
    size_t coset_serialization_size;
    size_t num_leaves;
    int zk; // TODO: remove this, force zk
    const Parameters *params;

    int _constructed;
    uint8_t *_hash_digest;
    uint8_t *_leaf_randomness;
} MerkleTree;

typedef struct
{
    uint8_t *proof;
    uint8_t *randomness_hashes;
    size_t proof_length; // TODO: remove this

    // TODO: remove all below
    size_t _randomness_hashes_length;
    size_t _coset_serialization_size;
    size_t _num_leaves;
    int _zk;
} MerkleTreeMembershipProof;

MerkleTree *merkle_tree_init(size_t leaf_content_len, size_t coset_serialization_size, int zk, const Parameters *params);
int merkle_tree_compute(MerkleTree *merkle_tree, const uint64_t **leaf_contents, const size_t leaf_contents_len);
void merkle_tree_free(MerkleTree *);

// Membership Proof
MerkleTreeMembershipProof *merkle_tree_membership_proof_init();
int merkle_tree_get_membership_proof(MerkleTreeMembershipProof *proof, const MerkleTree *mt, size_t *positions, size_t positions_len);
int merkle_tree_validate_membership_proof(const MerkleTreeMembershipProof *proof, const Parameters *params, const uint8_t *root, size_t *positions, size_t positions_len, uint64_t **leaf_columns, size_t leaf_columns_len, size_t leaf_column_len);
void merkle_tree_membership_proof_free(MerkleTreeMembershipProof *proof);

// Serializer
int merkle_tree_membership_proof_serialize_size(const MerkleTreeMembershipProof *proof, const Parameters *params);
int merkle_tree_membership_proof_serialize(const MerkleTreeMembershipProof *proof, const Parameters *params, uint8_t *buf);
// merkle_tree_membership_proof_deserialize returns the length of buffer read. 0 on error.
size_t merkle_tree_membership_proof_deserialize(const uint8_t *buf, const Parameters *params, MerkleTreeMembershipProof *proof);

// Helper function
int merkle_tree_get_root(uint8_t *root, const MerkleTree *merkle_tree);
void merkle_tree_get_leaf_contents_by_positions(uint64_t **out, const Parameters *params, size_t coset_serialization_size, const size_t *positions, size_t positions_len, const uint64_t **leaf_contents, size_t leaf_contents_len);

#endif
