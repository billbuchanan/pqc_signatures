#!/usr/bin/env sage

load('impl-gen.sage')
#del x   # forget private key

def eMLE_att(x, o, a):
    p = mkP()
    G = mkG()	
    h = vector(ZZ, [0 for _ in range(n)])
    E = []
    for i in range(d):
        if i==0:
            h = (h+mul(G[i], x+o, n, p[i]))%p[i]
        else:
            h = (h+mul(G[i], x, n, p[i]))%p[i]  
        if i < d-1:
            E = E + [h]

    return h, E


def sign_att(x1, x2, EE1, EE2, h1_org, h2_org, m):
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

    cp1, cp2 = mkVecC(str(m)+str(h1_org)+str(h2_org))

    while 1:

        y = vector(ZZ, [ randint(64, 512-64) for _ in range(n)])
        u, E = eMLE_att(y, cp1+cp2, 1 ) 
        c1, c2 = mkVecC(str(m)+str(u)+str(h1_org)+str(h2_org))
        s = mul(x1, c1, n, 0)+ mul(x2, c2, n, 0) + y 

        on = true

        if on:
            break 

    return u, s

def checkX(h, x):
	p = mkP()
	G = mkG()

	t = h
	for i in range(d):			
		if d-i-1  ==0:
			g = mul(G[d-i-1], x+G[1], n, p[d-i-1])	 	
			t = (t-g)%p[d-i-1]
		else:
			t = (t-mul(G[d-i-1], x, n, p[d-i-1]))%p[d-i-1]
			if i>0:
				for j in range(n):
					if t[j] > p[d-i]//2:
						t[j] = t[j] - p[d-i]


	print ("check x------------")
	print (f'x correctness:  \x1b[32m{t}\x1b[0m')

