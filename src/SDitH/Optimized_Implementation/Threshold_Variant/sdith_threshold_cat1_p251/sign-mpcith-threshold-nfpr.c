#include "sign-mpcith.h"
#include "rnd.h"
#include "merkle-tree.h"
#include "commit.h"
#include "hash.h"
#include "keygen.h"
#include "mpc.h"
#include "views.h"
#include "parameters-all.h"
#include "share.h"
#include "serialization.h"
#include <string.h>
#include <stdio.h>

/***********************************************
 *            Signature Structure              *
 ***********************************************/

typedef struct proof_threshold_nfpr_t {
    uint8_t* cv_info; // Information required to authenticate the opened commitment in the Merkle tree
    uint32_t len_cv_info; // Length of cv_info buffer

    // All the broadcast messages
    mpc_broadcast_t* broadcast[PARAM_NB_REVEALED];

    // Revealed party's shares
    mpc_wit_t* wit[PARAM_NB_REVEALED];
} proof_threshold_nfpr_t;

typedef struct signature_threshold_nfpr_t {
    uint8_t* salt;
    uint8_t* mpc_challenge_hash;
    // No need of view_challenge_hash, it can be recomputed
    // Plaintext broadcast messages
    mpc_broadcast_t* plain_broadcast;
    proof_threshold_nfpr_t proofs[PARAM_NB_EXECUTIONS];
    uint8_t* allocated_memory; // Just to manage the memory
} signature_threshold_nfpr_t;

// For parsing
typedef struct const_proof_threshold_nfpr_t {
    const uint8_t* cv_info; // Information required to authenticate the opened commitment in the Merkle tree
    uint32_t len_cv_info; // Length of cv_info buffer

    // All the broadcast messages
    const mpc_broadcast_t* broadcast[PARAM_NB_REVEALED];

    // Revealed party's shares
    const mpc_wit_t* wit[PARAM_NB_REVEALED];
} const_proof_threshold_nfpr_t;

typedef struct const_signature_threshold_nfpr_t {
    const uint8_t* salt;
    const uint8_t* mpc_challenge_hash;
    // No need of view_challenge_hash, it can be recomputed
    // Plaintext broadcast messages
    const mpc_broadcast_t* plain_broadcast;
    const_proof_threshold_nfpr_t proofs[PARAM_NB_EXECUTIONS];
    uint8_t* allocated_memory; // Just to manage the memory
} const_signature_threshold_nfpr_t;

// Free signature structure
void free_signature(signature_threshold_nfpr_t* sig);
void free_const_signature(const_signature_threshold_nfpr_t* sig);

// For signing
signature_threshold_nfpr_t* init_signature_structure(const uint8_t* salt, uint8_t* buf, size_t buflen);
int build_signature(const signature_threshold_nfpr_t* sig, uint8_t* buf, size_t buflen);

// For verification
const_signature_threshold_nfpr_t* parse_signature(const uint8_t* buf, size_t buflen, uint16_t open_views[PARAM_NB_EXECUTIONS][PARAM_NB_REVEALED], const uint8_t* m, size_t mlen);

/***********************************************
 *             Hash for challenge              *
 ***********************************************/

static void hash_for_mpc_challenge(uint8_t challenge_hash[PARAM_DIGEST_SIZE], uint8_t merkle_roots[PARAM_NB_EXECUTIONS][PARAM_DIGEST_SIZE],
    const instance_t* inst, const uint8_t* salt, const uint8_t* message, size_t mlen)
{
    hash_context ctx;
    hash_init_prefix(&ctx, HASH_PREFIX_FIRST_CHALLENGE);
    if(inst != NULL)
        hash_update_instance(&ctx, inst);
    if(mlen > 0)
        hash_update(&ctx, message, mlen);
    hash_update(&ctx, salt, PARAM_SALT_SIZE);

    for (size_t e=0; e<PARAM_NB_EXECUTIONS; e++)
        hash_update(&ctx, merkle_roots[e], PARAM_DIGEST_SIZE);

    hash_final(&ctx, challenge_hash);
}

