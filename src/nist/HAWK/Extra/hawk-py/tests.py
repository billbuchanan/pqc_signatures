import ctypes
import numpy as np
from tqdm import tqdm
import glob
from params import PARAMS
from rngcontext import RngContext
from sign import hawksign
from keygen import hawkkeygen
from verify import hawkverify
from codec import (
    encode_public,
    encode_private,
    decode_private,
    decode_public,
    decode_sign,
)
import hashlib
import os


print("Compile reference implementation")
os.system("make -C ref all lib > /dev/null 2>&1 ")
hawk = ctypes.CDLL(glob.glob("**/hawk.so")[0])


######################
## Hawk reference top functions
######################
def c_keygen(logn, seedrng=None):
    priv = np.zeros(PARAMS(logn, "lenpriv"), dtype=np.uint8)
    pub = np.zeros(PARAMS(logn, "lenpub"), dtype=np.uint8)

    if seedrng is None:
        seedrng = np.random.randint(0, 256, 32, dtype=np.uint8)
    else:
        seedrng = np.array(seedrng, dtype=np.uint8)

    hawk.c_keygen(
        logn,
        priv.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        pub.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        seedrng.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        len(seedrng),
    )

    return priv, pub


def c_sign(logn, priv, msg, seedrng=None):
    sig = np.zeros(PARAMS(logn, "lensig"), dtype=np.uint8)
    if seedrng is None:
        seedrng = np.random.randint(0, 256, 32, dtype=np.uint8)
    else:
        seedrng = np.array(seedrng, dtype=np.uint8)

    hawk.c_sign(
        logn,
        priv.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        msg.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        len(msg),
        sig.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        seedrng.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        len(seedrng),
    )
    return sig


def c_verify(logn, pub, msg, sig):
    return hawk.c_verify(
        logn,
        pub.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        msg.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        len(msg),
        sig.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
    )


######################
## Uncompressed calls
######################
def c_keygen_unpacked(logn, seedrng=None):
    n = 1 << logn
    if seedrng is None:
        seedrng = np.random.randint(0, 256, 32, dtype=np.uint8)
    else:
        seedrng = np.array(seedrng, dtype=np.uint8)

    f = np.zeros(n, dtype=np.int8)
    g = np.zeros(n, dtype=np.int8)
    F = np.zeros(n, dtype=np.int8)
    G = np.zeros(n, dtype=np.int8)
    seed = np.zeros(PARAMS(logn, "lenkgseed"), dtype=np.uint8)
    q00 = np.zeros(n, dtype=np.int16)
    q01 = np.zeros(n, dtype=np.int16)
    q11 = np.zeros(n, dtype=np.int32)

    hawk.c_keygen_unpacked(
        f.ctypes.data_as(ctypes.POINTER(ctypes.c_int8)),
        g.ctypes.data_as(ctypes.POINTER(ctypes.c_int8)),
        F.ctypes.data_as(ctypes.POINTER(ctypes.c_int8)),
        G.ctypes.data_as(ctypes.POINTER(ctypes.c_int8)),
        q00.ctypes.data_as(ctypes.POINTER(ctypes.c_int16)),
        q01.ctypes.data_as(ctypes.POINTER(ctypes.c_int16)),
        q11.ctypes.data_as(ctypes.POINTER(ctypes.c_int32)),
        seed.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        logn,
        seedrng.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        len(seedrng),
    )

    pub, flag = c_encode_public(logn, q00, q01)
    if not flag:
        return c_keygen_unpacked(logn)

    priv = c_encode_private(logn, seed, F, G, pub)
    return f, g, F, G, q00, q01, q11, seed, priv, pub


def c_encode_public(logn, q00, q01):
    q00 = np.array(q00, dtype=np.int16)
    q01 = np.array(q01, dtype=np.int16)
    buf2 = np.zeros(PARAMS(logn, "lenpub"), dtype=np.uint8)
    flag = hawk.c_encode_public(
        logn,
        buf2.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        q00.ctypes.data_as(ctypes.POINTER(ctypes.c_int16)),
        q01.ctypes.data_as(ctypes.POINTER(ctypes.c_int16)),
    )
    return buf2, flag


def c_decode_public(logn, pub):
    n = 1 << logn
    pub = np.array(pub, dtype=np.uint8)
    q00 = np.zeros(n, dtype=np.int16)
    q01 = np.zeros(n, dtype=np.int16)
    hawk.c_decode_public(
        logn,
        q00.ctypes.data_as(ctypes.POINTER(ctypes.c_int16)),
        q01.ctypes.data_as(ctypes.POINTER(ctypes.c_int16)),
        pub.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
    )
    return q00, q01


