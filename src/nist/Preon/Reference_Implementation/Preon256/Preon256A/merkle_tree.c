#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "aurora_inner.h"
#include "merkle_tree.h"
#include "util.h"
#include "params.h"
#include "rand.h"

// #define DEBUG

#ifdef DEBUG
static void print_bytes(const uint8_t *bytes, size_t len, const char *prefix)
{
    if (prefix != NULL)
    {
        printf("%s", prefix);
    }
    for (size_t i = 0; i < len; ++i)
    {
        printf("%02x", bytes[i]);
    }
}
#endif

static void SHA3_zk(uint8_t *hm, const uint8_t *msg, size_t msg_len, const uint8_t *rand, size_t rand_len, size_t bitSize)
{
    SHA3s(hm, (const uint8_t *[]){msg, rand}, (size_t[]){msg_len, rand_len}, 2, bitSize);
}

MerkleTree *merkle_tree_init(size_t leaf_content_len, size_t coset_serialization_size, int zk, const Parameters *params)
{
    size_t num_leaves = leaf_content_len / coset_serialization_size;
    assert(num_leaves > 0 && is_power_of_2(num_leaves));
    size_t inner_nodes_count = 2 * num_leaves - 1;
    size_t hash_bytesize = params->hash_bitsize / 8;
    // TODO: need a hash function capable of handling longer bytesize
    MerkleTree *mt = (MerkleTree *)malloc(sizeof(MerkleTree));
    if ((mt->_hash_digest = (uint8_t *)malloc(inner_nodes_count * hash_bytesize)) == NULL)
    {
        free(mt);
        return NULL;
    }
    mt->leaf_content_len = leaf_content_len;
    mt->coset_serialization_size = coset_serialization_size;
    mt->num_leaves = num_leaves;
    mt->zk = zk;
    mt->params = params;
    mt->_leaf_randomness = NULL;
    mt->_constructed = 0;
    return mt;
}

void merkle_tree_free(MerkleTree *merkle_tree)
{
    free(merkle_tree->_hash_digest);
    free(merkle_tree->_leaf_randomness);
    free(merkle_tree);
}

static int merkle_tree_sample_leaf_randomness(MerkleTree *);

static inline __attribute__((always_inline)) void uint64_t_to_uint8_t(uint8_t *dst, const uint64_t src)
{
    for (size_t b = 0; b < 8; b++)
    {
        dst[b] = (src >> (8 * (7 - b))) & 0xFF;
    }
}

static inline __attribute__((always_inline)) uint64_t read_uint64_t(const uint8_t *buf)
{
    uint64_t result = 0;
    for (size_t b = 0; b < 8; b++)
    {
        result |= (uint64_t)buf[b] << (8 * (7 - b));
    }
    return result;
}

int merkle_tree_compute(
    MerkleTree *merkle_tree, const uint64_t **leaf_contents, size_t leaf_contents_len)
{
    if (merkle_tree->_constructed)
    {
        return 1;
    }
    if (merkle_tree->zk)
    {
        int s = merkle_tree_sample_leaf_randomness(merkle_tree);
        if (s != 0)
        {
            return s;
        }
    }
    // helper variable
    size_t hash_bytesize = merkle_tree->params->hash_bytesize;
    size_t hash_bitsize = merkle_tree->params->hash_bitsize;
    size_t field_words = merkle_tree->params->field_bytesize / 8;
    /* Domain with the same size as inputs, used for getting coset positions */
    /* First hash the leaves. Since we are putting an entire coset into a leaf,
     * our slice is of size num_input_oracles * coset_size */
    size_t slice_bytesize = leaf_contents_len * merkle_tree->coset_serialization_size * merkle_tree->params->field_bytesize;
    uint8_t *slice = (uint8_t *)malloc(slice_bytesize);
    uint8_t *digest = (uint8_t *)malloc(hash_bytesize);
    for (size_t i = 0; i < merkle_tree->num_leaves; ++i)
    {
        for (size_t j = 0; j < merkle_tree->coset_serialization_size; j++)
        {
            for (size_t k = 0; k < leaf_contents_len; k++)
            {
                size_t offset = (j + k * merkle_tree->coset_serialization_size) * 8 * field_words;
                const uint64_t *leaf_content = &leaf_contents[k][(i * merkle_tree->coset_serialization_size + j) * field_words];
                for (size_t w = 0; w < field_words; ++w)
                {
                    uint64_t_to_uint8_t(&slice[offset], leaf_content[w]);
                    offset += 8;
                }
            }
        }

        if (merkle_tree->zk)
        {
            SHA3_zk(digest, slice, slice_bytesize, merkle_tree->_leaf_randomness + i * merkle_tree->params->hash_zk_bytesize, merkle_tree->params->hash_zk_bytesize, hash_bitsize);
#ifdef DEBUG
            printf("zk randomness = ");
            print_bytes(merkle_tree->_leaf_randomness + i * merkle_tree->params->hash_zk_bytesize, merkle_tree->params->hash_zk_bytesize, "0x");
            puts("");
#endif
        }
        else
        {
            SHA3(digest, slice, slice_bytesize, hash_bitsize);
        }
#ifdef DEBUG
        printf("slice = ");
        print_bytes(slice, slice_bytesize, "0x");
        puts("");
        printf("mt->digest[%lu] =", merkle_tree->num_leaves - 1 + i);
        print_bytes(digest, hash_bytesize, "0x");
        puts("");
#endif
        memcpy(&merkle_tree->_hash_digest[(merkle_tree->num_leaves - 1 + i) * hash_bytesize], digest, hash_bytesize);
    }
    free(slice);

/* Then hash all the layers */
#ifdef DEBUG
    puts("Then hash all the layers");
#endif
    size_t n = merkle_tree->num_leaves - 1;
    while (n > 0)
    {
        n /= 2;
        for (size_t i = n; i < 2 * n + 1; ++i)
        {
            SHA3(digest, &merkle_tree->_hash_digest[(2 * i + 1) * hash_bytesize], 2 * hash_bytesize, hash_bitsize);
            memcpy(&merkle_tree->_hash_digest[i * hash_bytesize], digest, hash_bytesize);
#ifdef DEBUG
            printf("mt->digest[%lu] =", i);
            print_bytes(digest, hash_bytesize, "0x");
            puts("");
#endif
        }
    }
    merkle_tree->_constructed = 1;
    free(digest);
    return 0;
}