static void hash_for_view_challenge(uint8_t challenge_hash[PARAM_DIGEST_SIZE], const uint8_t mpc_challenge_hash[PARAM_DIGEST_SIZE],
    const mpc_broadcast_t* broadcast[PARAM_NB_EXECUTIONS][PARAM_NB_REVEALED], const mpc_broadcast_t* plain_broadcast, 
    const uint8_t* salt, const uint8_t* message, size_t mlen)
{
    hash_context ctx;
    hash_init_prefix(&ctx, HASH_PREFIX_SECOND_CHALLENGE);
    if(mlen > 0)
        hash_update(&ctx, message, mlen);
    hash_update(&ctx, salt, PARAM_SALT_SIZE);
    hash_update(&ctx, mpc_challenge_hash, PARAM_DIGEST_SIZE);

    hash_update(&ctx, (const uint8_t*) plain_broadcast, PARAM_UNIF_SIZE); // We do not commit the v's zero values
    for(size_t e=0; e<PARAM_NB_EXECUTIONS; e++)
        for(size_t p=0; p<PARAM_NB_REVEALED; p++)
            hash_update(&ctx, (const uint8_t*) broadcast[e][p], PARAM_BR_SIZE);

    hash_final(&ctx, challenge_hash);
}

/***********************************************
 *             Signing Algorithm               *
 ***********************************************/

