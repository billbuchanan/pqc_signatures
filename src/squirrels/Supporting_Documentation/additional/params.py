import numpy as np
from sage.all import *
from itertools import product
import tqdm

# This Python file uses the following encoding: utf-8

"""
This script computes parameters and security estimates for Squirrels.
It is adapted from the script used in [Falcon20].

References:
- [BDGL16]: ia.cr/2015/1128
- [Duc18]:ia.cr/2017/999
- [Falcon20]: https://falcon-sign.info
- [HPRR20]: ia.cr/2019/1411
- [Laa16]: https://pure.tue.nl/ws/files/14673128/20160216_Laarhoven.pdf
- [Lyu12]: ia.cr/2011/537
- [MR07]: https://cims.nyu.edu/~regev/papers/average.pdf
- [MW16]: ia.cr/2015/1123
- [NIST]: https://csrc.nist.gov/CSRC/media/Projects/Post-Quantum-Cryptography
          /documents/call-for-proposals-final-dec-2016.pdf
- [Pre17]: ia.cr/2017/480
"""
#from Crypto.Util.number import isPrime
from math import sqrt, exp, log, pi, floor, log, erf

import sys
# For debugging purposes
if sys.version_info >= (3, 4):
    from importlib import reload  # Python 3.4+ only.

# This is the maximal acceptable standard deviation for
# the individual Gaussians over Z, lifted from [HPRR20]
sigmax = 1.8205

def smooth(eps, n, normalized=True):
    """
    Compute the smoothing parameter eta_epsilon(Z^n).
    - if normalized is True, take the definition from [Pre17,Falcon]
    - if normalized is False, take the definition from [MR07]
    """
    rep = sqrt(log(2 * n * (1 + 1 / eps)) / pi)
    if normalized is True:
        return rep / sqrt(2 * pi)
    else:
        return rep

def error_rounding_zn(min_gs_norm, dim, proba_reject):
    rng = np.random.Generator(np.random.PCG64(0))
    nbsamples = 5000
    measures = []
    for _ in range(nbsamples):
        x = rng.uniform(low=-1/2, high=1/2, size=dim)
        basis_vect = np.zeros(dim)
        basis_vect[0] = min_gs_norm

        measures.append(abs(np.linalg.norm(basis_vect+x)-min_gs_norm))
    measures.sort()

    return measures[ceil(nbsamples*(1-proba_reject))]


def bound_rounding_zn_in_dim(gs_norm, dim, proba_accept):
    rng = np.random.Generator(np.random.PCG64(0))
    nbsamples = 5000
    measures = []
    for _ in range(nbsamples):
        x = rng.uniform(low=-1/2, high=1/2, size=dim)
        basis_vect = np.zeros(dim)
        basis_vect[0] = gs_norm

        measures.append(np.linalg.norm(basis_vect+x)-gs_norm)
    measures.sort()

    return (measures[floor(nbsamples*(1-proba_accept))], measures[ceil(nbsamples*proba_accept)])

"""
Bound rounding error for a given norm and dimension.
"""
def bound_rounding_zn(gs_norm, dim, proba_accept):
    l1, u1 = bound_rounding_zn_in_dim(gs_norm, dim, proba_accept)
    l2, u2 = bound_rounding_zn_in_dim(gs_norm, 1, proba_accept)

    return min([l1, l2, 0]), max([u1, u2])

def dimensionsforfree(B):
    """
    d in [Duc18].
    """
    return round(B * log(4 / 3) / log(B / (2 * pi * exp(1))))

REP_AVG = 500
def encode_one_coefficient(coef, rate):
    head,tail,sign = "","",""
    enc = '#0'+str(rate+2)+'b'
    sign += "1" if coef > 0 else "0"
    tail += format((abs(coef) % (1 << rate)), enc)[:1:-1]
    head += "0"*(abs(coef) >> rate)+"1"
    return len(sign)+len(tail)+len(head)

def average_size_falcon(D, rate):
    rng = np.random.Generator(np.random.PCG64(0))

    return sum([sum([encode_one_coefficient(coeff, rate) for coeff in D(rng)]) for i in range(REP_AVG)])/float(REP_AVG)

def int_vec_sampler(target_norm, dim, rng):
    v = rng.normal(0, 1, dim)
    v /= np.linalg.norm(v)
    v *= target_norm

    vround = np.round(v).astype(int)
    return vround


