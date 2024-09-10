
def print_list(l):
    i=0
    for x in l:
        print(x, end=",")
        i += 1
        if (i==20):
            print("\n  ", end="")
            i = 0
    if (i):
        print()


def zetas(p, z):
    l1 = [0] * p
    l2 = [0] * (p-1)
    z2 = 1
    for i in range(p-1):
        l1[z2] = i
        l2[i]  = z2
        z2 = (z2*z) % p
    return (l1, l2)

for (p,z) in [(251,6)]: #, (1021,10), (4093,2)]:
    (z1,z2) = zetas(p,z)
    (n,idx) = mirror_idx(FFT_split(p))
    assert(n == p-1)
    roots = [ z2[i] for i in idx ]
    print("#if (VOX_Q == %d)" % p)
    print("static const Fq zetas_idx[%d] = {" % p, end="\n  ")
    print_list(z1)
    print("};")
    print("static const Fq zetas[%d] = {" % (p-1), end="\n  ")
    print_list(z2)
    print("};")
    print("#endif\n")