int sdith_threshold_nfpr_sign(uint8_t* sig, size_t* siglen,
                const uint8_t* m, size_t mlen,
                const uint8_t* sk,
                const uint8_t* salt, const uint8_t* seed
                ) {
    uint32_t i, j, e, p;
    int ret;

#ifndef NDEBUG
    // In debug mode, let us check if the key generation
    //    produces valid key pair. 
    ret = sdith_validate_keys(NULL, sk);
    if(ret)
        printf("Error: SK invalid for signing.\n");
#endif

    // Deserialize the private key
    sdith_secret_key_t ssk;
    ret = deserialize_secret_key(&ssk, sk, PARAM_SECRETKEYBYTES);
    if(ret < 0)
        return ret;
    uncompress_instance(ssk.inst);
    mpc_wit_t* plain_wit = ssk.wit;

    // Signature Structure
    signature_threshold_nfpr_t* ssig = init_signature_structure(salt, sig, PARAM_SIGNATURE_SIZEBYTES);

    /********************************************/
    /**********     INITIALIZATION     **********/
    /********************************************/

    prg_context entropy_ctx;
    prg_init(&entropy_ctx, seed, ssig->salt);
    samplable_t entropy = prg_to_samplable(&entropy_ctx);

    // Allocate memory
    uint8_t merkle_roots[PARAM_NB_EXECUTIONS][PARAM_DIGEST_SIZE];
    merkle_tree_t* merkle_trees[PARAM_NB_EXECUTIONS];
    for(e=0; e<PARAM_NB_EXECUTIONS; e++)
        merkle_trees[e] = malloc_merkle_tree(PARAM_NB_PARTIES);
    uint8_t** share_commitments = (uint8_t**) calloc(PARAM_NB_PARTIES, sizeof(uint8_t*));
    for(i=0; i<PARAM_NB_PARTIES; i++)
        share_commitments[i] = malloc(PARAM_DIGEST_SIZE);

    // Prepare the plain MPC input
    mpc_share_t* plain = new_share();
    vec_set(get_wit(plain), plain_wit, PARAM_WIT_SIZE);
    vec_rnd(get_unif(plain), PARAM_UNIF_SIZE, &entropy);
    compute_correlated(get_corr(plain), plain_wit, get_unif(plain));

    /********************************************/
    /********   COMMIT PARTIES' INPUTS   ********/
    /********************************************/

    // Sample randomness for sharing
    uint8_t rnd_buffer[PARAM_NB_EXECUTIONS][PARAM_NB_REVEALED][PARAM_SHARE_SIZE];
    vec_rnd(rnd_buffer, sizeof(rnd_buffer), &entropy);
    const mpc_share_t* rnd[PARAM_NB_EXECUTIONS][PARAM_NB_REVEALED];
    for(e=0; e<PARAM_NB_EXECUTIONS; e++)
        for(p=0; p<PARAM_NB_REVEALED; p++)
            rnd[e][p] = (mpc_share_t*) rnd_buffer[e][p];

    // Compute and commit the input shares
    mpc_share_t* shares[4];
    for(j=0; j<4; j++)
        shares[j] = new_share();
    for(e=0; e<PARAM_NB_EXECUTIONS; e++) {

        // Compute shares and commit them
        // Note: in case the platform has a implementation
        //    to hash four times in parallel, we pack values
        //    to commit in groups of four.
        for(i=0; i+3<PARAM_NB_PARTIES; i+=4) {
            for(j=0; j<4; j++)
                compute_complete_share(shares[j], plain, rnd[e], (uint8_t) (i+j));

            uint8_t* digests[4] = {
                share_commitments[i],   share_commitments[i+1],
                share_commitments[i+2], share_commitments[i+3]
            };
            const uint16_t is[4] = {(uint16_t) i, (uint16_t) i+1, (uint16_t) i+2, (uint16_t) i+3}; 
            commit_share_x4(digests, (mpc_share_t const*const*) shares, ssig->salt, (uint16_t) e, is);
        }
        for(; i<PARAM_NB_PARTIES; i++) {
            compute_complete_share(shares[0], plain, rnd[e], (uint8_t) i);
            commit_share(share_commitments[i], shares[0], ssig->salt, (uint16_t) e, (uint16_t) i);
        }

        // Build the Merkle tree
        build_merkle_tree(merkle_trees[e], share_commitments, NULL);
        memcpy(merkle_roots[e], get_root(merkle_trees[e]), PARAM_DIGEST_SIZE);
    }
    for(j=0; j<4; j++)
        free(shares[j]);

    // Expand the MPC challenge
    mpc_challenge_t* mpc_challenge = new_challenge();
    hash_for_mpc_challenge(ssig->mpc_challenge_hash, merkle_roots, ssk.inst, ssig->salt, NULL, 0);
    expand_mpc_challenge_hash(&mpc_challenge, ssig->mpc_challenge_hash, 1, ssk.inst);

    /********************************************/
    /*********  SIMULATE MPC PROTOCOL  **********/
    /********************************************/

    // Compute the values open by the parties
    mpc_compute_plain_broadcast(ssig->plain_broadcast, mpc_challenge, plain, ssk.inst);

    // Compute the randomness involved in the broadcast sharings 
    mpc_broadcast_t* broadcast[PARAM_NB_EXECUTIONS][PARAM_NB_REVEALED];
    for(e=0; e<PARAM_NB_EXECUTIONS; e++) {
        for(p=0; p<PARAM_NB_REVEALED; p++) {
            broadcast[e][p] = ssig->proofs[e].broadcast[p];
            mpc_compute_communications(broadcast[e][p], mpc_challenge, rnd[e][p], ssig->plain_broadcast, ssk.inst, 0);
        }
    }

    // Expand the view challenge
    uint16_t open_views[PARAM_NB_EXECUTIONS][PARAM_NB_REVEALED];
    uint8_t challenge_views_hash[PARAM_DIGEST_SIZE];
    hash_for_view_challenge(challenge_views_hash, ssig->mpc_challenge_hash, (const mpc_broadcast_t*(*)[PARAM_NB_REVEALED]) broadcast, ssig->plain_broadcast, ssig->salt, m, mlen);
    expand_view_challenge_hash((uint16_t*) open_views, challenge_views_hash, PARAM_NB_EXECUTIONS, PARAM_NB_REVEALED);

    /********************************************/
    /**********   FINALIZE SIGNATURE   **********/
    /********************************************/

    for(e=0; e<PARAM_NB_EXECUTIONS; e++) {
        // Build the authentication path in the Merkle tree
        uint8_t* ptr = open_merkle_tree(merkle_trees[e], open_views[e], PARAM_NB_REVEALED, &ssig->proofs[e].len_cv_info);
        memcpy(ssig->proofs[e].cv_info, ptr, ssig->proofs[e].len_cv_info);
        free(ptr);

        // Get the share of the witness for the open parties
        for(p=0; p<PARAM_NB_REVEALED; p++)
            compute_share_wit(ssig->proofs[e].wit[p], plain, rnd[e], (uint8_t) open_views[e][p]);
    }

    // Serialize the signature
    ret = build_signature(ssig, sig, PARAM_SIGNATURE_SIZEBYTES);
    free_signature(ssig);

    for(e=0; e<PARAM_NB_EXECUTIONS; e++)
        free_merkle_tree(merkle_trees[e]);
    for(i=0; i<PARAM_NB_PARTIES; i++)
        free(share_commitments[i]);
    free(share_commitments);
    sdith_free_keys_internal(NULL, &ssk);
    free(mpc_challenge);
    free(plain);

    if(ret < 0)
        return ret;
    *siglen = (size_t) ret;
    return 0;
}