static int merkle_tree_sample_leaf_randomness(MerkleTree *merkle_tree)
{
    size_t leaf_randomness_size = merkle_tree->num_leaves * merkle_tree->params->hash_zk_bytesize;
    uint8_t *leaf_randomness = (uint8_t *)malloc(leaf_randomness_size);
    if (leaf_randomness == NULL)
    {
        return 1;
    }
    if (random_bytes(leaf_randomness, leaf_randomness_size) != 0)
    {
        free(leaf_randomness);
        return 1;
    }
    merkle_tree->_leaf_randomness = leaf_randomness;
    return 0;
}

// ----- Membership Proof -----
MerkleTreeMembershipProof *merkle_tree_membership_proof_init()
{
    MerkleTreeMembershipProof *proof = (MerkleTreeMembershipProof *)malloc(sizeof(MerkleTreeMembershipProof));
    memset(proof, 0, sizeof(MerkleTreeMembershipProof));
    return proof;
}

static inline __attribute__((always_inline)) size_t sort_and_unique(size_t *key, size_t key_len)
{
    return sort_and_unique_values(key, key_len, NULL, 0, 0);
}

/*


                            0
                /                       \
              1                          2
           /       \                /        \
         3          4              5          6
        / \        / \            / \        / \
       7   8      9   10        11   12    13   14
       0   1      2   3         4    5     6    7

*/

