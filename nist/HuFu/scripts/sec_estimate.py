""" Security estimates."""
from math import *
import prettytable as pt

def delta_beta(b):
    """ Compute root Hermite factor delta on input BKZ block size b """
    return (((pi * b) ** (1. / b) * b) / (2 * pi * exp(1))) ** (0.5 / (b - 1))
    
def classic_core_svp(b):
    """ Compute classical cost of SVP in dim-b """
    return 0.292 * b

def quantum_core_svp(b):
    """ Compute quantum cost of SVP in dim-b """
    return 0.265 * b

def smooth(logeps, n):
    """ Compute the smoothing parameter of Z^n with epsilon
    Args:
        logeps: log_2(epsilon)
        n: dimension
    Returns:
        eta_epsilon(Z^n)
    """
    return sqrt(log(2 * n) + logeps * log(2)) / sqrt(2 * pi * pi)

def sis_attack(m, n, Q, sig_norm):  
    """ The cost of forgery attack
    Args:
        m: row number
        n: column number of A'
        Q: modulus
        sig_norm: length of the preimage (z0 + e, z_1, z_2)
    Returns:
        bit-security in the classical and quantum setting
    """
    Dim = 2 * m + n
    the_k = 0
    min_beta = 3000
    for k in range(n + m):
        dim = Dim - k
        for beta in range(200, 2000):
            delta = delta_beta(beta)
            Vol_root = Q ** (1. * m / dim)
            LHS = (delta ** dim) * Vol_root
            RHS = sig_norm
            if RHS > LHS:
                if min_beta > beta:
                    min_beta = beta
                    the_k = k
                break
    print("SIS:", min_beta)
    return (round(classic_core_svp(min_beta)), round(quantum_core_svp(min_beta)))

def lwe_attack(m, n, Q, sigma_f): 
    """ The cost of key-recovery attack 
    Args:
        m: number of samples
        n: dimension of secrets
        Q: modulus
    Returns:
        bit-security in the classical/quantum setting
    """
    classical_cost = []
    quantum_cost = []
    for l in range(m): # Use only (m-l) LWE samples
        dim = m + n - l
        beta = 200
        while (1):
            delta = delta_beta(beta)
            LHS = sqrt(beta) * sigma_f * sqrt(3. / 4)  # the faoctor sqrt(3/4) for a more conservative security
            RHS = Q ** (1. * (m - l) / (dim + 1)) * delta ** (2 * beta - dim - 1)
            if RHS > LHS: break
            else: beta += 1
        classical_cost.append(classic_core_svp(beta))
        quantum_cost.append(quantum_core_svp(beta))
    print("LWE:", min(classical_cost) / 0.292)
    return round(min(classical_cost)), round(min(quantum_cost))


def comp_parmas_Ours(m, n, p, q, sigma_f):
    """ Compute related parameters
    Args:
        m: row number
        n: dimension of LWE
        p: a parameter that determins the standard deviation of the approximate error e
        sigma_f: standard deviation of the secrets and errors
    Returns:
        beta: upper-bound of the scaled preimage (z0 + e, gamma * z_1, gamma * z_2)
        gamma: a scaler, defined by sqrt(variance(z0 + e) / variance(z0))
        sigma_pre: the standard deviation of the preimage (z_0, z_1, z_2)
    """
    r = smooth(49, 1)
    sigma_err = sqrt((p ** 2 - 1) / 12)
    sigma_pre = sqrt(q**2 + 1) / q * sigma_f * (sqrt(m) + sqrt(n + m)) * q * r * 1.05
    sigma_mix = sqrt(sigma_err ** 2 + sigma_pre ** 2)
    beta = 1.04 * sqrt(m * sigma_mix ** 2 + (m + n) * sigma_pre ** 2)
    beta = int(beta) 
    return round(beta, 1), round(sigma_mix, 2), round(sigma_pre, 1)