/***********************************************
 *           Verification Algorithm            *
 ***********************************************/

int sdith_threshold_nfpr_sign_verify(const uint8_t* sig, size_t siglen,
                const uint8_t* m, size_t mlen,
                const uint8_t* pk
                ) {
    uint32_t i, e, p;
    int ret;

    // Deserialize the public key
    sdith_public_key_t ppk;
    ret = deserialize_public_key(&ppk, pk, PARAM_PUBLICKEYBYTES);
    if (ret < 0)
        return ret;
    uncompress_instance(ppk.inst);

    // Parse the signature
    // Note: while parsing, it expands the view challenge.
    uint16_t open_views[PARAM_NB_EXECUTIONS][PARAM_NB_REVEALED];
    const_signature_threshold_nfpr_t* ssig = parse_signature(sig, siglen, open_views, m, mlen);
    if(ssig == NULL) {
        ret = -1;
        sdith_free_keys_internal(&ppk, NULL);
        return ret;
    }

    // Initialization
    uint8_t merkle_roots[PARAM_NB_EXECUTIONS][PARAM_DIGEST_SIZE];
    uint8_t share_commitments[PARAM_NB_REVEALED][PARAM_DIGEST_SIZE];

    // Expand MPC Challenge
    mpc_challenge_t* mpc_challenge = new_challenge();
    expand_mpc_challenge_hash(&mpc_challenge, ssig->mpc_challenge_hash, 1, ppk.inst);

    mpc_share_t* share = new_share();
    mpc_broadcast_t* sh_broadcast = new_br();
    // Loop over all the open parties
    for(e=0; e<PARAM_NB_EXECUTIONS; e++) {
        for(p=0; p<PARAM_NB_REVEALED; p++) {
            i = open_views[e][p];

            // Compute the share broadcast by the current party
            compute_share_broadcast(sh_broadcast, ssig->plain_broadcast, ssig->proofs[e].broadcast, (uint8_t) i);

            // Build the input share of the current party
            //    by reversing the MPC protocol
            vec_set(get_wit(share), ssig->proofs[e].wit[p], PARAM_WIT_SIZE);
            mpc_compute_communications_inverse(share, mpc_challenge, sh_broadcast, ssig->plain_broadcast, ppk.inst, (i!=0));

            // Hash the input share of the current party
            commit_share(share_commitments[p], share, ssig->salt, (uint16_t) e, (uint16_t) i);
        }

        // Compute the root of the Merkle tree for the current execution
        ret = get_merkle_root_from_auth(merkle_roots[e],
            PARAM_LOG_NB_PARTIES, PARAM_NB_PARTIES, PARAM_NB_REVEALED, // Tree Parameters
            open_views[e], (uint8_t*) share_commitments, // Open Leaves
            ssig->proofs[e].cv_info, ssig->proofs[e].len_cv_info, // Authentication Paths
            NULL // salt (if provided)
        );
        if (ret != 0) {
            ret = -1;
            goto Exit;
        }
    }
    free(share);
    free(sh_broadcast);

    // Recompute the hash digest of the MPC challenge
    //    and check it is consistent with the one in the signature
    uint8_t mpc_challenge_hash[PARAM_DIGEST_SIZE];
    hash_for_mpc_challenge(mpc_challenge_hash, merkle_roots, ppk.inst, ssig->salt, NULL, 0);
    ret = (memcmp(mpc_challenge_hash, ssig->mpc_challenge_hash, PARAM_DIGEST_SIZE) != 0);

Exit:
    free(mpc_challenge);
    sdith_free_keys_internal(&ppk, NULL);
    free_const_signature(ssig);
    return ret;
}

