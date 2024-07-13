#include "witness.h"
#include "poly.h"
#include "hash.h"
#include <stdio.h>
#include <stdlib.h>

extern uint8_t f_poly[];
extern uint8_t leading_coefficients_of_lj_for_S[];

#define DEGREE_S (PARAM_CHUNK_LENGTH-1)
#define DEGREE_Q (PARAM_CHUNK_WEIGHT)
#define DEGREE_REDUCED_Q (PARAM_CHUNK_WEIGHT-1)
#define DEGREE_F (PARAM_CHUNK_LENGTH)
#define DEGREE_P (PARAM_CHUNK_WEIGHT-1)

static void expand_extended_witness(samplable_t* entropy, uint8_t s_poly[PARAM_SPLITTING_FACTOR][PARAM_CHUNK_LENGTH], uint8_t q_poly[PARAM_SPLITTING_FACTOR][PARAM_CHUNK_WEIGHT], uint8_t p_poly[PARAM_SPLITTING_FACTOR][PARAM_CHUNK_WEIGHT]) {
    uint8_t tmp_poly[PARAM_CHUNK_LENGTH]; // deg = m-1
    uint32_t i, j;

    uint8_t positions[PARAM_CHUNK_WEIGHT];
    uint8_t non_zero_coordinates[PARAM_CHUNK_WEIGHT];

    for(size_t npoly=0; npoly<PARAM_SPLITTING_FACTOR; npoly++) {
        // First, let us compute a list of 'w' positions (without redundancy)
        i = 0;
        while(i<PARAM_CHUNK_WEIGHT) {
            byte_sample(entropy, &positions[i], 1);
            if(positions[i] >= PARAM_CHUNK_LENGTH)
                continue;
            char is_redundant = 0;
            for(j=0; j<i; j++)
                if(positions[i] == positions[j])
                    is_redundant = 1;
            if( !is_redundant )
                i++;
        }

        // Then, compute the non-zero evaluations of S 
        for(i=0; i<PARAM_CHUNK_WEIGHT; i++) {
            random_points(&non_zero_coordinates[i], 1, entropy);
            while(non_zero_coordinates[i] == 0)
                random_points(&non_zero_coordinates[i], 1, entropy);
        }

        // Compute x
        uint8_t x_vector[PARAM_CHUNK_LENGTH] = {0};
        for(i=0; i<PARAM_CHUNK_LENGTH; i++)
            for(j=0; j<PARAM_CHUNK_WEIGHT; j++)
                x_vector[i] ^= non_zero_coordinates[j]*(positions[j] == i);

        // Compute Q
        // Q <- 1
        if(q_poly != NULL) {
            //set_polynomial_as_zero(q_poly[npoly], DEGREE_Q);
            uint8_t* q_coeffs = q_poly[npoly];
            memset(q_coeffs, 1, PARAM_CHUNK_WEIGHT);
            for(i=0; i<PARAM_CHUNK_WEIGHT; i++) {
                // Q' <- Q · (X-w_i)
                uint8_t wi = positions[i];
                uint8_t minus_wi = sub_points(0,wi);
                //q_coeffs[i+1] = q_coeffs[i]; // No need anymore since the vector is filled of 1
                for(j=i; j>=1; j--)
                    q_coeffs[j] = add_points(q_coeffs[j-1], mul_points(minus_wi, q_coeffs[j]));
                q_coeffs[0] = mul_points(minus_wi, q_coeffs[0]);
            }
        }

        // Compute S and P
        set_polynomial_as_zero(s_poly[npoly], DEGREE_S);
        if(p_poly != NULL)
            set_polynomial_as_zero(p_poly[npoly], DEGREE_P);
        for(i=0; i<PARAM_CHUNK_LENGTH; i++) {
            uint8_t scalar = mul_points(x_vector[i], leading_coefficients_of_lj_for_S[i]);
            // For S
            remove_one_degree_factor_from_monic(tmp_poly, f_poly, DEGREE_F, (uint8_t) i);
            multiply_polynomial_by_scalar(tmp_poly, tmp_poly, DEGREE_S, scalar);
            add_polynomials(s_poly[npoly], tmp_poly, DEGREE_S);
            // For P
            if(p_poly != NULL) {
                remove_one_degree_factor_from_monic(tmp_poly, q_poly[npoly], DEGREE_Q, (uint8_t) i);
                multiply_polynomial_by_scalar(tmp_poly, tmp_poly, DEGREE_P, scalar);
                add_polynomials(p_poly[npoly], tmp_poly, DEGREE_P);
            }
        }
    }
}

