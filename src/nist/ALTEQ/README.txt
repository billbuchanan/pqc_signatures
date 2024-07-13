

ALTEQ
======

This folder contains the subfolders

KAT
Refernece_Implementation
Optimized_Implementation
Supporting_Documentation



----------------------
KAT
This folder contains the Known Answer Test values of our implementation.
For additional details, please refer to the KAT/README.txt.

---------------------------------
Reference_Implementation
This folder contains the reference implementation of ALTEQ.
The subfolder /api contains 6 sets of recommended parameters as follows:
api.h.1.fe
This aims for NIST level I security with recommended parameter "N=13,Log_Q=32,ROUND=84,K=22,C=7"

api.h.1.lp
This aims for NIST level I security with recommended parameter "N=13,Log_Q=32,ROUND=16,K=14,C=458"

api.h.3.fe
This aims for NIST level III security with recommended parameter "N=20,Log_Q=32,ROUND=201,K=28,C=7"

api.h.3.lp
This aims for NIST level III security with recommended parameter "N=20,Log_Q=32,ROUND=39,K=20,C=229"

api.h.5.fe
This aims for NIST level V security with recommended parameter "N=25,Log_Q=32,ROUND=119,K=48,C=8"

api.h.5.lp
This aims for NIST level V security with recommended parameter "N=25,Log_Q=32,ROUND=67,K=25,C=227"

A makefile is present for users to measure the performance by:
  - either the number of cycles through "make test_speed_all"
  - either the CPU time through "test_CPU_time"

Note that the extensions:
 - "fe" stands for "Fully Equlibrated" and correspond to "Balanced" in the supporting documentation
 - "lp" stands for "Large Public key" and correspond to "ShortSig" in the supporting documentation

---------------------------------
Optimized_Implementation
This folder contains an optimized implementation of ALTEQ, giving the same outputs as the above.
Essentially, almost all the files are identical, but compiled with performances options such as -mavx2.

The main differences concern the folder aes and the folder keccak, that contained avx optimized versions

---------------------------------
Supporting_Documentation
This folder contains the documentation ALTEQ_Documentation.pdf, the cover sheet, signed patent statement of each submitter (under the subfolder patent-statements), and the signed implementation owner statement (under the subfolder implementation-statement).

The file ALTEQ_Documentation contains information concerning our scheme, namely:
  - algorithm specification
  - performance data
  - implementation details
  - security analysis
  - advantage and limitations