/***********************************************
 *             About Serialization             *
 ***********************************************/

void free_signature(signature_threshold_nfpr_t* sig) {
    if(sig->allocated_memory != NULL)
        free(sig->allocated_memory);
    free(sig);
}

void free_const_signature(const_signature_threshold_nfpr_t* sig) {
    if(sig->allocated_memory != NULL)
        free(sig->allocated_memory);
    free(sig);
}

const_signature_threshold_nfpr_t* parse_signature(const uint8_t* buf, size_t buflen, uint16_t open_views[PARAM_NB_EXECUTIONS][PARAM_NB_REVEALED], const uint8_t* m, size_t mlen) {
    size_t bytes_required = PARAM_DIGEST_SIZE + PARAM_SALT_SIZE;
    bytes_required += PARAM_COMPRESSED_BR_SIZE;
    bytes_required += PARAM_NB_EXECUTIONS*PARAM_NB_REVEALED*PARAM_BR_SHORT_SIZE;
    bytes_required += PARAM_NB_EXECUTIONS*PARAM_NB_REVEALED*PARAM_WIT_SHORT_SIZE;
    if(buflen < bytes_required) {
        // Buffer too short
        return NULL;
    }

    const_signature_threshold_nfpr_t* sig = malloc(sizeof(const_signature_threshold_nfpr_t));
    sig->salt = buf;                buf += PARAM_SALT_SIZE;
    sig->mpc_challenge_hash = buf;    buf += PARAM_DIGEST_SIZE;

    const mpc_broadcast_t* broadcast[PARAM_NB_EXECUTIONS][PARAM_NB_REVEALED];
    sig->allocated_memory = malloc(
        PARAM_BR_SIZE
         + PARAM_NB_EXECUTIONS*PARAM_NB_REVEALED*(PARAM_BR_PARSED_SIZE+PARAM_WIT_PARSED_SIZE)
    );
    uint8_t* buf_ = sig->allocated_memory;
    // Plain Broadcast
    mpc_broadcast_t* ptr_plain_broadcast = (mpc_broadcast_t*) buf_;
    uncompress_plain_broadcast(ptr_plain_broadcast, buf);
    sig->plain_broadcast = ptr_plain_broadcast;
    buf_ += PARAM_BR_SIZE;
    buf += PARAM_COMPRESSED_BR_SIZE;
    for(size_t e=0; e<PARAM_NB_EXECUTIONS; e++) {
        for(size_t p=0; p<PARAM_NB_REVEALED; p++) {
            // Broadcast
            br_deserialize(&sig->proofs[e].broadcast[p], buf, buf_);
            buf += PARAM_BR_SHORT_SIZE;
            buf_ += PARAM_BR_PARSED_SIZE;
            broadcast[e][p] = sig->proofs[e].broadcast[p];

            // Witness share
            wit_deserialize(&sig->proofs[e].wit[p], buf, buf_);
            buf += PARAM_WIT_SHORT_SIZE;
            buf_ += PARAM_WIT_PARSED_SIZE;
        }
    }

    uint8_t challenge_views_hash[PARAM_DIGEST_SIZE];
    hash_for_view_challenge(challenge_views_hash, sig->mpc_challenge_hash, broadcast, sig->plain_broadcast, sig->salt, m, mlen);
    expand_view_challenge_hash((uint16_t*) open_views, challenge_views_hash, PARAM_NB_EXECUTIONS, PARAM_NB_REVEALED);

    for(size_t e=0; e < PARAM_NB_EXECUTIONS; e++) {
        sig->proofs[e].len_cv_info = get_auth_size(PARAM_LOG_NB_PARTIES, PARAM_NB_PARTIES, PARAM_NB_REVEALED, open_views[e]);
        bytes_required += sig->proofs[e].len_cv_info;
    }

    /* Fail if the signature does not have the exact number of bytes we expect */
    if(buflen != bytes_required) {
        free(sig->allocated_memory);
        free(sig);
        return NULL;
    }

    for (size_t e=0; e<PARAM_NB_EXECUTIONS; e++) {
        sig->proofs[e].cv_info = buf;
        buf += sig->proofs[e].len_cv_info;
    }

    return sig;
}