void uncompress_instance(instance_t* inst) {
    if(inst->mat_H == NULL) {
        inst->mat_H = malloc(PARAM_PCMATRIX_BYTESIZE);
        prg_context entropy_ctx;
        samplable_t entropy = prg_to_samplable(&entropy_ctx);
        prg_init(&entropy_ctx, inst->seed_H, NULL);
        random_points(*inst->mat_H, PARAM_PCMATRIX_BYTESIZE, &entropy);
    }
}

void generate_instance_with_solution(instance_t** inst, solution_t** sol, samplable_t* entropy) {
    // Allocate
    *sol = (solution_t*) malloc(sizeof(solution_t));
    *inst = (instance_t*) malloc(sizeof(instance_t));
    (*inst)->mat_H = NULL;

    // Build the polynomial S, Q and P
    uint8_t s_poly[PARAM_SPLITTING_FACTOR][PARAM_CHUNK_LENGTH];
    uint8_t* s = (uint8_t*) s_poly;
    expand_extended_witness(entropy, s_poly, (*sol)->q_poly, (*sol)->p_poly);

    // Split s as (s_A | s_B)
    uint8_t* s_b = &s[PARAM_PLAINTEXT_LENGTH];
    memcpy((*sol)->s_A, s, PARAM_PLAINTEXT_LENGTH);

    // Sample a seed for matrix H
    byte_sample(entropy, (*inst)->seed_H, PARAM_SEED_SIZE);

    // Build H
    uncompress_instance(*inst);

    // Build y = s_B + H s_A
    memcpy((*inst)->y, s_b, PARAM_SYNDROME_LENGTH);
    matcols_muladd((*inst)->y, (*sol)->s_A, *(*inst)->mat_H, PARAM_PLAINTEXT_LENGTH, PARAM_SYNDROME_LENGTH);
}

void hash_update_instance(hash_context* ctx, const instance_t* inst) {
    hash_update(ctx, inst->seed_H, PARAM_SEED_SIZE);
    hash_update(ctx, inst->y, PARAM_SYNDROME_LENGTH);
}

void serialize_instance(uint8_t* buf, const instance_t* inst) {
    memcpy(buf, inst->seed_H, PARAM_SEED_SIZE);
    memcpy(buf + PARAM_SEED_SIZE, inst->y, PARAM_SYNDROME_LENGTH);
}

instance_t* deserialize_instance(const uint8_t* buf) {
    instance_t* inst = (instance_t*) malloc(sizeof(instance_t));
    inst->mat_H = NULL;
    memcpy(inst->seed_H, buf, PARAM_SEED_SIZE);
    memcpy(inst->y, buf + PARAM_SEED_SIZE, PARAM_SYNDROME_LENGTH);
    return inst;
}

void serialize_instance_solution(uint8_t* buf, const solution_t* sol) {
    memcpy(buf, sol->s_A, PARAM_PLAINTEXT_LENGTH);
    buf += PARAM_PLAINTEXT_LENGTH;
    for(size_t nchunk=0; nchunk<PARAM_SPLITTING_FACTOR; nchunk++) {
        memcpy(buf, sol->q_poly[nchunk], PARAM_CHUNK_WEIGHT);
        buf += PARAM_CHUNK_WEIGHT;
        memcpy(buf, sol->p_poly[nchunk], PARAM_CHUNK_WEIGHT);
        buf += PARAM_CHUNK_WEIGHT;
    }
}

solution_t* deserialize_instance_solution(const uint8_t* buf) {
    solution_t* sol = (solution_t*) malloc(sizeof(solution_t));
    memcpy(sol->s_A, buf, PARAM_PLAINTEXT_LENGTH);
    buf += PARAM_PLAINTEXT_LENGTH;
    for(size_t nchunk=0; nchunk<PARAM_SPLITTING_FACTOR; nchunk++) {
        memcpy(sol->q_poly[nchunk], buf, PARAM_CHUNK_WEIGHT);
        buf += PARAM_CHUNK_WEIGHT;
        memcpy(sol->p_poly[nchunk], buf, PARAM_CHUNK_WEIGHT);
        buf += PARAM_CHUNK_WEIGHT;
    }
    return sol;
}

void free_instance_solution(solution_t* sol) {
    free(sol);
}

void free_instance(instance_t* inst) {
    if(inst->mat_H)
        free(inst->mat_H);
    free(inst);
}

int are_same_instances(instance_t* inst1, instance_t* inst2) {
    int ret = 0;
    ret |= memcmp(inst1->seed_H, inst2->seed_H, PARAM_SEED_SIZE);
    ret |= memcmp(inst1->y, inst2->y, PARAM_SYNDROME_LENGTH);
    return ((ret) ? 0 : 1);
}

