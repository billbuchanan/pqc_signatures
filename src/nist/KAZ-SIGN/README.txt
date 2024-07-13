KAZ-SIGN
========

This archive contains the following files and directories:

Reference_Implementation/
    KAZ458/
        Reference implementation of KAZ-SIGN with recommended
        parameters "N=458", using NIST API (api.h).

    KAZ738/
        Reference implementation of KAZ-SIGN with recommended
        parameters "N=738", using NIST API (api.h).

    KAZ970/
        Reference implementation of KAZ-SIGN with recommended
        parameters "N=970", using NIST API (api.h).

    KAZ_SIGN_REFERENCES.txt
        References of KAZ-SIGN for System Parameters, Key Generation Process, 	Signing Process and Verifying Process.

Optimized_Implementation/
    KAZ458/
        Optimized implementation of KAZ-SIGN with recommended
        parameters "N=458", using NIST API (api.h).
        This implementation uses Intel x64 processor (a 64-bit 
	implementation).

    KAZ738/
        Optimized implementation of KAZ-SIGN with recommended
        parameters "N=738", using NIST API (api.h).
        This implementation uses Intel x64 processor (a 64-bit 
	implementation). 

    KAZ970/
        Optimized implementation of KAZ-SIGN with recommended
        parameters "N=970", using NIST API (api.h).
        This implementation uses Intel x64 processor (a 64-bit 
	implementation).    
        
KAT/
    NIST-provided code to generate KAT files.

    KAZ458-KAT.req
    KAZ458-KAT.rsp
        KAT vectors for KAZ-458 (N size is 458-bit)

    KAZ738-KAT.req
    KAZ738-KAT.rsp
        KAT vectors for KAZ-738 (N size is 738-bit)

    KAZ970-KAT.req
    KAZ970-KAT.rsp
        KAT vectors for KAZ-970 (N size is 970-bit)

Supporting_Documentation/
    kaz_sign_D.pdf
        Detailed algorithm specifications of KAZ-SIGN.
    
    coversheet.pdf
	Cover sheet of the submission.


IMPORTANT NOTE
==============

The KAZ-SIGN algorithm accepts values of \delta \in (0,1) as mentioned in the 
KAZ-SIGN Algorithm Specifications in the submission package for the Post-Quantum 
Cryptographic Algorithm Submissions by NIST. For execution purposes in the 
submission, we utilize the value \delta=0.3. As such, it will be understood 
by those skilled in the art that various changes upon the value of \delta 
can be done and in turn will provide different parameter lengths for 
targeted security levels.



