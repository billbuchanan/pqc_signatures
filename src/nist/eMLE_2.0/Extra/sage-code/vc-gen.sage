#!/usr/bin/env sage

load('impl-vc.sage')


#set_random_seed(0)

n,d,c_max, x_max, p_max = 64,  3, 4, 4, 26  
#n,d,c_max, x_max, p_max = 96, 3, 4, 4, 28
#n,d,c_max, x_max, p_max = 128,3, 4, 4, 30

q = 2^30 

p = mkP()


sVarList =[]
tVarList =[]


aa = 500
bb = 20

for j in range(aa):
    x1, h1, x2, h2, F1, F2 = keygen()

    G = mkG()

    correct=0
    count=0
    sum  = 0



    for i in range(bb):
        u, s =sign(x1, x2, F1, F2, h1, h2, i)

        sVar,  tVar =verify(i,h1, h2, u, s)
        sVarList = sVarList + [sVar]
        tVarList = tVarList +[tVar]


sVarList = sorted(sVarList)
tVarList = sorted(tVarList)

vc = []

len = int((aa*bb)*0.01)
vc = vc + [int(sumList(sVarList[len:2*len], len)/len)]

len = int((aa*bb)*0.01)
vc = vc + [int(sumList(sVarList[aa*bb-2*len:aa*bb-len], len)/len)]

len = int((aa*bb)*0.02)
vc = vc + [int(sumList(tVarList[len:2*len], len)/len)]

len = int((aa*bb)*0.04)
vc = vc + [int(sumList( tVarList[aa*bb-2*len:aa*bb-len], len)/len)]


print ("-------------------------p: ", p)
print (vc)