int is_correct_solution(instance_t* inst, solution_t* sol) {
    int ret = 0;
    uncompress_instance(inst);


    /// We recompute the vector s as (s_A||s_B),
    ///      where s_B := y - H' s_A

    // s = s_A || s_B
    uint8_t* s = (uint8_t*) aligned_alloc(32, PARAM_CODEWORD_LENGTH_CEIL32);
    if(s == NULL) {
        printf("Error: Aligned alloc returns NULL.\n");
        return 0;
    }
    memcpy(s, sol->s_A, PARAM_PLAINTEXT_LENGTH);
    memcpy(&s[PARAM_PLAINTEXT_LENGTH], inst->y, PARAM_SYNDROME_LENGTH);

    // s_B = y - H_A x_A
    neg_tab_points(&s[PARAM_PLAINTEXT_LENGTH], PARAM_SYNDROME_LENGTH);
    matcols_muladd(&s[PARAM_PLAINTEXT_LENGTH], sol->s_A, *inst->mat_H, PARAM_PLAINTEXT_LENGTH, PARAM_SYNDROME_LENGTH);
    neg_tab_points(&s[PARAM_PLAINTEXT_LENGTH], PARAM_SYNDROME_LENGTH);

    // We check if the precomputed polynomial F is well-computed
    for(uint32_t i=0; i<PARAM_CHUNK_LENGTH; i++) {
        uint8_t f_eval = evaluate_monic_polynomial(f_poly, DEGREE_F, (uint8_t) i);
        if(f_eval != 0) {
            printf("Error: Wrong F evaluation (%d, %d).\n", i, f_eval);
            ret = -1;
        }
    }

    // Interpret the vector s as a vector of polynomials
    uint8_t (*s_poly)[PARAM_CHUNK_LENGTH] = (void*) s;

    for(size_t h=0; h<PARAM_SPLITTING_FACTOR; h++) {
        uint8_t x_vector[PARAM_CHUNK_LENGTH] = {0};

        /// We check that the evaluations of the polynomial S forms
        ///     a vector of hamming weight equal to w
        int weight = 0;
        for(uint32_t i=0; i<PARAM_CHUNK_LENGTH; i++) {
            x_vector[i] = evaluate_polynomial(s_poly[h], DEGREE_S, (uint8_t) i);
            weight += (x_vector[i] != 0);
        }
        if(weight != PARAM_CHUNK_WEIGHT) {
            printf("Error: S does not form a vector with the right hamming weight (w=%d).\n", weight);
            ret = -1;
        }

        /// We check that an evaluation of Q is zero iff the corresponding
        ///    coordinate of x is zero
        for(uint32_t i=0; i<PARAM_CHUNK_LENGTH; i++) {
            uint8_t q_eval = evaluate_monic_polynomial(sol->q_poly[h], DEGREE_Q, (uint8_t) i);
            if((q_eval != 0) == (x_vector[i] != 0)) {
                printf("Error: Wrong Q evaluation (%d,%d,%d).\n", i, q_eval, x_vector[i]);
                ret = -1;
            }
        }

        /// To check that P is correct, we check that SQ-PF is equal to 0.
        ///     To proceed, we evaluate the polynomial equations in more than
        ///     deg(SQ-PF)+1 points.
        for(uint32_t i=0; i<2; i++) {
            for(uint32_t j=0; j<PARAM_CHUNK_LENGTH; j++) {
                uint8_t eval_point[PARAM_EXT_DEGREE] = {0};
                eval_point[1] = (uint8_t) i;
                eval_point[0] = (uint8_t) j;

                // Evaluate the four polynomials S, Q, P and F
                uint8_t s_eval[PARAM_EXT_DEGREE], q_eval[PARAM_EXT_DEGREE];
                uint8_t p_eval[PARAM_EXT_DEGREE], f_eval[PARAM_EXT_DEGREE];
                evaluate_polynomial_in_extfield(s_eval, s_poly[h], DEGREE_S, eval_point, PARAM_EXT_DEGREE);
                evaluate_monic_polynomial_in_extfield(q_eval, sol->q_poly[h], DEGREE_Q, eval_point, PARAM_EXT_DEGREE);
                evaluate_polynomial_in_extfield(p_eval, sol->p_poly[h], DEGREE_P, eval_point, PARAM_EXT_DEGREE);
                evaluate_monic_polynomial_in_extfield(f_eval, f_poly, DEGREE_F, eval_point, PARAM_EXT_DEGREE);

                // Check if SQ-PF is zero for the current evaluation point
                uint8_t sq_prod[PARAM_EXT_DEGREE], pf_prod[PARAM_EXT_DEGREE];
                mul_points_ext(sq_prod, s_eval, q_eval);
                mul_points_ext(pf_prod, p_eval, f_eval);
                if(eq_points_ext(sq_prod,pf_prod) != 1) {
                    printf("Error: SQ-PF is not zero for some points.\n");
                    ret = -1;
                }
            }
        }
    }

    free(s);
    return (ret == 0);
}
