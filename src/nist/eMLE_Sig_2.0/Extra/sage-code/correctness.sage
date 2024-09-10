load('impl-gen.sage')


#set_random_seed(0)

n,d,c_max, x_max, p_max = 64,  3, 4, 4, 26  
#n,d,c_max, x_max, p_max = 96, 3, 4, 4, 28
#n,d,c_max, x_max, p_max = 128,3, 4, 4, 30

#n,d,c_max, x_max, p_max = 64,  3, 4, 4, 32  
#n,d,c_max, x_max, p_max = 96, 3, 4, 4, 32
#n,d,c_max, x_max, p_max = 128,3, 4, 4, 32

#n,d,c_max, x_max, p_max = 64,  4, 4, 4, 40  

q = 2^48

p = mkP()



R =[]
for j in range(1):
    x1, h1, x2, h2, F1, F2 = keygen()

    G = mkG()
    print ("p: ", p)
    #print ("G0: ")
    #print (G[0])

    #print ("G1: ")
    #print (G[1])

    #print ("G2: ")
    #print (G[2])
    correct=0
    count=0
    sum  = 0

    T = []
    for i in range(1):
        u, s, trials =sign(x1, x2, F1, F2, h1, h2, i)
        sum = sum + trials
        T = T + [trials]
        #print ("-----------: s, u")
        #print (s)
        #print (u)
        r=verify(i,h1, h2, u, s)
        count = count +1
        if r:
            correct=correct+1

    R = R + [(j, float(correct/count), float(sum/count))]
    print ("trlias: ", j, T)
    if float(sum/count) > 4:
        print ("big one: ", i, j, float(sum/count), T)
        break

print("correctness ratio, trials ratio:", R)

