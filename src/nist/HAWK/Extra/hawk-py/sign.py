# This file contains functions related to signing with Hawk-(256,512,1024)
from keygen import regeneratefg
import hashlib
from poly import poly_mul_ntt, poly_add, l2norm, poly_sub, bytes_to_poly
from rngcontext import RngContext, SHAKE256x4
from params import PARAMS
from codec import decode_private, encode_sign
import numpy as np


def samplersign(s, t, n, T0, T1):
    """
    SampleSign (see Alg 14)

    Inputs:
        - s randomness seed (bytes)
        - t center vector
        - n degree of polynomials
        - T0 / T1 CDT tables

    Outputs:
        - d: discret Gaussian polynomial
    """
    y = SHAKE256x4(s, int(5 * n / 2))
    d = [None] * 2 * n

    for j in range(4):
        for i in range(int(n / 8)):
            for k in range(3 + 1):
                r = 16 * i + 4 * j + k
                a = y[j + 4 * (5 * i + k)]
                b = (y[j + 4 * (5 * i + 4)] // (2 ** (16 * k))) % 2**15
                c = (a % 2**63) + 2**63 * b
                (v0, v1) = (0, 0)
                z = 0
                while T0[z] != 0 and T1[z] != 0:
                    if c < T0[z]:
                        v0 = v0 + 1
                    if c < T1[z]:
                        v1 = v1 + 1
                    z = z + 1
                if t[r] == 0:
                    v = 2 * v0
                else:
                    v = 2 * v1 + 1
                if a >= 2**63:
                    v = -v
                d[r] = v
    return d


def symbreak(w):
    """
    symbreak (see Section 3.5.2)

    Inputs:
        - w input polynomial

    Outputs:
        - True iff w[i] > 0 and w[j] = 0 for all j < i
    """
    for x in w:
        if x != 0:
            if x > 0:
                return True
            else:
                return False
    return False


def hawksign(logn, priv, msg, rng=None):
    """
    hawksign (see Alg. 15)

    Inputs:
        - logn log2 of polynomial degree
        - priv private key
        - msg  mesasge to sign
        - rng RngContext used during signing

    outputs:
        - valid signature of msg under the private key
    """
    # Line 1: (kgseed, F mod 2, G mod 2, hpub) <- DecodePrivate(priv)
    kgseed, Fmod2, Gmod2, hpub = decode_private(logn, priv)

    while True:
        salt, s1 = hawksign_unpacked(logn, Fmod2, Gmod2, kgseed, hpub, msg, rng)

        # Line 18: EncodeSignature(salt, s1)
        sig = encode_sign(logn, salt, s1)

        # Line 19: if sig != bottom then
        if sig is not None:
            # Line 20: return sig
            return sig


def hawksign_unpacked(logn, Fmod2, Gmod2, kgseed, hpub, msg, rng=None):
    """
    hawksign_unpacked (see Alg. 15)

    This is the subroutine of hawksign with decoded / unpacked private key.

    Inputs:
        - logn log2 of polynomial degree
        - Fmod2
        - Gmod2
        - kgseed seed to reconstruct (f,g)
        - hpub hash of public key
        - msg message to sign
        - rng RngContext used during signing

    Outputs:
        - salt to generate (h0,h1)
        - s1 signature polynomial
    """
    if rng is None:
        rng = RngContext(np.random.randint(0, 256, 32))

    n = 1 << logn

    # Line 2: (f,g) <- Regenerate(kgseed)
    f, g = regeneratefg(kgseed.tobytes(), n)

    # Line 3: M <- SHAKE256(m || hpub)[0:512]
    shake256 = hashlib.shake_256()
    shake256.update(msg.tobytes() + hpub.tobytes())
    M = shake256.digest(64)

    # Line 4: a <- 0
    a = 0

    # prime used for NTTs
    p = (1 << 16) + 1

    # Line 5: loop
    while True:
        # Line 6: salt <- SHAKE256(M||kgseed||EncodeInt(a,32)||Rnd(saltlen))[0:saltlenbits]
        shake256 = hashlib.shake_256()
        shake256.update(
            M
            + kgseed.tobytes()
            + a.to_bytes(4, "little")
            + rng.random(PARAMS(logn, "lensalt")).tobytes()
        )
        salt = np.frombuffer(shake256.digest(PARAMS(logn, "lensalt")), dtype=np.uint8)

        # Line 7: (h0,h1) <- SHAKE256(M||salt)[0:2n]
        shake256 = hashlib.shake_256()
        shake256.update(M + salt.tobytes())

        h = shake256.digest(n // 4)
        h0 = bytes_to_poly(h[: n // 8], n)
        h1 = bytes_to_poly(h[n // 8 :], n)

        # Line 8: (t0,t1) <- ((h0*f + f1*F) mod 2, (h0*g + f1*G) mod 2)
        t0 = [
            x % 2 for x in poly_add(poly_mul_ntt(h0, f, p), poly_mul_ntt(h1, Fmod2, p))
        ]
        t1 = [
            x % 2 for x in poly_add(poly_mul_ntt(h0, g, p), poly_mul_ntt(h1, Gmod2, p))
        ]

        # Line 9: s <- M || kgseed || EncodeInt(a+1,32) || Rnd(320)
        seed = rng.random(320 // 8)
        s = M + kgseed.tobytes() + (a + 1).to_bytes(4, "little") + seed.tobytes()

        # Line 10: (d0, d1) <- SampleSign(s,(t0,t1))
        d = samplersign(s, t0 + t1, n, PARAMS(logn, "T0"), PARAMS(logn, "T1"))
        d0 = d[:n]
        d1 = d[n:]

        # Line 11: a <- a + 2
        a = a + 2

        # Line 12: if ||(d0,d1)||2 > 8 * n * sigm_verify^2
        if l2norm(d0) + l2norm(d1) > 8 * n * (PARAMS(logn, "sigmaverify") ** 2):
            # Line 13: continue loop
            continue

        # Line 14: f * d1 - g * d0
        w1 = poly_sub(poly_mul_ntt(f, d1, p), poly_mul_ntt(g, d0, p))

        # Line 15: if sym-break(w1) ==
        if not symbreak(w1):
            # Line 16: w1 <- -w1
            w1 = [-x for x in w1]

        # Line 17: s <- (h1 - w1) / 2
        sig = [x // 2 for x in poly_sub(h1, w1)]

        return salt, sig
