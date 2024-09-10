from poly import adjoint, isinvertible, poly_mul_ntt, poly_add, infnorm, l2norm
from ntrugen.ntrugen_hawk import ntru_solve
import ntrugen
from params import PARAMS
from codec import encode_public, encode_private
import hashlib
import numpy as np
from rngcontext import RngContext, SHAKE256x4


def hawkkeygen(logn, rng=None):
    """
    hawkkeygen (see Alg 13)

    Regenerate a priv/pub key pair for Hawk

    Inputs:
        - logn : log2 of polynomial degree
        - rng : rng context used

    Outputs:
        - priv : encoded private key (uint8)
        - pub : encoded public key (uint8)
    """
    if rng is None:
        rng = RngContext(np.random.randint(0, 256, 40, dtype=np.uint8))

    _, _, _, _, _, _, _, priv, pub = hawkkeygen_unpacked(logn, rng)
    return priv, pub


def hawkkeygen_unpacked(logn, rng=None):
    """
    hawkkeygen_unpacked (see Alg 13)

    This is used as a subroutine of hawkkeygen used to exposed directly polynomials

    Inputs:
        - logn : log2 of polynomial degree
        - rng : rng context used

    Outputs:
        - f : polynomial (int8)
        - g : polynomial (int8)
        - F : polynomial (int8)
        - G : polynomial (int8)
        - q00: polynomial (int16)
        - q01: polynomial (int16)
        - kgseed: seed to regenerate (f,g) (uint8)
        - priv: encoded private key
        - pub: encoded public key
    """
    n = 1 << logn

    if rng is None:
        rng = RngContext(np.random.randint(0, 256, 40, dtype=np.uint8))

    # Line 1: kgseen <- Rnd(len_bits(kgseed))
    kgseed = rng.random(PARAMS(logn, "lenkgseed"))

    # Line 2: (f, g) <- Regeneratefg(kgseed)
    f, g = regeneratefg(kgseed.tobytes(), n)

    # Line 3: if isInvertible(f, 2) false or isInvertible(f, 2) is false then
    if not isinvertible(f, 2) or not isinvertible(g, 2):
        # Line 4: restart
        return hawkkeygen_unpacked(logn, rng)

    fadj = adjoint(f)
    gadj = adjoint(g)

    # Line 5: if ||(f,g)||2 <= 2*n*sigkrsec**2:
    if (l2norm(f) + l2norm(g)) <= (2 * n * (PARAMS(logn, "sigmakrsec") ** 2)):
        return hawkkeygen_unpacked(logn, rng)

    p = (1 << 16) + 1

    # Line 7: q00 <- f f* + g g*
    q00 = poly_add(poly_mul_ntt(f, fadj, p), poly_mul_ntt(g, gadj, p))

    # Line 8: (p1, p2) <- (2147473409, 2147389441)
    p1, p2 = (2147473409, 2147389441)

    # Line 9: if isInvertible(q00, p1) false or isInvertible(q00, p2) is false then
    if not isinvertible(q00, p1) or not isinvertible(q00, p2):
        # Line 10: restart
        return hawkkeygen_unpacked(logn, rng)

    # Line 11: if (1/q00)[0] >= beta0 then
    invq00 = ntrugen.fft.inv_fft(q00)
    if invq00[0] >= PARAMS(logn, "beta0"):
        # Line 12: restart
        return hawkkeygen_unpacked(logn, rng)

    try:
        # Line 13: r <- NTRUSolve(f, g, 1)
        # Line 16: (F, G) <- r
        F, G = ntru_solve(f, g)
    except ValueError:
        # Line 14&15: if r = \bot then restart
        return hawkkeygen_unpacked(logn, rng)

    # Line 17: if infnorm( (F,G) ) > 127 then
    if infnorm(F) > 127 or infnorm(G) > 127:
        # Line 18: restart
        return hawkkeygen_unpacked(logn, rng)

    Fadj = adjoint(F)
    Gadj = adjoint(G)

    # Line 19: q01 <- F f* + G g*
    q01 = poly_add(poly_mul_ntt(F, fadj, p), poly_mul_ntt(G, gadj, p))
    p = 8380417
    # Line 20: q11 <- F F* + G G*
    q11 = poly_add(poly_mul_ntt(F, Fadj, p), poly_mul_ntt(G, Gadj, p))

    # Line 21: if |q11[i]| >= 2^high11 for any i > 0 then
    if any(abs(q11i) >= 2 ** (PARAMS(logn, "high11")) for q11i in q11[1:]):
        # Line 22: restart
        return hawkkeygen_unpacked(logn, rng)

    # Line 23: pub <- EncodePublic(q00, q01)
    pub = encode_public(logn, q00, q01)

    # Line 24: if pub = \bot then
    if pub is None:
        # Line 25: restart
        return hawkkeygen_unpacked(logn, rng)

    # Line 16: hpub <- SHAKE256(pub)
    shake256 = hashlib.shake_256()
    shake256.update(pub.tobytes())
    hpub = np.frombuffer(shake256.digest(PARAMS(logn, "lenhpub")), dtype=np.uint8)

    # Line 27: priv <- EncodePrivate(kgseen, F mod 2, G mod 2, hpub)
    priv = encode_private(kgseed, F, G, hpub)

    # Line 28: return (priv, pub)
    return f, g, F, G, q00, q01, kgseed, priv, pub


def regeneratefg(kgseed, n):
    """
    regeneratefg (see Alg 12)

    Regenerates (f,g) deterministically from a seed

    Inputs:
        - kgseed : seed
        - n : polynomial degree of f and g

    Outputs:
        - f : polynomial of int
        - g : polynomial of int
    """
    b = int(n / 64)
    assert b == 4 or b == 8 or b == 16

    y = SHAKE256x4(kgseed, 2 * n * b // 64)  # array of 64-bit values

    # map y to a sequence of bits
    ybits = [None] * b * 2 * n
    for j, y in enumerate(y):
        for bi in range(64):
            ybits[j * 64 + bi] = (y >> bi) & 1

    f = [int(0)] * n
    for i in range(n):
        sum = 0
        for j in range(b):
            sum += ybits[i * b + j]
        f[i] = sum - b // 2

    g = [0] * n
    for i in range(n):
        sum = 0
        for j in range(b):
            sum += ybits[(i + n) * b + j]
        g[i] = sum - b // 2

    return (f, g)
