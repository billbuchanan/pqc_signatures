"""
File: Compute_Sizes.py
Author: EagleSign
Date: 2023-06-19

Description: Computation of EagleSign Public Key and Signature Sizes
Usage: python Compute_Sizes.py

"""

if __name__ == "__main__":

    # Parameters definition in the format 
    # (n, q, k, l, eta_y1, eta_y2, eta_g, eta_d, t, tau)
    # per Nist Security Levels
    params = {
        2: (512, 12289, 2, 1, 1, 64, 6, 6, 140, 18),
        3: (1024, 12289, 1, 1, 1, 64, 1, 1, 140, 38),
        "3+": (512, 12289, 2, 2, 1, 32, 2, 1, 90, 18),
        5: (1024, 12289, 1, 2, 1, 32, 1, 1, 86, 18),
    }

    # Computing the sizes for each level
    print("Eagle\t|\tdelta\t|\tlog_delta|\tdelta_p\t|\tlog_delta_p|\t|Sig|\t|\t|Pk|")
    print("-------------------------------------------\
--------------------------------------------------\
--------------")
    for idx in params.keys():
        param = params[idx]
        
        delta = param[3]*param[6]*(param[8]+param[9])
        delta_p = param[3]*param[7]*(param[8]+param[9]) + param[5]

        # Computing logdelta = bitLength(1+2*delta) and 
        # logdelta_prime = bitLength(1+2*delta_p)
        logdelta = int(2*delta+1).bit_length()
        logdelta_p = int(2*delta_p+1).bit_length()

        # Computing |Sig| and |Pk|
        sigma = 32 + (param[3]*logdelta + param[2]*logdelta_p)*param[0]/8
        pk = 32 + (param[2]*param[3]*int(param[1]).bit_length())* param[0]/8
        print("{}\t|\t{}\t|\t{}\t |\t{}\t|\t{}\t   |\t{}\t|\t{}".format(idx, delta, logdelta, delta_p, logdelta_p, int(sigma), int(pk)))
        print("-------------------------------------------\
--------------------------------------------------\
--------------")