def c_encode_private(logn, seed, F, G, pub):
    priv = np.zeros(PARAMS(logn, "lenpriv"), dtype=np.uint8)

    seed = np.array(seed, dtype=np.uint8)
    F = np.array(F, dtype=np.int8)
    G = np.array(G, dtype=np.int8)

    hawk.c_encode_private(
        logn,
        priv.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        seed.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        F.ctypes.data_as(ctypes.POINTER(ctypes.c_int8)),
        G.ctypes.data_as(ctypes.POINTER(ctypes.c_int8)),
        pub.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
    )
    return priv


def c_encode_sig(logn, salt, s1):
    salt_len = len(salt)

    salt = np.array(salt, dtype=np.uint8)
    s1 = np.array(s1, dtype=np.int16)
    sig = np.zeros(PARAMS(logn, "lensig"), dtype=np.uint8)

    flag = hawk.c_encode_sig(
        logn,
        sig.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        salt.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
        salt_len,
        s1.ctypes.data_as(ctypes.POINTER(ctypes.c_int16)),
    )

    return sig, flag


######################################
# Test functions
######################################


def test_verify(logn):
    n = 1 << logn

    msg = np.random.randint(0, 256, 128, dtype=np.uint8)

    priv, pub = c_keygen(logn)
    sig = c_sign(logn, priv, msg)

    err = np.random.randint(0, 2)
    sig[np.random.randint(0, len(sig))] ^= err << (np.random.randint(0, 8))

    assert hawkverify(logn, pub, msg, sig) != err


def test_sign(logn):
    msg = np.random.randint(0, 256, 128, dtype=np.uint8)
    seedrng = np.random.randint(0, 256, 32, dtype=np.uint8)
    rng = RngContext(seedrng)

    priv, _ = c_keygen(logn)

    sig = hawksign(logn, priv, msg, rng)
    sig_ref = c_sign(logn, priv, msg, seedrng)
    assert np.all(sig == sig_ref)


def test_codec_key(logn):
    _, _, F, G, q00, q01, _, kgseed_ref, priv_ref, pub_ref = c_keygen_unpacked(logn)

    pub = encode_public(logn, q00, q01)
    assert pub is not None
    Fmod2_ref = F % 2
    Gmod2_ref = G % 2

    assert np.all(pub_ref == pub), "Public key encoding failed"

    r = decode_public(logn, pub)
    assert r is not None
    q00_dec, q01_dec = r
    assert np.all(q00_dec == q00)
    assert np.all(q01_dec == q01)

    shake256 = hashlib.shake_256()
    shake256.update(pub.tobytes())
    hpub_ref = np.frombuffer(shake256.digest(PARAMS(logn, "lenhpub")), dtype=np.uint8)

    priv = encode_private(kgseed_ref, F, G, hpub_ref)

    assert np.all(priv_ref == priv)

    kgseed, Fmod2, Gmod2, hpub = decode_private(logn, priv)
    assert np.all(kgseed == kgseed_ref)
    assert np.all(Fmod2 == Fmod2_ref)
    assert np.all(Gmod2 == Gmod2_ref)
    assert np.all(hpub == hpub_ref)


def test_keygen_unpacked(logn):
    seed = np.random.randint(0, 256, 40, dtype=np.uint8)
    rng = RngContext(seed)
    msg = np.random.randint(0, 256, 128, dtype=np.uint8)

    priv, pub = hawkkeygen(logn, rng)
    sig = c_sign(logn, priv, msg)

    err = np.random.randint(0, 2, dtype=np.uint8)
    if err:
        msg = np.random.randint(0, 256, 128, dtype=np.uint8)

    v = c_verify(logn, pub, msg, sig)
    assert v != err, "Did not verified correctly"


if __name__ == "__main__":
    NTEST_CODEC = 100
    NTEST_SIGN = 100
    NTEST_KEYGEN = 100
    NTEST_VERIFY = 100

    logns = [8,9,10]
    # np.random.seed(0)

    for logn in logns:
        for _ in tqdm(range(NTEST_CODEC), desc=f"Hawk-{1<<logn} (encoding)"):
            test_codec_key(logn)

    for logn in logns:
        for _ in tqdm(range(NTEST_VERIFY), desc=f"Hawk-{1<<logn} (verify)"):
            test_verify(logn)

    for logn in logns:
        for _ in tqdm(range(NTEST_SIGN), desc=f"Hawk-{1<<logn} (sign)"):
            test_sign(logn)

    for logn in logns:
        for _ in tqdm(range(NTEST_KEYGEN), desc=f"Hawk-{1<<logn} (keygen)"):
            test_keygen_unpacked(logn)
