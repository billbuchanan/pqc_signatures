# %%
from utils import *
import sys

SIGMA =float(sys.argv[1]) # 1018.8 #
SAMPLE_COUNT_POWER = 24
KEEP_BITS = 7
QUANT_SCALE = 12

SAMPLE_COUNT = 2**SAMPLE_COUNT_POWER
TRUNC = int(SIGMA*10)
HIGHBITS_TRUC = TRUNC >> KEEP_BITS
# %%
print("#define TABLE_INFO \"SIGMA:",SIGMA, "TRUNC:",TRUNC, "QUANT_SCALE:", QUANT_SCALE,"SAMPLE_COUNT_POWER:",SAMPLE_COUNT_POWER,"KEEP_BITS",KEEP_BITS,"HIGHBITS_TRUC",HIGHBITS_TRUC,"\"")
A = dgg(SAMPLE_COUNT,SIGMA)
# %%
_,TO_COMP_TRAIN = split_keep_to_comp(A,KEEP_BITS)
# %%
freq_to_comp = calc_freq(TO_COMP_TRAIN,HIGHBITS_TRUC,0)
freq_quat_to_comp = quat_freq(freq_to_comp,QUANT_SCALE)
# %%
print("#define SCALE_BITS",QUANT_SCALE)
print_freq_quat(freq_quat_to_comp)
# %%