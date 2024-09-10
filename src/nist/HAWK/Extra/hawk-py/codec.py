# This file contains the functions for coding/decoding keys and signature.
import numpy as np
from params import PARAMS
from poly import bytes_to_poly


def decode_private(logn, priv):
    """
    decode_private (see Alg. 5)

    Decodes private key

    Inputs:
        - logn log2 of polynomial degree
        - priv encoded private key

    Outputs:
        - kgseed seed to regenerate (f,g)
        - Fmod2 polynomial F reduced mod 2
        - Gmod2 polynomial G reduced mod 2
        - hpub hash of public key
    """
    n = 1 << logn
    lenkgseed = PARAMS(logn, "lenkgseed")
    kgseed = priv[:lenkgseed]
    Fmod2 = bytes_to_poly(priv[lenkgseed : lenkgseed + n // 8], 1 << logn)
    Gmod2 = bytes_to_poly(priv[lenkgseed + n // 8 : lenkgseed + n // 4], 1 << logn)
    hpub = priv[-PARAMS(logn, "lenhpub") :]
    return kgseed, Fmod2, Gmod2, hpub


def encode_private(kgseed, F, G, hpub):
    """
    encode_private (see Alg. 4)

    Perform private key encoding

    Inputs:
        - kgseed seed to regenerate (f,g)
        - F polymial F
        - G polymial G
        - hpub hash of public key

    Outputs:
        - private key (as numpy array of np.uint8)
    """
    Fmod2 = [x % 2 for x in F]
    Gmod2 = [x % 2 for x in G]
    return np.append(
        np.append(
            kgseed, np.packbits(np.array(Fmod2 + Gmod2, dtype=bool), bitorder="little")
        ),
        hpub,
    )


def encodeint(x, k):
    """
    encodeint (see Section 3.3.1)

    Encodes the integer x in k bits

    Inputs:
        - x : integer
        - k : number of bits to encode x

    Outputs:
        - b : numpy array with b[0] being the LSB and b[k-1] the MSB
    """
    b = np.array([], dtype=bool)
    assert k >= 0
    if k == 0:
        return b  # return an empty sequence
    assert x >= 0
    assert x < 2**k
    n = x
    for _ in range(int(k)):
        b = np.append(b, int(n % 2 == 1))
        n //= 2
    return b


def decodeint(x):
    """
    decodeint (see Section 3.3.1)

    Decodes sequence of bits into integer

    Inputs:
        - x : sequence of bits

    Outputs:
        - c : integer sum(x[i] * (2**i))
    """
    c = 0
    for i in range(len(x)):
        c *= 2
        assert x[len(x) - 1 - i] == 0 or x[len(x) - 1 - i] == 1
        c += x[len(x) - 1 - i]
    return c


def compressgr(x, low, high):
    """
    compressgr (see Alg 6)

    Performs Golomb Rice compression for integer sequence x

    Inputs:
        - x : integer sequence
        - low: size low
        - high: size high

    Outputs:
        - y : sequence of bits (result of compression)
    """
    k = len(x)
    assert k % 8 == 0
    for i in range(k):
        assert x[i] < 2 ** high and x[i] > -(2**high)
    # Set y to an empty sequence of bits.
    y = np.array([], dtype=bool)
    # Set v to an empty sequence of integers.
    v = np.array([], dtype=np.int16)
    # for i = 0 to k − 1 do
    #   s = 1 if x[i] < 0, or 0 if x[i] ≥ 0
    #   y = y ∥ s
    #   v = v ∥ x[i] − s(2x[i] + 1)
    #   if v[i] ≥ 2^high then
    #     return ⊥
    for i in range(0, k):
        s = int(x[i] < 0)
        y = np.append(y, bool(s))
        v = np.append(v, x[i] - s * (2 * x[i] + 1))
        if v[i] >= (2**high):
            return None
    # for i = 0 to k − 1 do
    #   y ← y ∥ encodeint(v[i] mod 2^low,low)
    for i in range(k):
        y = np.append(y, encodeint(v[i] % (2**low), low))
    #   for i = 0 to k − 1 do
    #     y ← y ∥ encodeint(0, ⌊v[i]/2^low⌋) ∥ 1
    for i in range(k):
        y = np.append(y, encodeint(0, v[i] // 2**low))
        y = np.append(y, 1)

    return y


def decompressgr(y, k, low, high):
    """
    decompressgr (see Alg. 7)

    Decompression for Golomb and Rice

    Inputs:
        - y : bit sequence
        - k : length of bit sequence, must have k % 8 == 0
        - low: size low
        - high : size high

    Outputs:
        - x : integer decoded sequence
        - j : size of integer sequence

    """
    assert k % 8 == 0
    # if lenbits(y) < k(low + 2) then
    #   return ⊥
    if len(y) < k * (low + 2):
        return None
    # for i = 0 to k − 1 do
    #   x[i] ← decodeint(y[i ·low + k : (i + 1) ·low + k])
    x = [0] * k
    for i in range(k):
        x[i] = decodeint(y[i * low + k : (i + 1) * low + k])
    # j ← k(low + 1)
    # for i = 0 to k − 1 do
    #   z ← −1
    #   repeat
    #     z ← z + 1
    #     if j ≥ len_bits(y) or z ≥ 2^(high−low) then
    #       return ⊥
    #     t ← y[j]
    #     j ← j + 1
    #   until t = 1
    #   x[i] ← x[i] + z · 2^low
    j = k * (low + 1)
    for i in range(k):
        z = -1
        t = 0
        while t != 1:
            z = z + 1
            if j >= len(y) and z >= 2 ** (high - low):
                return None
            t = y[j]
            j = j + 1
        x[i] = x[i] + z * 2**low
    # for i = 0 to k − 1 do
    #   x[i] ← x[i] − y[i](2x[i] + 1)  ▷ Application of the sign bit.
    for i in range(k):
        x[i] = x[i] - y[i] * (2 * x[i] + 1)
    return (x, j)


def encode_public(logn, q00, q01):
    """
    encode_public (See Alg 8)

    Perform public key encoding

    Inputs:
        - logn : log2 of polynomial degree
        - q00 : public polynomial on int16
        - q01 : public polynomial on int16

    Outputs:
        - pub: public key on uint8
    """
    n = 1 << logn

    if q00[0] < -(2**15) or q00[0] >= 2**15:
        return None, False

    v = 16 - PARAMS(logn, "high00")

    qp00 = q00.copy()
    qp00[0] = q00[0] // (2**v)

    y00 = compressgr(
        qp00[0 : int(n / 2)], PARAMS(logn, "low00"), PARAMS(logn, "high00")
    )

    if y00 is None:
        return None

    y00 = np.append(y00, encodeint(q00[0] % (2**v), v))

    while len(y00) % 8 != 0:
        y00 = np.append(y00, 0)

    y01 = compressgr(q01, PARAMS(logn, "low01"), PARAMS(logn, "high01"))
    if y01 is None:
        return None

    y = np.append(y00, y01)

    if len(y) > PARAMS(logn, "lenpub") * 8:
        return None

    while len(y) < PARAMS(logn, "lenpub") * 8:
        y = np.append(y, 0)

    return np.packbits(y, axis=None, bitorder="little")


def decode_public(logn, pub):
    """
    decode_public (see Alg 9)

    Perform public key decoding

    Inputs:
        - logn : log2 of polynomial degree
        - pub: public key encoded (on uint8)

    Outputs:
        - q00: public polynomial (on int16)
        - q01: public polynomial (on int16)
    """
    n = 1 << logn
    if len(pub) != PARAMS(logn, "lenpub"):
        return None

    v = 16 - PARAMS(logn, "high00")
    y = np.unpackbits(pub, bitorder="little")

    r00 = decompressgr(y, n // 2, PARAMS(logn, "low00"), PARAMS(logn, "high00"))
    if r00 is None:
        return None
    r00, j = r00
    q00 = np.zeros(n, dtype=np.int16)
    q00[: n // 2] = r00

    if len(y) * 8 < j + v:
        return None

    q00[0] = 2**v * q00[0] + decodeint(y[j : j + v])

    j = j + v

    while j % 8 != 0:
        if j >= len(y) or y[j] != 0:
            return None
        j += 1

    q00[n // 2] = 0
    for i in range(n // 2 + 1, n):
        q00[i] = -q00[n - i]

    r01 = decompressgr(y[j:], n, PARAMS(logn, "low01"), PARAMS(logn, "high01"))
    if r01 is None:
        return None
    r01, jp = r01

    j = j + jp
    q01 = np.array(r01, dtype=np.int16)

    while j < len(y):
        if y[j] != 0:
            return None

        j += 1

    return q00, q01


def encode_sign(logn, salt, s1):
    """
    encode_sign (see Alg 10)

    Encode signatures

    Inputs:
        - logn : log2 of polynomial degree
        - salt : salt (uint8s) to regenerate h0, h1
        - s1: signature polynomial (int16)

    Outputs:
        - y : encoded signature (uint8s)
    """
    y = compressgr(s1, PARAMS(logn, "lows1"), PARAMS(logn, "highs1"))
    if y is None:
        return None

    leny = (PARAMS(logn, "lensig") - PARAMS(logn, "lensalt")) * 8
    if len(y) > leny:
        return None

    while len(y) < leny:
        y = np.append(y, 0)

    return np.append(salt, np.packbits(y, bitorder="little"))


def decode_sign(logn, sig):
    """
    decode_sign (see Alg 11)

    Decodes signatures

    Inputs:
        - logn : log2 of polynomial degree
        - sig : encoded signature (uint8)

    Outputs:
        - salt: salt (uint8s) to regenerate h0,h1
        - s1: signature polynomial (int16)

    """
    n = 1 << logn
    y = np.unpackbits(sig, bitorder="little")
    if len(sig) != PARAMS(logn, "lensig"):
        return None

    salt = sig[: PARAMS(logn, "lensalt")]

    s1 = decompressgr(
        y[PARAMS(logn, "lensalt") * 8 :],
        n,
        PARAMS(logn, "lows1"),
        PARAMS(logn, "highs1"),
    )

    if s1 is None:
        return None

    s1, j = s1
    s1 = np.array(s1, dtype=np.int16)

    j += PARAMS(logn, "lensalt") * 8
    while j < len(y):
        if y[j] != 0:
            return None
        j += 1

    return salt, s1