int merkle_tree_get_membership_proof(MerkleTreeMembershipProof *proof, const MerkleTree *mt, size_t *positions, size_t positions_len)
{
    if (!mt->_constructed)
    {
        puts("not constructed");
        return 1;
    }
    positions_len = sort_and_unique(positions, positions_len);
    if (positions_len == 0)
    {
        return 0;
    }
    if (mt->zk)
    {
        if (proof->randomness_hashes == NULL)
        {
            proof->randomness_hashes = (uint8_t *)malloc(positions_len * mt->params->hash_zk_bytesize);
        }
        for (size_t i = 0; i < positions_len; ++i)
        {
            memcpy(&proof->randomness_hashes[i * mt->params->hash_zk_bytesize], &mt->_leaf_randomness[positions[i] * mt->params->hash_zk_bytesize], mt->params->hash_zk_bytesize);
        }
        proof->_zk = mt->zk;
        proof->_randomness_hashes_length = positions_len;
    }
    for (size_t i = 0; i < positions_len; ++i)
    {
        if (positions[i] >= mt->num_leaves)
        {
            printf("%zu >= %zu\n", positions[i], mt->num_leaves);
            return 1;
        }
        //  transform leaf positions to indices in mt->_hash_digest
        positions[i] += mt->num_leaves - 1;
    }
    proof->_coset_serialization_size = mt->coset_serialization_size;
    proof->_num_leaves = mt->num_leaves;
    const size_t hash_bytesize = mt->params->hash_bytesize;
    // malloc for worst case
    proof->proof = (uint8_t *)malloc(mt->num_leaves / 2 * hash_bytesize);
    proof->proof_length = 0;
    size_t *new_position = (size_t *)malloc(positions_len * sizeof(size_t));
    size_t new_position_len;
    size_t *cur_position = (size_t *)malloc(positions_len * sizeof(size_t));
    size_t cur_position_len = positions_len;
    memcpy(cur_position, positions, positions_len * sizeof(size_t));
    while (1) /* for every layer */
    {
        if (cur_position_len == 0 || cur_position[0] == 0)
        {
            /* we have arrived at the root */
            break;
        }
        new_position_len = 0;
        for (size_t i = 0; i < cur_position_len; ++i)
        {
            size_t it_pos = cur_position[i];

            /* Always process parent. */
            new_position[new_position_len++] = (it_pos - 1) / 2;

            if ((it_pos & 1) == 0)
            {
                /* We are the right node, so there was no left node
                   (o.w. would have been processed in b)
                   below). Insert it as auxiliary */
#ifdef DEBUG
                printf("Right node, add left node (%zu) to proof\n", it_pos - 1);
#endif
                memcpy(&proof->proof[(proof->proof_length++) * hash_bytesize], &mt->_hash_digest[(it_pos - 1) * hash_bytesize], hash_bytesize);
            }
            else
            {
                /* We are the left node. Two cases: */
                if (i + 1 == cur_position_len || cur_position[i + 1] != it_pos + 1)
                {
                    /* a) Our right sibling is not in S, so we must
                       insert auxiliary. */
#ifdef DEBUG
                    printf("Left node, add right node (%zu) to proof\n", it_pos + 1);
#endif
                    memcpy(&proof->proof[(proof->proof_length++) * hash_bytesize], &mt->_hash_digest[(it_pos + 1) * hash_bytesize], hash_bytesize);
                }
                else
                {
                    /* b) Our right sibling is in S. So don't need
                       auxiliary and skip over the right sibling.
                       (Note that only one parent will be processed.)
                    */
#ifdef DEBUG
                    printf("Left node(%zu) and right node(%zu) are known\n", it_pos, it_pos + 1);
#endif
                    ++i;
                }
            }
        }

        // swap
        size_t *tmp;
        tmp = cur_position;
        cur_position = new_position;
        new_position = tmp;
        size_t tmp_len;
        tmp_len = cur_position_len;
        cur_position_len = new_position_len;
        new_position_len = tmp_len;
    }
    free(new_position);
    free(cur_position);
    return 0;
}

