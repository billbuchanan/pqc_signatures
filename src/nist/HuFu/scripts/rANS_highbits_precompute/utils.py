import pandas as pd
from sage.all_cmdline import *   # import sage library
from sage.stats.distributions.discrete_gaussian_lattice import DiscreteGaussianDistributionLatticeSampler
from tqdm import trange, tqdm

_PER_SAMPLE_COUNT = 2**6

def dgg(length,_sigma):
    _DGG = DiscreteGaussianDistributionLatticeSampler(ZZ**_PER_SAMPLE_COUNT, _sigma)
    c = ceil(length/_PER_SAMPLE_COUNT)
# A = [d for _ in trange(c) for d in np.random.normal(scale=SIGMA,size=n).astype(np.int64)]
    return [d for _ in trange(c,desc="dgg") for d in _DGG()][:length]

def quat_freq(_freq, _quat_scale):
    '''
    quantlize freq to given precision.
    Input: an array
    freq[0]=233 means 0 occurs 233 times
    '''
    _df = pd.DataFrame(_freq).value_counts().sort_index().reset_index(name="val")

    _v, _c = _df[0], _df["val"]
    _d = {}
    _prev_quat = 1


    _left = 2**_quat_scale
    _left_count = sum(_freq)

    for _a, _b in zip(_v, _c):
        _x = max(_prev_quat, int((_a/_left_count)*_left))
        _prev_quat = _x
        _d[_a] = _x
        _left -= _x*_b
        _left_count -= _a*_b
    
    _freq_quat = [_d[f] for f in _freq]
    if _left!=0:
        raise Exception(f"Left: {_left} should be 0! check quat scale!")
    return _freq_quat
def calc_freq(_vals,_range, _shift):
    _freq = [0]*_range
    for _a in tqdm(_vals,desc="calc_freq"):
        _freq[_a+_shift] += 1
    return _freq

def print_freq_quat(_freq_quat):
    print("#define M_SIG ",len(_freq_quat))
    print("static uint32_t f_sig[M_SIG] = {")
    for _a in _freq_quat:
        print(_a,",",end="",sep="")
    print("};")
def split_keep_to_comp(_vals,_keep_bits):
    _keeps = []
    _to_comps = []
    for a in _vals:
        _sign = 1 if a<0 else 0
        _val = abs(a)
        _keep = (_val&(2**_keep_bits-1)) | _sign << _keep_bits
        _to_comp = _val >> _keep_bits
        _keeps.append(_keep)
        _to_comps.append(_to_comp)
    return _keeps,_to_comps