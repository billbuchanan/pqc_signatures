Preon
======

This archive contains the following files and directories:

Reference_Implementation/
    Preon128/
            Preon128A/
            	Reference implementation of Preon with recommended
            	parameters "AES-128, |F| = 2^{192}, l=26", using NIST API (api.h).
            Preon128B/
            	Reference implementation of Preon with recommended
            	parameters "AES-128, |F| = 2^{192}, l=58", using NIST API (api.h).
            Preon128C/
            	Reference implementation of Preon with (non-recommended) conservative
            	parameters "AES-128, |F| = 2^{192}, l=381", using NIST API (api.h).

     Preon192/
            Preon192A/
            	Reference implementation of Preon with recommended
            	parameters "AES-192, |F| = 2^{256}, l=39", using NIST API (api.h).
            Preon192B/
            	Reference implementation of Preon with recommended
            	parameters "AES-192, |F| = 2^{256}, l=87", using NIST API (api.h).
            Preon192C/
            	Reference implementation of Preon with (non-recommended) conservative
            	parameters "AES-192, |F| = 2^{256}, l=556", using NIST API (api.h).

    Preon256/
            Preon256A/
            	Reference implementation of Preon with recommended
            	parameters "AES-256, |F| = 2^{320}, l=52", using NIST API (api.h).
            Preon256B/
            	Reference implementation of Preon with recommended
            	parameters "AES-256, |F| = 2^{320}, l=118", using NIST API (api.h).
            Preon256C/
            	Reference implementation of Preon with (non-recommended) conservative
            	parameters "AES-256, |F| = 2^{320}, l=729", using NIST API (api.h).


Optimized_Implementation/
    Preon128/
            Preon128A/
            	Optimized implementation of Preon with recommended
            	parameters "AES-128, |F| = 2^{192}, l=26", using NIST API (api.h).
            Preon128B/
            	Optimized implementation of Preon with recommended
            	parameters "AES-128, |F| = 2^{192}, l=58", using NIST API (api.h).
            Preon128C/
            	Optimized implementation of Preon with (non-recommended) conservative
            	parameters "AES-128, |F| = 2^{192}, l=381", using NIST API (api.h).

     Preon192/
            Preon192A/
            	Optimized implementation of Preon with recommended
            	parameters "AES-192, |F| = 2^{256}, l=39", using NIST API (api.h).
            Preon192B/
            	Optimized implementation of Preon with recommended
            	parameters "AES-192, |F| = 2^{256}, l=87", using NIST API (api.h).
            Preon192C/
            	Optimized implementation of Preon with (non-recommended) conservative
            	parameters "AES-192, |F| = 2^{256}, l=556", using NIST API (api.h).

    Preon256/
            Preon256A/
            	Optimized implementation of Preon with recommended
            	parameters "AES-256, |F| = 2^{320}, l=52", using NIST API (api.h).
            Preon256B/
            	Optimized implementation of Preon with recommended
            	parameters "AES-256, |F| = 2^{320}, l=118", using NIST API (api.h).
            Preon256C/
            	Optimized implementation of Preon with (non-recommended) conservative
            	parameters "AES-256, |F| = 2^{320}, l=729", using NIST API (api.h).

KAT/

    PQCsignKAT_Preon128A.req
    PQCsignKAT_Preon128A.rsp
        KAT vectors for Preon128A (AES-128, |F| = 2^{192}, l=26)

    PQCsignKAT_Preon128B.req
    PQCsignKAT_Preon128B.rsp
        KAT vectors for Preon128B (AES-128, |F| = 2^{192}, l=58)

    PQCsignKAT_Preon192A.req
    PQCsignKAT_Preon192A.rsp
        KAT vectors for Preon192A (AES-192, |F| = 2^{256}, l=39)

    PQCsignKAT_Preon192B.req
    PQCsignKAT_Preon192B.rsp
        KAT vectors for Preon192B (AES-192, |F| = 2^{256}, l=87)

    PQCsignKAT_Preon256A.req
    PQCsignKAT_Preon256A.rsp
        KAT vectors for Preon256A (AES-256, |F| = 2^{320}, l=52)

    PQCsignKAT_Preon256B.req
    PQCsignKAT_Preon256B.rsp
        KAT vectors for Preon256B (AES-256, |F| = 2^{320}, l=118)



Supporting_Documentation/
    Preon_v1.pdf
        Detailed specification of Preon.

    coverletter_Preon.pdf
        Coverletter of Preon

NIST IP Statement/
        IP statements from all submitters


Notes
=====

Each implementation under Reference_Implementation and
Optimized_Implementation has its own Makefile; when used, it compiles
the code. The resulting binary (created in the same directory), when
executed, produced the .req and .rsp files, which should be identical
to the ones provided in KAT/.

User should have `openssl` installed before making the binary.

For Debian-based linux distributions (Debian, Ubuntu… etc), follow the below instructions to install prerequisites.

sudo apt update
sudo apt install libssl-dev -y
