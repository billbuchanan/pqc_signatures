#include "api.h"
#include "common.h"

void export_sk(unsigned char *sk,uint16_t *Q, uint16_t *part_perm1, uint16_t* part_perm2, matrix* Hrep){
	//export private in order: Q, part_perm1, pert_perm2, Hrep
	memcpy		(sk, 
					Q, sizeof(uint16_t)*CODE_N);
	memcpy		(sk+sizeof(uint16_t)*CODE_N, 
					part_perm1, sizeof(uint16_t)*CODE_N/4);
	memcpy		(sk+sizeof(uint16_t)*CODE_N+sizeof(uint16_t)*CODE_N/4, 
					part_perm2, sizeof(uint16_t)*CODE_N/4);
	export_matrix(Hrep, sk + sizeof(uint16_t)*CODE_N + (sizeof(uint16_t)*CODE_N/4)*2);
}

int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk){
	// nrows of Gm is set to K + 1 for public key
	matrix* Gm = new_matrix(CODE_K, CODE_N);
	matrix* Hm = new_matrix(CODE_N - CODE_K, CODE_N);

	matrix* Gpub = new_matrix(CODE_K + 1, CODE_N);
	matrix* Hpub = new_matrix(CODE_N - CODE_K - 1, CODE_N);

	uint16_t Q[CODE_N];
	
	uint16_t part_perm1[(CODE_N/4)];
	uint16_t part_perm2[(CODE_N/4)];

	// generate secret partial permutations
	partial_permutation_gen(part_perm1);
	partial_permutation_gen(part_perm2);
	
	// Generate a partially permute generator matrix Gm
	rm_gen(Gm, RM_R, RM_M, 0, CODE_K, 0, CODE_N);

	// replace of RM(r,r)
	matrix* Grep = new_matrix((1<<RM_R) - K_REP, (1<<RM_R));
	matrix* Hrep = new_matrix(K_REP, 1<<RM_R);

	uint8_t is_odd = 0;
	uint8_t randstr[Grep->nrows * Grep->colsize * 8];

	// check if Hrep has a odd row.
	while(1){
		randombytes(randstr, Grep->nrows * Grep->colsize * 8);
		import_matrix(Grep, randstr);
		
		dual(Grep, Hrep);
		for (uint32_t i = 0; i < Hrep->nrows; i++)
		{	
			uint8_t parity = 0;
			for (uint32_t j = 0; j < Hrep->ncols; j++)
			{
				parity ^= get_element(Hrep, i, j);
			}
			if (parity == 1){
				is_odd = 1;
				break;
			}
		}
		if(is_odd) break;
	}
	rref(Hrep);
	dual(Hrep, Grep);
	
	// replace the code (starting from second row)
	for (uint32_t i = 0; i < CODE_N; i += Grep->ncols)
	{
		partial_replace(Gm, K_REP, K_REP + Grep->nrows, i, i + Grep->ncols, Grep, 0, 0); 
	}
	
	// Partial permutation
	for (uint32_t i = 0; i < 4; ++i)
	{
		col_permute(Gm, 0, rm_dim[RM_R][RM_M -2], 
			i*(CODE_N/4),(i+1)*(CODE_N/4), part_perm1);
	}
	col_permute(Gm, CODE_K - rm_dim[RM_R-2][RM_M-2], CODE_K, 
		3*CODE_N/4, CODE_N, part_perm2);
	
	// two random rows (one of them has odd Hamming weight)
	uint8_t randstr_for_row[Gm->colsize * 8];
	matrix *random_row = new_matrix(1, Gm->ncols);
	for (uint32_t i = 0; i < 2; i++){
		// if i = 0 generate a row with Hamming weight
		if(i == 0){
			while(1){
				randombytes(randstr_for_row, Gm->colsize * 8);
				// check if odd
				uint8_t parity = 0;
				for (uint32_t j = 0; j < Gm->ncols / 8; j++)
				{
					parity ^= randstr_for_row[j];
				}
				parity ^= parity >> 4;
				parity ^= parity >> 2;
				parity ^= parity >> 1;
				// break when it is odd
				if (parity & 0x01){
					break;
				}
			}
		}
		else {
			randombytes(randstr_for_row, Gm->ncols / 8);
		}
		
		import_matrix(random_row, randstr_for_row);
		partial_replace(Gm, i, i+1, 0, Gm->ncols, random_row, 0, 0);
	}

	// Parity check matrix of the modified RM code
	dual(Gm, Hm);

	// pick a random codeword from the dual code
	matrix* code_from_dual = new_matrix(1, Hm->ncols);
	matrix* random_syndrome = new_matrix(1, Hm->nrows);
	uint8_t seed[(Hm->nrows + 7)/8];
	while(1){
		randombytes(seed, (Hm->nrows + 7)/8);
		codeword(Hm, seed, code_from_dual);
		
		vec_mat_prod(random_syndrome, Hm, code_from_dual);
		if(! is_zero(random_syndrome)){
			break;
		}
	}
	delete_matrix(random_syndrome);

	copy_matrix(Gpub, Gm);
	permutation_gen(Q, CODE_N);

	// Generate the dual code of Gm and the public key
	dual(Gpub, Hpub);
	matrix* Hcpy = new_matrix(Hpub->nrows, Hpub->ncols); 
	copy_matrix(Hcpy, Hpub);

	col_permute(Hcpy, 0, Hcpy->nrows, 0, Hcpy->ncols, Q);
	rref(Hcpy);

	uint16_t pivot[Hcpy->nrows];
	uint16_t d_pivot[Hcpy->ncols - Hcpy->nrows];
	get_pivot(Hcpy, pivot, d_pivot);

	for (uint32_t i = 0; i < Hcpy->nrows; i++)
	{
		if(pivot[i] != i){
			uint16_t tmp = Q[i];
			Q[i] = Q[pivot[i]];
			Q[pivot[i]] = tmp;
		}		
	}
	
	col_permute(Hpub, 0, Hpub->nrows, 0, Hpub->ncols, Q);
	rref(Hpub);

	matrix* non_identity_hpub = new_matrix_with_pool(Hpub->nrows, Hpub->ncols - Hpub->nrows, pk);
	init_zero(non_identity_hpub);
    partial_replace(non_identity_hpub, 0, non_identity_hpub->nrows, 
        0, non_identity_hpub->ncols, 
        Hpub, 0, Hpub->nrows);

    export_sk(sk, Q, part_perm1, part_perm2, Hrep);

	delete_matrix(Gm);
	delete_matrix(Hm);
	delete_matrix(Gpub);
	delete_matrix(Hpub);
	delete_matrix(Grep);
	delete_matrix(Hrep);
	delete_matrix(code_from_dual);
	delete_matrix(Hcpy);
	delete_matrix(random_row);
	delete_matrix(non_identity_hpub);

	return 0;
}