int merkle_tree_validate_membership_proof(const MerkleTreeMembershipProof *proof, const Parameters *params, const uint8_t *root, size_t *positions, size_t positions_len, uint64_t **leaf_columns, size_t leaf_columns_len, size_t leaf_column_len)
{
    // Actually this positions is already sorted and unique
    positions_len = sort_and_unique_values(positions, positions_len, leaf_columns, leaf_column_len, params->field_words);
    if (positions_len == 0)
    {
        return 0;
    }
    // helper variables
    size_t field_words = params->field_words;
    size_t hash_bytesize = params->hash_bytesize;
    size_t hash_bitsize = params->hash_bitsize;
    uint8_t *hash_digest = (uint8_t *)malloc(positions_len * hash_bytesize);
    size_t slice_bytesize = leaf_column_len * params->field_bytesize;
    uint8_t *slice = (uint8_t *)malloc(slice_bytesize);
    uint8_t *digest = (uint8_t *)malloc(hash_bytesize);
    for (size_t i = 0; i < positions_len; ++i)
    {
        for (size_t j = 0; j < leaf_column_len * field_words; j++)
        {
            uint64_t_to_uint8_t(&slice[j * 8], leaf_columns[i][j]);
        }
        if (proof->_zk)
        {
            SHA3_zk(digest, slice, slice_bytesize, &proof->randomness_hashes[i * params->hash_zk_bytesize], params->hash_zk_bytesize, hash_bitsize);
#ifdef DEBUG
            printf("zk randomness = ");
            print_bytes(&proof->randomness_hashes[i * params->hash_zk_bytesize], params->hash_zk_bytesize, "0x");
            puts("");
#endif
        }
        else
        {
            SHA3(digest, slice, slice_bytesize, hash_bitsize);
        }
        memcpy(&hash_digest[hash_bytesize * i], digest, hash_bytesize);
        /* transform to sorted set of indices */
        positions[i] += proof->_num_leaves - 1;
#ifdef DEBUG
        printf("slice = ");
        print_bytes(slice, slice_bytesize, "0x");
        puts("");
        printf("mt->digest[%lu] =", positions[i]);
        print_bytes(digest, hash_bytesize, "0x");
        puts("");
#endif
    }
    free(slice);

    size_t aux_it = 0;
    size_t *new_position = (size_t *)malloc(positions_len * sizeof(size_t));
    size_t new_position_len;
    uint8_t *new_hash_digest = (uint8_t *)malloc(proof->_num_leaves / 2 * hash_bytesize);
    size_t *cur_position = (size_t *)malloc(positions_len * sizeof(size_t));
    size_t cur_position_len = positions_len;
    uint8_t *hash_message = (uint8_t *)malloc(2 * hash_bytesize);
    memcpy(cur_position, positions, positions_len * sizeof(size_t));
    while (1) /* for every layer */
    {
        if (cur_position_len == 0 || cur_position[0] == 0)
        {
            /* we have arrived at the root */
            break;
        }

        new_position_len = 0;
        for (size_t i = 0; i < cur_position_len; ++i)
        {
            size_t it_pos = cur_position[i];
            uint8_t *it_hash = &hash_digest[i * hash_bytesize];

            uint8_t *left_hash, *right_hash;

            if ((it_pos & 1) == 0)
            {
                /* We are the right node, so there was no left node
                   (o.w. would have been processed in b)
                   below). Take it from the auxiliary. */
#ifdef DEBUG
                printf("Right node, grap left node(%zu) from proof\n", it_pos - 1);
#endif
                left_hash = &proof->proof[(aux_it++) * hash_bytesize];
                right_hash = it_hash;
            }
            else
            {
                /* We are the left node. Two cases: */
                left_hash = it_hash;

                if (i + 1 == cur_position_len || cur_position[i + 1] != it_pos + 1)
                {
                    /* a) Our right sibling is not in S, so we must
                       take an auxiliary. */
                    right_hash = &proof->proof[(aux_it++) * hash_bytesize];
#ifdef DEBUG
                    printf("Left node, grap right node(%zu) from proof\n", it_pos + 1);
#endif
                }
                else
                {
                    /* b) Our right sibling is in S. So don't need
                       auxiliary and skip over the right sibling.
                       (Note that only one parent will be processed.)
                    */
#ifdef DEBUG
                    printf("Left node(%zu) and right node(%zu) are known\n", it_pos, it_pos + 1);
#endif
                    right_hash = &hash_digest[(++i) * hash_bytesize];
                }
            }

#ifdef DEBUG
            print_bytes(left_hash, hash_bytesize, "  left = 0x");
            puts("");
            print_bytes(right_hash, hash_bytesize, "  right = 0x");
            puts("");
#endif

            memcpy(&hash_message[0], left_hash, hash_bytesize);
            memcpy(&hash_message[hash_bytesize], right_hash, hash_bytesize);
            SHA3(digest, hash_message, 2 * hash_bytesize, hash_bitsize);

            new_position[new_position_len] = (it_pos - 1) / 2;
            memcpy(&new_hash_digest[new_position_len * hash_bytesize], digest, hash_bytesize);

#ifdef DEBUG
            printf("mt->digest[%zu] =", (it_pos - 1) / 2);
            print_bytes(&new_hash_digest[new_position_len * hash_bytesize], hash_bytesize, "0x");
            puts("");
#endif

            new_position_len++;
        }

        uint8_t *tmp_hash = new_hash_digest;
        new_hash_digest = hash_digest;
        hash_digest = tmp_hash;

        size_t tmp_len = new_position_len;
        new_position_len = cur_position_len;
        cur_position_len = tmp_len;

        size_t *tmp = new_position;
        new_position = cur_position;
        cur_position = tmp;
    }

    if (aux_it != proof->proof_length)
    {
        fprintf(stderr, "Validation did not consume the entire proof.\n");
        return 0;
    }

    int result = (memcmp(root, hash_digest, hash_bytesize) == 0);
#ifdef DEBUG
    print_bytes(root, hash_bytesize, "Root = 0x");
    puts("");
    print_bytes(hash_digest, hash_bytesize, "Calculated = 0x");
    puts("");
#endif
    free(digest);
    free(new_position);
    free(cur_position);
    free(new_hash_digest);
    free(hash_message);
    free(hash_digest);
    return result;
}

void merkle_tree_membership_proof_free(MerkleTreeMembershipProof *proof)
{
    free(proof->proof);
    free(proof->randomness_hashes);
    free(proof);
}

