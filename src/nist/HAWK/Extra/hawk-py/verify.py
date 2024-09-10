from sympy.ntheory import npartitions
from codec import decode_public, decode_sign
from params import PARAMS
import numpy as np
from sign import symbreak
from poly import poly_sub, bytes_to_poly, brv, get_roots, ntt, nttadj, adjoint
import hashlib


def hawkverify(logn, pub, msg, sig):
    """
    hawkverify (see Alg 20)

    Performs signature verification for Hawk

    Inputs:
        - logn : log2 of polynomial degree
        - pub : public key
        - msg : message
        - sig : candidate signature

    Outputs:
        - True if sig is a valid signature of msg, False otherwise
    """
    n = 1 << logn
	
	# Line 1: r <- DecodeSignature(sig)
    r = decode_sign(logn, sig)
	# Line 2: if r == ⊥ then
    if r is None:
		#Line 3: return False
        return False
	# Line 4: (salt,s1) <- r 
    salt, s1 = r

	# Line 5: DecodePublic(pub)
    r = decode_public(logn, pub)
	# Line 6: if r == ⊥ then
    if r is None:
		#Line 7: return False
        return False
	# Line 8: (q00,q01) <- r
    q00, q01 = r
	
	# Line 9: hpub <- SHAKE256(pub)
    shake256 = hashlib.shake_256()
    shake256.update(pub.tobytes())
    hpub = shake256.digest(PARAMS(logn, "lenhpub"))

	# Line 10: M <- SHAKE256( m || hpub)
    shake256 = hashlib.shake_256()
    shake256.update(msg.tobytes() + hpub)
    M = shake256.digest(64)

	# Line 11: (h0,h1) <- SHAKE256(M||salt)[0:2n]
    shake256 = hashlib.shake_256()
    shake256.update(M + salt.tobytes())
    h = shake256.digest(n // 4)
    h0 = bytes_to_poly(h[: n // 8], n)
    h1 = bytes_to_poly(h[n // 8 :], n)

    f = hawkverify_unpacked(logn, s1, q00, q01, h0, h1)
    return f


def hawkverify_unpacked(logn, s1, q00, q01, h0, h1):
    """
    hawkverify_unpacked (see Alg 20)

    This is used a subroutine for hawkverify, operating on polynomials and not encoded sig / pub
    """
    n = 1 << logn

	# Line 12: w1 <- h1 - 2 * s1
    w1 = poly_sub(h1, 2 * np.array(s1))

	# Line 13: if sym-break(w1) == false then 
    if symbreak(w1) == False:
		# Line 14: Return False 
        return False
		
	# Line 15: w0 <- RebuildS0(q00,q01,w1,h0)
    w0 = rebuilds0(logn, q00, q01, w1, h0)
	# Line 16: if w0 == ⊥ then
    if w0 is None:
		# Line 17: return False 
        return False

    p1, p2 = (2147473409, 2147389441)
	
	# Line 18: r1 <- PolyQnorm(q00,q01,w0,w1,p1)
    r1 = polyQnorm(q00, q01, w0, w1, p1)
	# Line 19: r2 <- PolyQnorm(q00,q01,w0,w1,p2)
    r2 = polyQnorm(q00, q01, w0, w1, p2)

	# Line 20: if r1 != r2 or r1 != 0 mod n then
    if r1 != r2 or r1 % n != 0:
		# Line 21: return False 
        return False
	
	# Line 22 : r1 <- r1 / n
    r1 = r1 // n

	# Line 23: if r1 > 8 * n * sigmaverfy**2 then 
    if r1 > 8 * n * PARAMS(logn, "sigmaverify") ** 2:
		# Line 24: return False 
        return False

	# Line 25: return True
    return True


def rebuilds0(logn, q00, q01, w1, h0):
    """
    rebuilds0 (see Alg 18)

    Rebuild s0 from public key and signature polynomials

    Inputs:
        - logn : log2 of polynomial degree
        - q00 : public polynomial (pub key)
        - q01 : public polynomial (pub key)
        - w1 : public polynomial (sig)
        - h0 : public polynomial (h0)

    Outputs:
        - w0: recovered s0
    """

    n = 1 << logn
    cw1 = 2 ** (29 - (1 + PARAMS(logn, "highs1")))
    cq00 = 2 ** (29 - PARAMS(logn, "high00"))
    cq01 = 2 ** (29 - PARAMS(logn, "high01"))
    cs0 = (2 * cw1 * cq01) / (n * cq00)
    w1scalled = (np.array(w1, dtype=np.int32)) * cw1
    w1fft = fft(w1scalled)
    z00 = q00.copy()
    if z00[0] < 0:
        return False
    z00[0] = 0
    q00fft = fft(np.array(z00, dtype=np.int32) * cq00)
    q01fft = fft(np.array(q01, dtype=np.int32) * cq01)
    alpha = (2 * cq00 * np.int64(q00[0])) // n
    for u in range(0, n // 2):
        x_re = np.int64(q01fft[u]) * np.int64(w1fft[u])
        x_re -= np.int64(q01fft[u + n // 2]) * np.int64(w1fft[u + n // 2])

        x_im = np.int64(q01fft[u]) * np.int64(w1fft[u + n // 2])
        x_im += np.int64(q01fft[u + n // 2]) * np.int64(w1fft[u])

        (x_re, z_re) = (np.abs(x_re), sgn(x_re))
        (x_im, z_im) = (np.abs(x_im), sgn(x_im))

        v = alpha + q00fft[u]
        if v <= 0 or v >= 2**32 or x_re >= v * 2**32 or x_im >= v * 2**32:
            return False

        y_re = np.int32(x_re // v)
        y_im = np.int32(x_im // v)

        q01fft[u] = y_re - 2 * z_re * y_re
        q01fft[u + n // 2] = y_im - 2 * z_im * y_im

    t = invfft(q01fft)
    w0 = np.zeros(n, dtype=np.int32)
    for u in range(n):
        v = cs0 * np.int32(h0[u]) + np.int32(t[u])
        z = (v + cs0) // (2 * cs0)
        if z < -(2 ** PARAMS(logn, "highs0")) or z >= 2 ** (PARAMS(logn, "highs0")):
            return False
        w0[u] = np.int32(h0[u]) - 2 * z

    return w0


def polyQnorm(q00, q01, w0, w1, p):
    """
    polyQnorm (see Alg 19)

    Computes the Q norm of w

    Inputs:
        - q00 : polynomial
        - q01 : polynomial
        - w0 : polynomial
        - w1 : polynomial
        - p : prime

    Outputs:
        - n ||W||_Q**2 mod p
    """
    zetas, _ = get_roots(p, len(q00))
    q00ntt = ntt(q00, p, zetas)
    q01ntt = ntt(q01, p, zetas)
    w0ntt = ntt(w0, p, zetas)
    w1ntt = ntt(w1, p, zetas)

    d = [(w1 * pow(int(q00), p - 2, p)) % p for (w1, q00) in zip(w1ntt, q00ntt)]
    c = [(d * w1adj) % p for (d, w1adj) in zip(d, ntt(adjoint(w1), p))]
    acc = sum(c)
    e = [(int(w0) + d * int(q01)) % p for (w0, d, q01) in zip(w0ntt, d, q01ntt)]
    c = [
        (int(q00) * int(e) * int(eadj)) % p
        for (q00, e, eadj) in zip(q00ntt, e, nttadj(e, p))
    ]
    acc += sum(c)
    return acc % p


###########################
## FFT and invFFT fixed point
###########################
def delta(k):
    """
    delta (see Section 3.6)

    Computes roots

    Inputs:
        - k : index

    Outputs:
        - re : real part
        - im : imaginary part
    """
    d = np.exp(2 * 1j * np.pi * brv(k, b=10) / 2048)
    d = d * (2**31)
    re = np.int32(np.round(d.real))
    im = np.int32(np.round(d.imag))
    return re, im


def sgn(x):
    """
    sgn

    Returns the sign bit of x

    Inputs:
        - x : integer

    Outputs:
        - 1 if x < 0, 0 otherwise
    """
    if x < 0:
        return 1
    return 0


def fft(a):
    """
    fft (see Alg 16)

    Computes the fixed point fft

    Inputs:
        - a : polynomial

    Outputs:
        - afft : fft representation of a

    """
    n = len(a)
    afft = a.copy()
    t = n // 2
    m = 2
    while m < n:
        v0 = 0
        for u in range(0, m // 2):
            e_re, e_im = delta(u + m)
            e_re = np.int64(e_re)
            e_im = np.int64(e_im)

            for v in range(v0, v0 + t // 2):
                x1_re = np.int64(afft[v])
                x1_im = np.int64(afft[v + n // 2])

                x2_re = np.int64(afft[v + t // 2])
                x2_im = np.int64(afft[v + t // 2 + n // 2])

                t_re = x2_re * e_re - x2_im * e_im
                t_im = x2_re * e_im + x2_im * e_re

                afft[v] = np.int32((2**31 * x1_re + t_re) // 2**32)
                afft[v + n // 2] = np.int32((2**31 * x1_im + t_im) // 2**32)
                afft[v + t // 2] = np.int32((2**31 * x1_re - t_re) // 2**32)
                afft[v + t // 2 + n // 2] = np.int32(
                    (2**31 * x1_im - t_im) // 2**32
                )
            v0 = v0 + t
        t = t // 2
        m = 2 * m

    return afft


def invfft(afft):
    """
    invfft (see Alg 17)

    Computes the fixed point invfft

    Inputs:
        - afft : fft representaiton of a

    Outputs:
        - a : polynomial a
    """
    n = len(afft)
    a = afft.copy()
    t = 2
    m = n // 2
    while m > 1:
        v0 = 0
        for u in range(0, m // 2):
            eta_re, eta_im = delta(u + m)

            eta_re = np.int64(eta_re)
            eta_im = np.int64(-eta_im)

            for v in range(v0, v0 + t // 2):
                x1_re = a[v]
                x1_im = a[v + n // 2]

                x2_re = a[v + t // 2]
                x2_im = a[v + t // 2 + n // 2]

                t1_re = x1_re + x2_re
                t1_im = x1_im + x2_im
                t2_re = x1_re - x2_re
                t2_im = x1_im - x2_im

                a[v] = t1_re // 2
                a[v + n // 2] = t1_im // 2
                a[v + t // 2] = np.int32(
                    (np.int64(t2_re) * eta_re - np.int64(t2_im) * eta_im) // 2**32
                )
                a[v + t // 2 + n // 2] = np.int32(
                    (np.int64(t2_re) * eta_im + np.int64(t2_im) * eta_re) // 2**32
                )
            v0 = v0 + t
        t = 2 * t
        m = m // 2

    return a
