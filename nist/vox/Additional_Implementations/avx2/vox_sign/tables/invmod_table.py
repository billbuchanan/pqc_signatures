#!/usr/bin/env python3

def print_invmod_table(p):
    print(" {", end="")
    for i in range(p):
        if (i % 16 == 0):
            print("\n  ", end="")
        print("%4d," % pow(i, p-2, p), end="")
    print("\n};")

for q in [251, 1021, 4093]:
    print("#if VOX_Q ==", q);
    print("static const uint16_t invmod_table[VOX_Q] =")
    print_invmod_table(q);
    print("#endif /* VOX_Q == %d */\n" % q)