// Helper funciton
int merkle_tree_get_root(uint8_t *root, const MerkleTree *merkle_tree)
{
    if (!merkle_tree->_constructed)
    {
        return 1;
    }
    memcpy(root, merkle_tree->_hash_digest, merkle_tree->params->hash_bytesize);
    return 0;
}

void merkle_tree_get_leaf_contents_by_positions(uint64_t **out, const Parameters *params, size_t coset_serialization_size, const size_t *positions, size_t positions_len, const uint64_t **leaf_contents, size_t leaf_contents_len)
{
    const size_t chunk_size = coset_serialization_size * params->field_bytesize;
    const size_t chunk_word = chunk_size / 8;
    for (size_t i = 0; i < positions_len; ++i)
    {
        for (size_t j = 0; j < leaf_contents_len; ++j)
        {
            memcpy(&out[i][j * chunk_word], &leaf_contents[j][positions[i] * chunk_word], chunk_size);
        }
    }
}

// Serializer

#define SERIALIZE_UINT64(buf, cur, value)              \
    uint64_t_to_uint8_t(&(buf)[cur], (uint64_t)value); \
    cur += 8;

int merkle_tree_membership_proof_serialize_size(const MerkleTreeMembershipProof *proof, const Parameters *params)
{
    return 1 /* version */ +
           8 /* total length */ +
           8 /* proof_length */ +
           8 /* randomness_hashes_length*/ +
           8 /* coset_serialization_size */ +
           8 /* num_leaves */ +
           proof->proof_length * params->hash_bytesize /* proofs */ +
           proof->_randomness_hashes_length * params->hash_zk_bytesize /* zk randomness */;
}

int merkle_tree_membership_proof_serialize(const MerkleTreeMembershipProof *proof, const Parameters *params, uint8_t *buf)
{
    size_t len = merkle_tree_membership_proof_serialize_size(proof, params);
    size_t cur = 0;
    buf[cur++] = 1;
    SERIALIZE_UINT64(buf, cur, len);
    SERIALIZE_UINT64(buf, cur, proof->proof_length);
    SERIALIZE_UINT64(buf, cur, proof->_randomness_hashes_length);
    SERIALIZE_UINT64(buf, cur, proof->_coset_serialization_size);
    SERIALIZE_UINT64(buf, cur, proof->_num_leaves);
    memcpy(&buf[cur], proof->proof, proof->proof_length * params->hash_bytesize);
    cur += proof->proof_length * params->hash_bytesize;
    if (proof->_randomness_hashes_length)
    {
        memcpy(&buf[cur], proof->randomness_hashes, proof->_randomness_hashes_length * params->hash_zk_bytesize);
        cur += proof->_randomness_hashes_length * params->hash_zk_bytesize;
    }
    // self check
    if (len != cur)
    {
        fprintf(stderr, "internal error, expected length %zu, got %zu\n", len, cur);
        return 1;
    }
    return 0;
}

#undef SERIALIZE_UINT64

#define DESERIALIZE_UINT64(buf, cur) \
    read_uint64_t(&buf[cur]);        \
    cur += 8;

size_t merkle_tree_membership_proof_deserialize(const uint8_t *buf, const Parameters *params, MerkleTreeMembershipProof *proof)
{
    size_t cur = 0;
    if (buf[cur++] != 1)
    {
        fprintf(stderr, "cannot deserialize: unsupported version: %d\n", buf[0]);
        return 0;
    }
    size_t len = DESERIALIZE_UINT64(buf, cur);
    proof->proof_length = DESERIALIZE_UINT64(buf, cur);
    proof->_randomness_hashes_length = DESERIALIZE_UINT64(buf, cur);
    proof->_coset_serialization_size = DESERIALIZE_UINT64(buf, cur);
    proof->_num_leaves = DESERIALIZE_UINT64(buf, cur);
    proof->proof = (uint8_t *)malloc(proof->proof_length * params->hash_bytesize);
    memcpy(proof->proof, &buf[cur], proof->proof_length * params->hash_bytesize);
    cur += proof->proof_length * params->hash_bytesize;
    if (proof->_randomness_hashes_length)
    {
        proof->randomness_hashes = (uint8_t *)malloc(proof->_randomness_hashes_length * params->hash_zk_bytesize);
        memcpy(proof->randomness_hashes, &buf[cur], proof->_randomness_hashes_length * params->hash_zk_bytesize);
        cur += proof->_randomness_hashes_length * params->hash_zk_bytesize;
        proof->_zk = 1;
    }
    // self check
    if (len != cur)
    {
        fprintf(stderr, "internal error, expected length %zu, got %zu\n", len, cur);
        return 0;
    }
    return len;
}

#undef DESERIALIZE_UINT64
