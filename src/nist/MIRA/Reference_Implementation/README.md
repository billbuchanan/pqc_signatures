-------------------------------- 
MIRA: a Digital Signature scheme
--------------------------------


1. SUBMISSION OVERVIEW 
----------------------

Both reference implementation and optimized implementation provided for this 
submission are the same. The six parameters sets denoted respectively MIRA-128F, 
MIRA-128S, MIRA-192F, MIRA-192S, MIRA-256F and MIRA-256S are provided as explained 
in the supporting documentation. Each parameter set folder is organized as follows:

- bin/: Files generated during compilation
- doc/: Technical documentation of the scheme
- lib/: Third party libraries used 
- src/: Source code of the scheme  
- doxygen.conf: Documentation configuration file
- Makefile: Makefile


2. INSTALLATION INSTRUCTIONS 
----------------------------

2.1 Requirements

The following softwares and librairies are required: gcc and openssl.

2.2 Compilation Step

Let X denotes 128f, 128s, 192f, 192s, 256f or 256s depending on the parameter set considered. 
MIRA can be compiled in three differents ways:

- Execute make miraX-main to compile a working example of the scheme. Run bin/miraX-main to
  execute the scheme.
- Execute make miraX-kat to compile the NIST KAT generator. Run bin/miraX-kat to
  generate KAT files.
- Execute make miraX-verbose to compile a working example of the scheme in
  verbose mode. Run bin/miraX-verbose to generate intermediate values.

During compilation, objects files are created inside the bin/build folder.

3. DOCUMENTATION GENERATION 
---------------------------

3.1 Requirements

The following softwares are required: doxygen and bibtex.

3.2 Generation Step

- Run doxygen doxygen.conf to generate the code documentation
- Browse doc/html/index.html to read the documentation


4. ADDITIONAL INFORMATIONS 
--------------------------

4.1 Implementation overview

The MIRA scheme is defined in the api.h and parameters.h files and implemented in nist_sign.c.
The latter is based on the MPC construction of [1] using the Hypercube techniques from [2], 
which is defined in mpc.h and implemented in mpc.c.
The files finite_field.h and finite_fields.c provide the functions performing the 
various operations over finite fields required by the scheme.
As public key, secret key and signature can manipulated either with theirs mathematical 
representations or as bit strings, the files parsing.h and parsing.c provide functions to 
switch between these two representations.
Finally, the files hash_fips202.h and seedexpander_shake.h (inside the src/wrapper), 
randombytes.h, randombytes.c, along with the files in the XKCP folder (inside the lib/ folder),  
provides SHAKE and SHA3 implementations as well as the NIST random functions.

4.3 Public key, secret key, ciphertext and shared secret

The public key, secret key and signature are respectively composed of (M0, M1, ..., Mk), (x) and 
(sm, m). 
In order to shorten the keys, the public key is stored as (seed1, M0) and the secret key 
is stored as (seed2).


5. REFERENCES 
-------------

[1] Thibauld Feneuil. Building MPCitH-based Signatures from MQ, MinRank, Rank SD and PKP.
Cryptology ePrint Archive, Report 2022/1512, 2022.

[2] Carlos Aguilar-Melchor, Nicolas Gama, James Howe, Andreas Hülsing, David Joseph, 
Dongze Yue. The Return of the SDitH. Advances in Cryptology - EUROCRYPT 2023, LNCS 14008, 564-596.