########################################
def attack(pp, GG, h, k0Avg, k1Avg, withExtraConstraint, shift):
	print ("attack starts ..........")
	#print (h)


	R = PolynomialRing(ZZ, [f'x{i}' for i in range(n)] + [f'k{i}_{j}' for i in range(d) for j in range(n)])
	xs,ks = R.gens()[:n], R.gens()[n:]
	ks = [vector(R, ks[i:i+n]) for i in range(0,len(ks),n)]

	ineqs = []
	ineqs2 = []

	# eMLE forward
	y = vector(R, [0]*n)
	x = vector(R, xs)
	for i in range(d-1):
		y = y + GG[i]*x - ks[i]*pp[i]
		ineqs += [(f, pp[i]) for f in y]
	
	y = y + GG[d-1]*x - ks[d-1]*pp[d-1]
	ineqs2 += [(f, pp[d-1]) for f in y]

	eqs = list(y - h)

	#print('-'*64)
	#for eq in eqs:
		#print(eq)
	#print('-'*64)

	#for ineq in ineqs:
	#	#print(ineq)
	#	assert not ineq[0].constant_coefficient()  # lazy coding
	#print('-'*64)

	########################################adapted from Lorez Panny's attack 

	#big = 2**(99+ceil(log(pp[-1],2)))   # or something

	#       1         eqs         xs                    ks                                 G*x small        G*x-k*p < p
	#wei = [big^2] + [big^2]*n + [big/pp[-1]]*n + sum(([big/pp[-1]]*n for p in pp),[]) + [big/pp[-1]]*n + [big/m for _,m in ineqs]
	#est =                       [pp[-1]//2]*n + sum(([pp[-1]//n//2]*n for p in pp),[]) + [pp[-2]//2]*n + [m//2 for _,m in ineqs]
	#mat = matrix(ZZ, [
	#		[1] + [eq.constant_coefficient( ) for eq in eqs] + [ -c  for c in est]
	#	] + [
	#		[0] + [eq.monomial_coefficient(v) for eq in eqs] + [j==i for j in range((d+1)*n)]  + [(row*x).monomial_coefficient(v) for row in GG[-1].rows()] + [f.monomial_coefficient(v) for f,_ in ineqs]
	#		for i,v in enumerate(R.gens())
	#] + [
	#		# G*x small
	#		[0] + [0]*n + [0]*(d+1)*n + [(j==i)*pp[-1] for j in range(n)] + [0]*len(ineqs)
	#		for i in range(n)
	#	])



	big = 2**(99+ceil(log(pp[-1],2)))   # or something

	#       1         eqs         xs                    ks                                 G*x small        G*x-k*p < p
	#wei = [big] + [big]*n + [2^12]*n + [1]*n + [1]*n + [1]*n + [1]*n +[2^24]*n + [2^8]*n
	wei = [big] + [big]*n + [1]*n + [1]*n + [1]*n + [1]*n + [1]*n +[2^24]*n + [2^4]*n

	est =                       [0//2]*n + sum(([pp[-1]//n//2]*n for p in pp),[]) + [0]*n + ([pp[0]//(2)]*n) + [0]*n #pp[2]//(2)
	for i in range(n):
		est[i] = 0 
		est[n+i] =  k0Avg 
		est[2*n+i] = k1Avg 
	
	#print ("est: ", est[:3*n])

	mat = matrix(ZZ, [
			[1] + [eq.constant_coefficient( ) for eq in eqs] + [ -c  for c in est]
		] + [
			[0] + [eq.monomial_coefficient(v) for eq in eqs] + [j==i for j in range((d+1)*n)]  + [(row*x).monomial_coefficient(v) for row in GG[-1].rows()] + [f.monomial_coefficient(v) for f,_ in ineqs]
			for i,v in enumerate(R.gens())
	] + [
			# G*x small
			[0] + [0]*n + [0]*(d+1)*n + [(j==i)*pp[-1] for j in range(n)] + [0]*len(ineqs) 
			for i in range(n)
		])

	mat = mat.transpose()
	zero  = vector(ZZ, [0 for _ in range(len(mat[0]))]) 

	for col in range(1*n): 
		mat[4*n+1+col, :] = zero
		mat[5*n+1+col, :] = zero

	if withExtraConstraint==0:
		for col in range(1*n): 
			mat[6*n+1+col, :] = 0
			mat[7*n+1+col, :] = 0

	if withExtraConstraint==1:
		for col in range(1*n): 
			mat[7*n+1+col, :] = 0

	if withExtraConstraint==2:
		for col in range(1*n): 
			mat[6*n+1+col, :] = 0


	mat = mat.transpose()
	#print ("inital mat: ")
	#print(mat)

	S = diagonal_matrix(ZZ, map(ceil, wei))

	#mat = ((mat * S).LLL() * ~S).change_ring(ZZ)
	mat = ((mat * S).BKZ(block_size  = 15, fp='rr', precision= 88) * ~S).change_ring(ZZ)  #, fp='rr', precision= 64
	#print ("---- reduced Matrix:")
	#print (mat)

	for row in mat:
		if row[0]:
			assert abs(row[0]) == 1 and row[1:1+n] == 0
			xx = sgn(row[0]) * row[1+n:1+2*n] + vector(est[:n])
			xxx = sgn(row[0]) * row[1+n:1+4*n] + vector(est[:3*n])
			print ("++++++++++++++++++++++++++ v norm : ", float(xxx[:3*n].norm()))
			#print(row)
			#print (xx)
			#print (xxx)
			break
	else: assert False

	print('-'*64)
	print(f'maybe x:  \x1b[32m{xx}\x1b[0m')
	#print(f'maybe x extra:  \x1b[32m{((xxx[n:3*n]))}\x1b[0m')
	

	return 0, xx, xxx[n:3*n]


#set_random_seed(11)

n,d,c_max, x_max, p_max = 64,  3, 4, 4, 26  
#n,d,c_max, x_max, p_max = 96, 3, 4, 4, 28
#n,d,c_max, x_max, p_max = 128,3, 4, 4, 30

q = 2^30

sig = 0
same  =0

for i in range(1):
	x1_org, h1_org, x2_org, h2_org, E1_org, E2_org = keygen()

	pp = mkP()
	G = mkG()

	o =  G[1]	
	shift = mul(G[0], o, n, pp[0])

	GG = [] 
	for l in range(d):
		g = [G[l][n-j-1] for j in range(n-1)]
		g = [G[l][0]] + g

		GG = GG+ [matrix.circulant(g)]

	withExtraConstraint = 3



	for k0 in range(1):
		for k1 in range(1): 

			_, xx1, xxx1 = attack(pp, GG, h1_org-shift,  26, 0, withExtraConstraint, shift) 
			checkX(h1_org, xx1)
			_, xx2, xxx2 = attack(pp, GG, h2_org-shift,  26, 0, withExtraConstraint, shift)
			checkX(h2_org, xx2)
			print ("original x1_org, x2_org:", x1_org, x2_org)
			print ("original x1_org norm, x2_org norm:", float(x1_org.norm()), float(x2_org.norm()))

			if (x1_org[0] == xx1[0] and x1_org[1] == xx1[1]) or (x2_org[0] == xx2[0] and x2_org[1] == xx2[1]):
				same = same + 1
				break
		if same >=1:
			break
	if same >=1:
		break

print ("sumary: ", n, same, "sig", sig)