def preimage_size(n, std):
    """ Compute the storage size (in byte) of dim-n Gaussian vector with standard deviation std
    Args:
        n: dimension
        std: standard deviation of Gaussian
    Returns:
        number of bytes to store dim-n Gaussian vector
    """
    entropy = ceil((0.5 + log(sqrt(2 * pi) * std)) / log(2) * n)
    return entropy / 8
    

def main():
    classical_key_rec_sec, quantum_key_rec_sec = 0, 0
    tb = pt.PrettyTable()
    tb.field_names = ["(m, n, p, q, Q)", "sigma_pre", "beta", "beta / Q", "Forgery (C/Q)", "Key Recovery(C/Q)", "|pk|(KB)", "|sig|(B)"]
    tb.title = "Parameters of HuFu"
    r = smooth(49, 1)
    
    # NIST-1
    m, n, p, q = 736, 848, 2 ** 12, 2 ** 4
    Q = p * q
    sigma_f = sqrt(1/2)

    beta, sigma_mix, sigma_pre = comp_parmas_Ours(m, n, p, q, sigma_f)
    classical_forg_sec, quantum_forg_sec = sis_attack(m, n, Q, beta)  # Forgery attack
    classical_key_rec_sec, quantum_key_rec_sec = lwe_attack(m, n, Q, sigma_f) # Key Recovery attack
    
    pk_size = ceil((m * m * ceil(log(Q, 2)) + 256) / 8 / 1024)
    sig_size = ceil(preimage_size(m + n, sigma_pre) + 40)
    
    tb.add_row([(m, n, p, q, Q), sigma_pre, round(beta, 2), round(beta / Q, 2), "%d / %d" % (classical_forg_sec, quantum_forg_sec), "%d / %d" % (classical_key_rec_sec, quantum_key_rec_sec), pk_size, sig_size])
    
    # NIST-3
    m, n, p, q = 1024, 1232, 2 ** 13, 2 ** 4
    Q = p * q
    sigma_f = sqrt(1/2)

    beta, sigma_mix, sigma_pre = comp_parmas_Ours(m, n, p, q, sigma_f)
    classical_forg_sec, quantum_forg_sec = sis_attack(m, n, Q, beta)  # Forgery attack
    classical_key_rec_sec, quantum_key_rec_sec = lwe_attack(m, n, Q, sigma_f) # Key Recovery attack
    
    pk_size = ceil((m * m * ceil(log(Q, 2)) + 256) / 8 / 1024)
    sig_size = ceil(preimage_size(m + n, sigma_pre) + 40)
    
    tb.add_row([(m, n, p, q, Q), sigma_pre, round(beta, 2), round(beta / Q, 2), "%d / %d" % (classical_forg_sec, quantum_forg_sec), "%d / %d" % (classical_key_rec_sec, quantum_key_rec_sec), pk_size, sig_size])
    
    # NIST-5
    m, n, p, q = 1312, 1552, 2 ** 13, 2 ** 4
    Q = p * q
    sigma_f = sqrt(1/2)

    beta, sigma_mix, sigma_pre = comp_parmas_Ours(m, n, p, q, sigma_f)
    classical_forg_sec, quantum_forg_sec = sis_attack(m, n, Q, beta)  # Forgery attack
    classical_key_rec_sec, quantum_key_rec_sec = lwe_attack(m, n, Q, sigma_f) # Key Recovery attack
    
    pk_size = ceil((m * m * ceil(log(Q, 2)) + 256) / 8 / 1024)
    sig_size = ceil(preimage_size(m + n, sigma_pre) + 40)
    
    tb.add_row([(m, n, p, q, Q), sigma_pre, round(beta, 2), round(beta / Q, 2), "%d / %d" % (classical_forg_sec, quantum_forg_sec), "%d / %d" % (classical_key_rec_sec, quantum_key_rec_sec), pk_size, sig_size])
    

    print(tb)
    
if __name__ == '__main__':
  main()
