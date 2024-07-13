# ===============================
# eMLE-Sig 2.0
# ===============================
from hashlib import sha256, sha3_256, sha3_512

#set_random_seed(1)

def sumList(l, n):
    sum = 0
    for i in range(n):
        sum = sum + l[i]
    return sum


def mul(a, b, n, m):

    if m==5:
        A = matrix.circulant(a)
        cp = (b)*A

        cp1 = cp%m
        if m==5:
            K = (cp - cp1)/m
            #print ("K0: ", K)

    c = [0 for _ in range(n)]
    noise = vector(ZZ, [0 for _ in range(n)])
    for i in range(n):
        c[i] = 0 
        for j in range(n):
            c[i] = (c[i]+a[j]*b[(i-j)%n])
            if m>0:
                c[i] = c[i]%m 

    return vector(ZZ, c)


def mkVecC(msg): #only for reference
    c1 = vector(ZZ, [0 for i in range(n)])
    c2 = vector(ZZ, [0 for i in range(n)])
    for i in range(n):
        hv = sha3_256((str(i)+msg).encode()).hexdigest()
        c1[i] =  int(hv[0:8], 16)%(c_max)
        c2[i] =  int(hv[8:16], 16)%(c_max)

    return c1, c2 

def mkP():
    p = vector(ZZ, [0 for _ in range(d)])
    for i in range(d):
        if i == 0:
            p[i] = next_prime(c_max)
        if i== d-1:
                p[i] = 2^p_max 

        if i>1 and i < d-1:
            p[i] = next_prime((n)*(c_max-1)*(p[i-1])+p[i-1] ) # -2*n*p[i-1]

        if i==1:
            p[i] = next_prime((n//2)*(c_max-1)*(p[i-1])+p[i-1]+n) # -2*n*p[i-1]

    if n==6 or n==8 or n==10:
        p[1] = 557 

    return p

def mkG():
    p = mkP()
    G = []
    for l in range(d):
        g = vector(ZZ, [(int(sha3_256((str(l)+str(k)+str(n)+str(d)+str(c_max)+str(x_max)+str(p)).encode()).hexdigest(),16))%p[l] for k in range(n)] )
        G = G + [g]

    return G

def eMLE(x, o, a):
    p = mkP()
    G = mkG()	
    h = vector(ZZ, [0 for _ in range(n)])
    sumR = 0
    F  = []
    for i in range(d):
        if i==0:
            h = (h+mul(G[i], x+(o%p[0]), n, p[i]))%p[i]
        else:
            h = (h+mul(G[i], x, n, p[i]))%p[i]        

        if (i ==d-2 ):   
            num = int(((int(p[i+1]))-sumList(h, n)*(c_max-1))/(c_max*p[i])) 
            if num < 0:
                num = 0
            if a==1:
                num = (2*num)
            t = num
            locations  = int(n/(2))
            print ("noise: ", i, num)  
            k = vector(ZZ, [0 for _ in range(n) ])
            w = vector (ZZ, [randint(0, n-1) for _ in range(locations)])   
            for j in range(locations-1):
                if int(num/(locations-j)) > 0:
                    r = randint(0, int(num/(locations-j)))
                else:
                    r = 0    
                h[w[j]] = h[w[j]]+(r%q)*p[i]
                k[w[j]] = k[w[j]] +(r%q)  
                num = num - r
            
            h[w[locations-1]] = h[w[locations-1]]+(num%q)*p[i]
            k[w[locations-1]] = k[w[locations-1]] + (num%q) 

            w = vector (ZZ, [randint(0, n-1) for _ in range(2)])
            r = randint(0, int(t/3))
            for j in range(n):
                    if h[(j+w[0])%n] < p[i]:
                        h[(j+w[0])%n] = h[(j+w[0])%n]-(r%q)*p[i]
                        k[(j+w[0])%n] = k[(j+w[0])%n] - (r%q)
                        break
            r = int(t/3) - r
            for j in range(n):                    
                    if h[(j+w[1])%n] < p[i] and h[(j+w[1])%n]>0:
                        h[(j+w[1])%n] = h[(j+w[1])%n]-(r%q)*p[i]
                        k[(j+w[1])%n] = k[(j+w[1])%n] - (r%q)
                        break

            for j in range(n):
                    if h[j] < p[i] and h[j] > 0:
                        if a==1:
                            r = randint(-32*int(n), 32*int(n))
                        else:
                            r = randint(-16*int(n), 16*int(n))
                            sumR = sumR + r
                        if r>0:
                            h[(j)] = h[(j)]+(r%q)*p[i]
                            k[(j)] = k[(j)] + (r%q)
                        else:
                            h[(j)] = h[(j)]-((-r)%q)*p[i]
                            k[(j)] = k[(j)] - ((-r)%q)   
                        
            if a==0:
                print (k)
                bigNoise = 0
                security_guess_r = 0 
                if n==64:
                    for j in range(n):
                        if abs(k[j]) > 912:
                            bigNoise = bigNoise +1
                            security_guess_r = security_guess_r + log(abs(k[j])-912, 2)

                if n==96:
                    for j in range(n):
                        if abs(k[j]) > 912:
                            bigNoise = bigNoise +1
                            security_guess_r = security_guess_r + log(abs(k[j])-912, 2)

                if n==128:
                    for j in range(n):
                        if abs(k[j]) > 912:
                            bigNoise = bigNoise +1
                            security_guess_r = security_guess_r + log(abs(k[j])-912, 2)

                print ("****************noise norm: ", i, float(k.norm()),"secuirty bits: ", float(security_guess_r)+float(log(float(factorial(n+bigNoise-1)/(factorial(bigNoise)*factorial(n-1))),2)), float(log(9^n, 2)))

        F = F + [h]

        #if i == 0:
        #print ("h value layer: ", i, (h))  

    return h, F, sumR

def keygen():
    p = mkP()
    G = mkG()



    while 1:
        x1 = vector(ZZ, [ randint(-x_max, x_max) for _ in range(n)])
        x2 = vector(ZZ, [ randint(-x_max, x_max) for _ in range(n)])
        sumX = sumList(x1+x2, n) #+ sumList(x2, n)
        if sumX < 0:
            sumX = -sumX;

        if sumX < n//2 :
            break

    print ("call eMLE in keygen: ")
    while 1:        
        h1, F1, sumR1 = eMLE(x1, G[1], 0)
        h2, F2, sumR2 = eMLE(x2, G[1], 0)
        sumR = sumR1+sumR2
        if sumR < 0:
            sumR = -sumR;

        print ("sumR1, R2, sum: ", sumR1, sumR2, sumR1+sumR2, sumR)
        if sumR < n*n:
            break


    return x1, h1, x2,  h2, F1, F2

def sign(x1, x2, F1, F2, h1, h2, m):
    p = mkP()
    G = mkG()

    sumX  = 0 
    sumXp  = 0 
    for i in range(n):
        if x1[i] < 0:
            sumX = sumX + x1[i]     
        if x2[i] < 0:
            sumX = sumX + x2[i] 

    sumXp  = 0 
    for i in range(n):
        if x1[i] > 0:
            sumXp = sumXp + x1[i]     
        if x2[i] > 0:
            sumXp = sumXp + x2[i] 

    trials =0
    cp1, cp2 = mkVecC(str(m)+str(h1)+str(h2))
    

    while 1:
        trials = trials + 1 
        if sumXp > -sumX:
            y_min = randint(int(-sumX*c_max/10), int(-1*sumX*c_max/8))
            y_gap = randint(int(1*sumXp*c_max/7), int(1*sumXp*c_max/5))
        else:
            y_min = randint(int(-1*sumX*c_max/7), int(-1*sumX*c_max/5))
            y_gap = randint(int(1*sumXp*c_max/10), int(1*sumXp*c_max/8))
        
        y = vector(ZZ, [ randint(y_min, n*c_max*x_max//2 - y_gap) for _ in range(n)]) 
        u, F3, _ = eMLE(y, cp1+cp2, 1)       
        
        c1, c2 = mkVecC(str(m)+str(u)+str(h1)+str(h2))
        s = mul(x1, c1, n, 0)+ mul(x2, c2, n, 0) + y 


        valid = ckSize(s, 0, n*c_max*x_max//2-1)
        if not valid:
            continue


        valid = true
        for l in range(d-1):
            L = mul(F1[l], c1, n, 0)+mul(F2[l], c2, n, 0)+F3[l]
            valid = valid and ckSize(L, 0, p[l+1])                


        if valid:
            break 

    return u, s

def ckSize(s, min, max):
    v = True
    for i in range(n):
        v = v and s[i] <= max and s[i] >= min 
    return v

def verify(m, h1, h2, u, s):
    p = mkP()
    G = mkG()

    cp1, cp2 = mkVecC(str(m)+str(h1)+str(h2))

    c1, c2 = mkVecC(str(m)+str(u)+str(h1)+str(h2))

    a = int(sumList(s, n)/n)
    s1 = vector(ZZ, [ (s[k] - a) for k in range(n)])
    sVar = s1.norm()*s1.norm()

    t = mul(h1, c1, n, p[d-1]) + mul(h2, c2, n, p[d-1]) + u
    for i in range(d):			
        if d-i-1  ==0:
            #print ("raw k0: ", t)
            g = mul(G[1], c1+c2, n, p[d-i-1])
            K0a = (t-mul(G[d-i-1], s+g+cp1+cp2, n, p[d-i-1]))/p[d-i-1]
            K0 = vector(ZZ, [int(K0a[k]) for k in range(n)])

            a = int(sumList(K0, n)/n)
            k01 = vector(ZZ, [ (K0[k] - a) for k in range(n)])

            tVar = k01.norm()*k01.norm()

            t = (t-mul(G[d-i-1], s+g+cp1+cp2, n, p[d-i-1]))%p[d-i-1]
        else:
            t = (t-mul(G[d-i-1], s, n, p[d-i-1]))%p[d-i-1]

    #print ("Verification layer 0------------")
    print (f't:  \x1b[32m{t}\x1b[0m')	 	
    return sVar, tVar
