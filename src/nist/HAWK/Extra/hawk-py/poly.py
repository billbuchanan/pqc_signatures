#This file contains several helper functions of polynomial arithmetic

import numpy as np
from sympy.ntheory import primitive_root
import math

###############################
# NTT / invNTT 
###############################
def brv(x, b=32):
    r = 0
    for i in range(b):
        r |= ((x >> i) & 1) << (b - 1 - i)
    return r

def compute_zetas(root_of_unity, p, log_deg):
    x = root_of_unity
    zetas = [None for _ in range(1 << log_deg)]
    for u in range(1 << log_deg):
        zetas[u] = pow(x, brv(u, log_deg), p)
    return zetas

def ntt(f, p, zetas=None):
    if zetas is None:
        zetas, _ = get_roots(p, len(f))
    F = [int(f) for f in f]
    deg = len(f)
    l = deg // 2
    k = 1
    while l > 0:
        for s in range(0, deg, 2 * l):
            zeta = zetas[k]
            k += 1
            for j in range(s, s + l):
                t = (F[j + l] * zeta) % p
                F[j + l] = (F[j] - t) % p
                F[j] = (F[j] + t) % p
        l = l // 2
    return F

def intt(f, p, zetas=None):
    if zetas is None:
        _, zetas = get_roots(p, len(f))
    F = [int(f) for f in f]
    deg = len(f)
    l = 1
    k = deg - 1
    while l < deg:
        for s in reversed(range(0, deg, 2 * l)):
            zeta = zetas[k]
            k -= 1
            for j in range(s, s + l):
                t = F[j]
                F[j] = (t + F[j + l]) % p
                F[j + l] = (t - F[j + l]) % p
                F[j + l] = (F[j + l] * zeta) % p
        l = l * 2

    ideg = pow(deg, p - 2, p)
    F = [(f * ideg) % p for f in F]
    return F

def nttadj(u, p):
    zetas, izetas = get_roots(p, len(u))
    ui = intt(u, p, izetas)
    return ntt(adjoint(ui), p, zetas)

def get_roots(p, n):
    logn = int(math.log2(n))
    g0 = primitive_root(p)
    b = (p - 1) // (2 * n)
    g0 = (g0**b) % p

    zetas = compute_zetas(g0, p, logn)
    izetas = [pow(z, p - 2, p) for z in zetas]
    return zetas, izetas

########################################
# Polynomial operations
########################################
def poly_mul_ntt(p1, p2, p):
    n = len(p1)

    zetas, izetas = get_roots(p, n)
    p1ntt = ntt(p1, p, zetas)
    p2ntt = ntt(p2, p, zetas)
    rntt = [(a * b) % p for a, b in zip(p1ntt, p2ntt)]
    m = intt(rntt, p, izetas)
    for i in range(n):
        if m[i] > (p - 1) // 2:
            m[i] -= p
    return m


def poly_mul_schoolbook(p1, p2, p):
    deg = len(p1)
    r = [0 for _ in range(deg)]
    for i in range(deg):
        for j in range(deg):
            ij = i + j
            if ij >= deg:
                r[ij % deg] -= ((p1[i] * p2[j])) % p
            else:
                r[ij % deg] += ((p1[i] * p2[j])) % p

    r = [x % p for x in r]
    return r

def adjoint(u):
    ustar = u.copy()
    n = len(u)
    for i in range(1, n):
        ustar[i] = -u[n - i]
    return ustar

def poly_sub(p0, p1):
    return [p0 - p1 for (p0, p1) in zip(p0, p1)]


def poly_add(p0, p1):
    return [p0 + p1 for (p0, p1) in zip(p0, p1)]

########################################
# Polynomial properties
########################################
def infnorm(poly):
    return max([abs(p) for p in poly])

def bytes_to_poly(h, n):
    h0 = [None] * n
    for i in range(n):
        h0[i] = (h[i // 8] >> (i % 8)) & 1
    return h0

def l2norm(x):
    x = [x * x for x in x]
    return sum(x)

def isinvertible(poly, p):
    if p == 2:
        return (np.sum(poly) % 2) == 1

    polyntt = ntt(poly, p)
    return all([c != 0 for c in polyntt])
