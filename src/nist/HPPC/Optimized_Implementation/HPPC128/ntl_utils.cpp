#include <NTL/GF2.h>
#include <NTL/GF2X.h>
#include <NTL/GF2EX.h>
#include <NTL/GF2EXFactoring.h>
#include <NTL/GF2XFactoring.h>

#include "ntl_utils.h"

NTL::GF2X irred_poly;

void InitNTL(int n, fq_ctx_t ctx, unsigned long seed){
    // must copy ctx modulus to build NTL modulus
    fmpz_mod_ctx_t mctx;
    fmpz_t p, coeff;
    fmpz_init(coeff);
    fmpz_init(p);
    fmpz_set_ui(p, 2);
    fmpz_mod_ctx_init(mctx, p);
    for (int i = 0; i <= n; i++){ // deg is n so n+1 tuple
        fmpz_mod_poly_get_coeff_fmpz(coeff, ctx->modulus, i, mctx);
        if (fmpz_get_ui(coeff) == 1)
            NTL::SetCoeff(irred_poly, i, 1);
    }
    
    NTL::GF2E::init(irred_poly);
    NTL::SetSeed(NTL::conv<NTL::ZZ>((long) seed));

    fmpz_clear(p);
    fmpz_clear(coeff);
    fmpz_mod_ctx_clear(mctx);
}


void ConvertPrivatePolynomial(NTL::GF2EX& private_polynomial, mzd_t *_private_polynomial, int degree, int n, fq_ctx_t ctx ){
    for (int i = 0; i <= degree; i++){
        long exp = powl(2,i)+1;
        NTL::GF2X ntl_elem;
        for (int j = 0; j < n; j++){
            if (mzd_read_bit(_private_polynomial, i*n + j, 0) == 1)
                NTL::SetCoeff(ntl_elem, j, 1);
        }
        NTL::SetCoeff(private_polynomial, exp, NTL::conv<NTL::GF2E>(ntl_elem));
    }
}

void TraceMap(NTL::GF2EX& h, const NTL::GF2EX& a, const NTL::GF2EXModulus& F)
{
    //helper for FindRootTimeout()

   NTL::GF2EX res, tmp;

   res = a;
   tmp = a;

   long i;
   for (i = 0; i < NTL::GF2E::degree()-1; i++) {
      NTL::SqrMod(tmp, tmp, F);
      NTL::add(res, res, tmp);
   }

   h = res;
}

bool FindRootTimeout(NTL::GF2E& root, const NTL::GF2EX& ff){

    // Overwrite FindRoot as NTL's native never returns when no root is found for P(X) - Y = 0
    // Then define this new one that detects when the degree of F(X) doesn't change, this implies no solution


    NTL::GF2EXModulus F;
    NTL::GF2EX h, h1, f;
    NTL::GF2E r;

    f = ff;

    if (!NTL::IsOne(LeadCoeff(f)))
        NTL::LogicError("FindRoot: bad args");

    if (NTL::deg(f) == 0)
        NTL::LogicError("FindRoot: bad args");

    long cur_deg = 0;
    bool success = false;
    while (deg(f) > 1 && (success = (cur_deg != deg(f)))) {
        cur_deg = deg(f);
        NTL::build(F, f);
        NTL::random(r);
        NTL::clear(h);
        NTL::SetCoeff(h, 1, r);
        TraceMap(h, h, F);
        NTL::GCD(h, h, f);
        if (NTL::deg(h) > 0 && NTL::deg(h) < NTL::deg(f)) {
            if (NTL::deg(h) > deg(f)/2)
                NTL::div(f, f, h);
            else
                f = h;
        }
    }

    root = NTL::ConstTerm(f);

    return success;
}

bool RootPrivatePoly(mzd_t *_private_polynomial, mzd_t *root, mzd_t *_const_term, int n, int degree, fq_ctx_t ctx){

    // Use NTL here as FLINT has a big bottleneck for finding roots of a polynomial P(X) over Fq^n
    // On FLINT you can split a single linear factor of P(X) or find all roots. Both ways are slow for at least GF(2^128)
    // NTL finds a single root using TraceMap/GCD based on Berlekamp's Trace algorithm which is efficient on binary extensions fields

    NTL::GF2X const_term;
    NTL::GF2EX private_polynomial;

    ConvertPrivatePolynomial(private_polynomial, _private_polynomial, degree, n, ctx);
    
    for (int i = 0; i < n; i++){
        if (mzd_read_bit(_const_term, i, 0) == 1)
            NTL::SetCoeff(const_term, i, 1);
    }

    //std::cout << const_term << std::endl;
    
    NTL::GF2E a = NTL::conv<NTL::GF2E>(const_term);
    NTL::GF2EX F = private_polynomial - a;

    NTL::GF2E _root;

    if (!FindRootTimeout(_root, F)) {return false;}

    NTL::vec_GF2 root_vec =  NTL::to_vec_GF2(NTL::conv<NTL::GF2X>(_root));
    for (int i = 0; i < root_vec.length(); i++){
        if (root_vec[i]==1)
            mzd_write_bit(root, i, 0, 1);
    }

    return true;
}