def log_binom_approx(n, k):
    # approx (n, alpha*n)
    # see https://math.stackexchange.com/a/1439477
    alpha = k/n
    
    return n*(alpha*log(1/alpha, 2)+(1-alpha)*log(1/(1-alpha), 2))-1/2*log(2*pi*n*alpha*(1-alpha), 2)

def keyrec_condition(B, dim, rt_covolume, sigma_fg):
    return (B / (2 * pi * e)) ** (1 - dim / (2*B)) * rt_covolume <= sqrt(3 * B / 4) * sigma_fg

def forgery_condition_mitaka(bkz_blocksize, max_sign_norm, gs_norm, dim):
    B = bkz_blocksize
    n = floor(dim/2)

    m = min([(((pi*B)**(1/B)*B/(2 * pi * e)) ** ((2*n-k)/(2*B-2))) * \
            gs_norm**(2*n/(2*n-k)) for k in range(n)])

    return m > max_sign_norm

"""
Search the first False value given by a decreasing boolean function.
"""
def dichotomy(f, min_B, max_B):
    vmin = f(min_B)

    while vmin != False and min_B < max_B:
        mid = (min_B+max_B)//2
        vmid = f(mid)

        if vmid:
            min_B = mid+1
            vmin = f(min_B)
        else:
            max_B = mid

    return min_B