signature_threshold_nfpr_t* init_signature_structure(const uint8_t* salt, uint8_t* buf, size_t buflen) {
    size_t bytes_required = PARAM_DIGEST_SIZE + PARAM_SALT_SIZE;
    if(buflen < bytes_required) {
        return NULL;
    }

    signature_threshold_nfpr_t* sig = malloc(sizeof(signature_threshold_nfpr_t));
    sig->salt = buf;               buf += PARAM_SALT_SIZE;
    sig->mpc_challenge_hash = buf;   buf += PARAM_DIGEST_SIZE;
    memcpy(sig->salt, salt, PARAM_SALT_SIZE);

    sig->allocated_memory = malloc(
        PARAM_DIGEST_SIZE
         + PARAM_BR_SIZE // plain broadcast
         + PARAM_NB_EXECUTIONS*(
            PARAM_TREE_NB_MAX_OPEN_LEAVES*PARAM_DIGEST_SIZE // merkle tree
             + PARAM_NB_REVEALED*PARAM_BR_SIZE // broadcast
             + PARAM_NB_REVEALED*PARAM_WIT_SIZE // witness share
        )
    );
    uint8_t* buf_ = sig->allocated_memory;
    sig->plain_broadcast = (void*) buf_;
    buf_ += PARAM_BR_SIZE;
    for(size_t e=0; e<PARAM_NB_EXECUTIONS; e++) {
        sig->proofs[e].cv_info = buf_;
        buf_ += PARAM_TREE_NB_MAX_OPEN_LEAVES*PARAM_DIGEST_SIZE;
        for(size_t p=0; p<PARAM_NB_REVEALED; p++) {
            sig->proofs[e].wit[p] = (void*) buf_;
            buf_ += PARAM_WIT_SIZE;
            sig->proofs[e].broadcast[p] = (void*) buf_;
            buf_ += PARAM_BR_SIZE;
        }
    }
    return sig;
}

int build_signature(const signature_threshold_nfpr_t* sig, uint8_t* buf, size_t buflen) {
    size_t bytes_required = PARAM_DIGEST_SIZE + PARAM_SALT_SIZE;
    bytes_required += PARAM_COMPRESSED_BR_SIZE;
    bytes_required += PARAM_NB_EXECUTIONS*PARAM_NB_REVEALED*PARAM_BR_SHORT_SIZE;
    bytes_required += PARAM_NB_EXECUTIONS*PARAM_NB_REVEALED*PARAM_WIT_SHORT_SIZE;
    for(size_t e=0; e<PARAM_NB_EXECUTIONS; e++) {
        bytes_required += sig->proofs[e].len_cv_info;
    }
    if(buflen < bytes_required)
        return -1;

    uint8_t* bufBase = buf;
    buf += (PARAM_DIGEST_SIZE + PARAM_SALT_SIZE); // salt and mpc challenge hash are already set (via init)

    // warning: must be in the same order than in parsing
    compress_plain_broadcast(buf, sig->plain_broadcast);
    buf += PARAM_COMPRESSED_BR_SIZE;
    for(size_t e=0; e<PARAM_NB_EXECUTIONS; e++) {
        for(size_t p=0; p<PARAM_NB_REVEALED; p++) {
            br_serialize(buf, sig->proofs[e].broadcast[p]);
            buf += PARAM_BR_SHORT_SIZE;
            wit_serialize(buf, sig->proofs[e].wit[p]);
            buf += PARAM_WIT_SHORT_SIZE;
        }
    }
    for(size_t e=0; e<PARAM_NB_EXECUTIONS; e++) {
        memcpy(buf, sig->proofs[e].cv_info, sig->proofs[e].len_cv_info);
        buf += sig->proofs[e].len_cv_info;
    }

    return (int)(buf - bufBase);
}