class SquirrelsParam:
    """
    This class stores an object with all the parameters for Squirrels (adapted from Falcon codebase).
    """

    def __init__(self, min_sampling_gs_norm, max_sampling_gs_norm, dim, q, target_bitsec, fail_early_secu=None):
        """
        Initialize a SquirrelsParam object

        Input:
        - bounds on gram-schmidt sampling norms min_sampling_gs_norm and max_sampling_gs_norm
        - the lattice dimension dim
        - the hash size q
        - a target bit-security target_bitsec

        Output:
        - a SquirrelsParam object with:
          - the lattice dimension n
          - the hash size q
          - bounds on gram-schmidt sampling norms, and on norms after rounding
          - a target determinant and its factors
          - the signature standard deviation sigma
          - the tailcut rate and rejection rate
          - For key-recovery and forgery:
            - the required BKZ blocksize
            - the classical Core-SVP hardness
            - the quantum Core-SVP hardness
        """

        self.fail_early_secu = fail_early_secu

        self.dim = dim
        assert(self.dim % 2 == 0) # required in the code to sample a normal distribution

        # The maximal number of queries is limited to 2 ** 64 as per [NIST]
        self.nb_queries = 2 ** 64

        # We want q to be a power of two to ease implementing a constant time
        # HashToPoint function
        self.q = q
        assert(q & (q - 1) == 0)

        self.target_det_fact = None
        
        # Bounds on the norms of the continuous candidates sampled in key generation
        # when sampling vectors sequentially in the orthogonal of the previous vectors
        self.min_sampling_gs_norm = min_sampling_gs_norm     
        self.max_sampling_gs_norm = max_sampling_gs_norm
        
        proba_accept_rounding = 0.5
        (low_err_gs_min, _) = bound_rounding_zn(self.min_sampling_gs_norm, self.dim, proba_accept_rounding)
        (_, up_err_gs_max) = bound_rounding_zn(self.max_sampling_gs_norm, self.dim, proba_accept_rounding)        

        self.min_gs_norm = min_sampling_gs_norm + low_err_gs_min
        self.max_gs_norm = max_sampling_gs_norm + up_err_gs_max

        # Make sure that we have enough margin to correct the drift
        # We want the rounding error to be low enough after we sampled two thirds of the vectors
        # so that we are confident we can sample large or small vectors as we wish to correct the drift
        b_up = (self.max_sampling_gs_norm+3*self.min_sampling_gs_norm)/4
        b_down = (3*self.max_sampling_gs_norm+self.min_sampling_gs_norm)/4
        proba_accept_rounding = 0.9

        (_, mid_up_err_gs_min) = bound_rounding_zn(b_up, self.dim//3, proba_accept_rounding)
        (mid_low_err_gs_max, _) = bound_rounding_zn(b_down, self.dim//3, proba_accept_rounding)
        
        min_root_det = b_up + mid_up_err_gs_min
        max_root_det = self.max_sampling_gs_norm - 0.5 # we force some up margin
        assert(min_root_det < max_root_det)
        
        # rt_covolume is the n-th root of the target covolume/determinant of the sampled lattices
        self.rt_covolume = (min_root_det+max_root_det)/2
                
        # sigma is the standard deviation of the signatures:
        # - On one hand, we require sigma small to make forgery harder.
        # - On the other hand, we want sigma large so that the signatures'
        #   distribution is indistinguishable from an ideal Gaussian.
        # We set sigma according to [Pre17], so that we lose
        # O(1) bits of security compared to the ideal case.
        self.eps = 1 / sqrt(target_bitsec * self.nb_queries)
        self.smoothz2n = smooth(self.eps, self.dim, normalized=True)
        self.sigma = self.smoothz2n * self.max_gs_norm

        # sigmin and sigmax are the minimum and maximum standard deviations
        # for the Gaussian sampler over the *integers* SamplerZ.
        self.sigmin = self.sigma / self.max_gs_norm
        # sigmax is hardcoded, but it is important for security and correctness
        # that Falcon never calls SamplerZ with a standard deviation > sigmax
        self.sigmax = sigmax
        sigmax_in_practice = self.sigmin * self.max_gs_norm / self.min_gs_norm
        assert(sigmax_in_practice <= self.sigmax)

        # The tailcut rate intervenes during the signing procedure.
        # The expected value of the signature norm is sigma * sqrt(n).
        # If the signature norm is larger than its expected value by more than
        # a factor tailcut_rate, it is rejected and the procedure restarts.
        # The max squared signature norm is called ⌊\beta^2⌋ in our paper.
        # The rejection rate is given by [Lyu12, Lemma 4.4]. 
        self.tailcut_rate = 1.1
        tau = self.tailcut_rate
        m = self.dim
        aux = tau * sqrt(m) * self.sigma
        self.max_sig_norm = floor(aux)
        self.sq_max_sig_norm = floor(aux ** 2)
        self.rejection_rate = (tau ** m) * exp(m * (1 - tau ** 2) / 2)

        # Security metrics
        # This is the targeted bit-security.
        self.target_bitsec = target_bitsec

        # We compute the BKZ blocksize necessary to recovery the key.
        # This is the minimal blocksize B such that the left term in
        # equation from Key Recovery Attack is larger than the right term.
        B = 100
        e = exp(1)
        sigma_fg = self.min_gs_norm / sqrt(self.dim)
        B = dichotomy(lambda B: keyrec_condition(B, self.dim, self.rt_covolume, sigma_fg), 100, self.dim)
    
        # This is the smallest B which suffices to recover the private key.
        self.keyrec_blocksize = B
        self.keyrec_blocksize_opt = B - dimensionsforfree(B)
        # We deduce the classic and quantum CoreSVP security for key recovery.
        # Constants are lifted from [BDGL16] and [Laa16].
        self.keyrec_coresvp_c = floor(self.keyrec_blocksize * 0.292)
        self.keyrec_coresvp_q = floor(self.keyrec_blocksize * 0.265)
        self.keyrec_coresvp_opt_c = floor(self.keyrec_blocksize_opt * 0.292)
        self.keyrec_coresvp_opt_q = floor(self.keyrec_blocksize_opt * 0.265)
        
        if self.fail_early_secu[0] != None:
            assert(self.keyrec_coresvp_c >= self.fail_early_secu[0])
        if self.fail_early_secu[1] != None:
            assert(self.keyrec_coresvp_q >= self.fail_early_secu[1])

        # Evaluate hybrid keyrec attack

        def phi(x):
            # Cumulative distribution function for the standard normal distribution
            return (1.0 + erf(x / sqrt(2.0))) / 2.0

        # Compute sparsity level of vectors
        k = ceil(self.dim*(phi(0.5/sigma_fg) - phi(-0.5/sigma_fg)))
        
        # param g, B, complexity
        min_hybrid_complexity_c = (0, self.keyrec_blocksize, self.keyrec_coresvp_c)
        min_hybrid_complexity_q = (0, self.keyrec_blocksize, self.keyrec_coresvp_q)
        for g in range(1, k):
            B = dichotomy(lambda B: keyrec_condition(B, dim-g, self.rt_covolume**(dim/(dim-g)), sigma_fg), 100, dim-g)

            # proba selecting correct subset of zeros
            log_probasuccess = log_binom_approx(k, g) + log(dim, 2) - log_binom_approx(dim, g)
            log_probasuccess = min(0, log_probasuccess)

            complexity_c = floor(B * 0.292 - log_probasuccess)
            if complexity_c < min_hybrid_complexity_c[2]:
                min_hybrid_complexity_c = (g, B, complexity_c)
                
            complexity_q = floor(B * 0.265 - log_probasuccess)
            if complexity_q < min_hybrid_complexity_q[2]:
                min_hybrid_complexity_q = (g, B, complexity_q)
        
        self.keyrec_hybrid_c = min_hybrid_complexity_c
        self.keyrec_hybrid_q = min_hybrid_complexity_q

        if self.fail_early_secu[0] != None:
            assert(self.keyrec_hybrid_c[2] >= self.fail_early_secu[0])
        if self.fail_early_secu[1] != None:
            assert(self.keyrec_hybrid_q[2] >= self.fail_early_secu[1])

        # We compute the BKZ blocksize necessary to forge a signature.
        # This is done by embedding the signature in the lattice and applying
        # DBKZ. Subsequently, we apply [MW16, Corollary 1] with:
        # - k in the paper => B in this script
        # - n in the paper => n in our case
        # - sqrt[n]{det(B)} in the paper => rt_covolume in our case
        # This gives the formule used to control the while loop below.
        B = dichotomy(lambda B: ((B / (2 * pi * e)) ** (self.dim / (2*B))) * self.rt_covolume > self.max_sig_norm, 100, self.dim)

        # This is the smallest B which suffices to break EF-CMA security.
        self.forgery_blocksize = B
        self.forgery_blocksize_opt = B - dimensionsforfree(B)
        # We deduce the classic and quantum Core-SVP security for key recovery.
        # Constants are lifted from [BDGL16] and [Laa16].
        self.forgery_coresvp_c = floor(self.forgery_blocksize * 0.292)
        self.forgery_coresvp_q = floor(self.forgery_blocksize * 0.265)
        self.forgery_coresvp_opt_c = floor(self.forgery_blocksize_opt * 0.292)
        self.forgery_coresvp_opt_q = floor(self.forgery_blocksize_opt * 0.265)
        
        if self.fail_early_secu[0] != None:
            assert(self.forgery_coresvp_c >= self.fail_early_secu[0])
        if self.fail_early_secu[1] != None:
            assert(self.forgery_coresvp_q >= self.fail_early_secu[1])

        # We compute the BKZ blocksize necessary to forge a signature. Forgery attack from Mitaka: a simpler, parallelizable, maskable variant of Falcon
        B = dichotomy(lambda B: forgery_condition_mitaka(B, self.max_sig_norm, self.rt_covolume, self.dim), 100, self.dim)

        self.forgery2_blocksize = B
        self.forgery2_blocksize_opt = B - dimensionsforfree(B)
        # We deduce the classic and quantum Core-SVP security for key recovery.
        # Constants are lifted from [BDGL16] and [Laa16].
        self.forgery2_coresvp_c = floor(self.forgery2_blocksize * 0.292)
        self.forgery2_coresvp_q = floor(self.forgery2_blocksize * 0.265)
        self.forgery2_coresvp_opt_c = floor(self.forgery2_blocksize_opt * 0.292)
        self.forgery2_coresvp_opt_q = floor(self.forgery2_blocksize_opt * 0.265)

        if self.fail_early_secu[0] != None:
            assert(self.forgery2_coresvp_c >= self.fail_early_secu[0])
        if self.fail_early_secu[1] != None:
            assert(self.forgery2_coresvp_q >= self.fail_early_secu[1])
        
        self.th_sig_bytesize = ceil(log(self.max_sig_norm, 2)*self.dim/8)
        self.sig_bytesize = self.th_sig_bytesize
        self.sig_bytesize_rate = 0
        for rate in range(3, 8):
            new_size = ceil(average_size_falcon(lambda rng: int_vec_sampler(ceil(sqrt(m) * self.sigma), self.dim, rng), rate)/8)
            if new_size < self.sig_bytesize:
                self.sig_bytesize = new_size
                self.sig_bytesize_rate = rate

        self.sig_bytesize += 41 # store signature header and salt

    def compute_det_and_pksize(self):
        with seed(0):
            while True: # restart until we have a low error
                target_det = 1
                target_det_fact = []
                log_target_det = 0
                aim = self.dim*log(self.rt_covolume)

                while log_target_det < aim:
                    p = random_prime(1 << 31, True, (1 << 30) + 1)

                    target_det *= p
                    target_det_fact.append(p)
                    log_target_det += log(p)
                err = aim - log_target_det

                if len(target_det_fact) != len(set(target_det_fact)): # all unique
                    continue

                if abs(err) < 0.3:
                    break
            
            target_det_fact.sort()
            self.target_det_fact = target_det_fact
            self.log_det = sum([log(p) for p in target_det_fact])
            
        self.pubkey_size = (self.dim-1) * 32 * len(self.target_det_fact)/8
        self.seckey_size = self.dim*self.dim*(8+4) # one integer+double per coefficient
        
    def __repr__(self):
        if self.target_det_fact == None:
            self.compute_det_and_pksize()

        """
        Print a SquirrelsParam object.
        """
        rep = "\nParameters:\n"
        rep += "==========\n"
        rep += "- The degree of the ring ring Z[x]/(x^n + 1) is n.\n"
        rep += "- The integer modulus is q.\n"
        rep += "- The Gram-Schmidt norm is gs_norm"
        rep += "- The standard deviation of the signatures is sigma.\n"
        rep += "- The minimal std dev for sampling over Z is sigmin.\n"
        rep += "- The maximal std dev for sampling over Z is sigmax.\n"
        rep += "- The tailcut rate for signatures is tailcut.\n"
        rep += "- Signatures are rejected whenever ||(s1, s2)||^2 > β^2.\n"
        rep += "\n"
        rep += "dim     = " + str(self.dim) + "\n"
        rep += "q       = " + str(self.q) + "\n"
        rep += "min_sampling_gs_norm = " + str(self.min_sampling_gs_norm) + "\n"
        rep += "max_sampling_gs_norm = " + str(self.max_sampling_gs_norm) + "\n"
        rep += "min_gs_norm = " + str(self.min_gs_norm) + "\n"
        rep += "max_gs_norm = " + str(self.max_gs_norm) + "\n"
        rep += "sigma   = " + str(self.sigma) + "\n"
        rep += "sigmin  = " + str(self.sigmin) + "\n"
        rep += "sigmax  = " + str(self.sigmax) + "\n"
        rep += "tailcut = " + str(self.tailcut_rate) + "\n"
        rep += "⌊β⌋     = " + str(self.max_sig_norm) + "\n"
        rep += "⌊β^2⌋   = " + str(self.sq_max_sig_norm) + "\n"
        rep += "log det per dim = " + str(self.log_det/self.dim) + "\n"
        rep += "sq det = " + str(exp(self.log_det/self.dim)) + "\n"
        rep += "det factors   = " + str(self.target_det_fact) + "\n"
        rep += "\n\n"

        rep += "Metrics:\n"
        rep += "========\n"
        rep += "- The maximal number of signing queries is nb_queries.\n"
        rep += "- The signing rejection rate is rejection_rate.\n"
        rep += "- The maximal size of signatures is sig_bytesize.\n"
        rep += "\n"
        rep += "nb_queries     = 2^" + str(int(log(self.nb_queries, 2))) + "\n"
        rep += "rejection_rate = " + str(self.rejection_rate) + "\n"
        rep += "sig_bytesize   = " + str(int(self.sig_bytesize))
        rep += " (rate " + str(self.sig_bytesize_rate) + ")\n"
        rep += "sig_th_bytesize   = " + str(int(self.th_sig_bytesize)) + "\n"        
        rep += "pubkey_size    = " + str(int(self.pubkey_size)) + "\n"        
        rep += "seckey_size    = " + str(int(self.seckey_size)) + "\n"        
        rep += "\n\n"

        rep += "Security:\n"
        rep += "=========\n"
        rep += "- The targeted security level is target_bitsec.\n"
        rep += "- For x in {keyrec, forgery} (i.e. key recovery or forgery):\n"
        rep += "  - The BKZ blocksize required to achieve x is x_blocksize.\n"
        rep += "  - The classic CoreSVP hardness of x is x_coresvp_c.\n"
        rep += "  - The quantum CoreSVP hardness of x is x_coresvp_q.\n"
        rep += "  Values in parenthesis use the [Duc18] optimization.\n"
        rep += "\n"
        rep += "target_bitsec     = " + str(self.target_bitsec) + "\n"
        rep += "keyrec_blocksize  = " + str(self.keyrec_blocksize)
        rep += " (" + str(self.keyrec_blocksize_opt) + ")\n"
        rep += "keyrec_coresvp_c  = " + str(self.keyrec_coresvp_c)
        rep += " (" + str(self.keyrec_coresvp_opt_c) + ")\n"
        rep += "keyrec_coresvp_q  = " + str(self.keyrec_coresvp_q)
        rep += " (" + str(self.keyrec_coresvp_opt_q) + ")\n"
        rep += "keyrec_hybrid_c   = {g: " + str(self.keyrec_hybrid_c[0]) + ", B: "
        rep += str(self.keyrec_hybrid_c[1]) + ", sec: " + str(self.keyrec_hybrid_c[2]) + "}\n"
        rep += "keyrec_hybrid_q   = {g: " + str(self.keyrec_hybrid_q[0]) + ", B: "
        rep += str(self.keyrec_hybrid_q[1]) + ", sec: " + str(self.keyrec_hybrid_q[2]) + "}\n"
        rep += "forgery_blocksize = " + str(self.forgery_blocksize)
        rep += " (" + str(self.forgery_blocksize_opt) + ")\n"
        rep += "forgery_coresvp_c = " + str(self.forgery_coresvp_c)
        rep += " (" + str(self.forgery_coresvp_opt_c) + ")\n"
        rep += "forgery_coresvp_q = " + str(self.forgery_coresvp_q)
        rep += " (" + str(self.forgery_coresvp_opt_q) + ")\n"
        rep += "forgery2_blocksize = " + str(self.forgery2_blocksize)
        rep += " (" + str(self.forgery2_blocksize_opt) + ")\n"
        rep += "forgery2_coresvp_c = " + str(self.forgery2_coresvp_c)
        rep += " (" + str(self.forgery2_coresvp_opt_c) + ")\n"
        rep += "forgery2_coresvp_q = " + str(self.forgery2_coresvp_q)
        rep += " (" + str(self.forgery2_coresvp_opt_q) + ")\n"
        return rep

def explore(p_q, dims, max_gs_norm, gs_width, target_bitsec, secu_level):
    min_sign_size = float("+inf")
    min_params = None

    for q, dimension, gs_norm, width in tqdm.tqdm(list(product(p_q, dims, max_gs_norm, gs_width))):
        try:
            params = SquirrelsParam(
                min_sampling_gs_norm=gs_norm-width,
                max_sampling_gs_norm=gs_norm+width,
                dim=dimension,
                q=q,
                target_bitsec=target_bitsec,
                fail_early_secu=secu_level
            )
        except AssertionError:
            continue # the parameters are not suitable

        if secu_level[0] != None and (params.keyrec_coresvp_c < secu_level[0] or params.forgery_coresvp_c < secu_level[0] or params.forgery2_coresvp_c < secu_level[0]):
            continue
        if secu_level[1] != None and (params.keyrec_coresvp_q < secu_level[1] or params.forgery_coresvp_q < secu_level[1] or params.forgery2_coresvp_q < secu_level[1]):
            continue

        if min_sign_size > params.th_sig_bytesize:
            min_sign_size = params.th_sig_bytesize
            min_params = params

    return min_params

print("Finding parameters for NIST level I:")
min_params_nivI = explore(
    p_q=[1 << 12],
    dims=range(1028, 1038, 2),
    max_gs_norm=range(29, 32, 1),
    gs_width=[1.1],
    target_bitsec=128, 
    secu_level=(124, None),
)
print(min_params_nivI)

print("Finding parameters for NIST level II:")
min_params_nivII = explore(
    p_q=[1 << 12],
    dims=range(1140, 1170, 4),
    max_gs_norm=range(29, 34, 1),
    gs_width=[1],
    target_bitsec=128, 
    secu_level=(None, 128)
)
print(min_params_nivII)

print("Finding parameters for NIST level III:")
min_params_nivIII = explore(
    p_q=[1 << 12],
    dims=range(1540, 1560, 4),
    max_gs_norm=range(29, 40, 2),
    gs_width=[1.2],
    target_bitsec=192, 
    secu_level=(192, None)
)
print(min_params_nivIII)

print("Finding parameters for NIST level IV:")
min_params_nivIV = explore(
    p_q=[1 << 12],
    dims=range(1690, 1720, 2),
    max_gs_norm=range(29, 33, 1),
    gs_width=[1.2],
    target_bitsec=192, 
    secu_level=(None, 192)
)
print(min_params_nivIV)

print("Finding parameters for NIST level V:")
min_params_nivV = explore(
    p_q=[1 << 12],
    dims=range(2020, 2060, 4),
    max_gs_norm=range(30, 40, 2),
    gs_width=[1.3],
    target_bitsec=256, 
    secu_level=(256, None)
)
print(min_params_nivV)
