# Round 1 Additional Signatures

## Introduction
And, so, Dilithium, FALCON and SPHINCS+ have become NIST standards for digital signatures, and with an aim to remove RSA, ECDSA and EdDSA. But, NIST wants alternatives to these, especially so that we are not too dependent on lattice-based approaches (such as with Dilithium and FALCON). These are [here](https://csrc.nist.gov/projects/pqc-dig-sig/round-1-additional-signatures):

* Multivariate Signatures (10): 3WISE, DME-Sign, HPPC (Hidden Product of Polynomial Composition), MAYO, PROV (PRovable unbalanced Oil and Vinegar), QR-UOV, SNOVA, TUOV (Triangular Unbalanced Oil and Vinegar), UOV (Unbalanced Oil and Vinegar), and VOX.
* MPC-in-the-Head Signatures (7): Biscuit, MIRA, MiRitH (MinRank in the Head), MQOM (MQ on my Mind), PERK, RYDE, and SDitH (Syndrome Decoding in the Head).
* Lattice-based Signatures (6): EagleSign, EHTv3 and EHTv4, HAETAE, HAWK, HuFu (Hash-and-Sign Signatures From Powerful Gadgets), and SQUIRRELS ( Square Unstructured Integer Euclidean Lattice Signature).
* Code-based Signatures (5): CROSS (Codes and Restricted Objects Signature), Enhanced pqsigRM, FuLeeca, LESS (Linear Equivalence), MEDS (Matrix Equivalence Digital Signature).
* Symmetric-based Signatures (4): AIMer, Ascon-Sign, FAEST, and SPHINCS-alpha.
* Other Signatures (4): ALTEQ, eMLE-Sig 2.0 (Embedded Multilayer Equations with Heavy Layer Randomization), KAZ-SIGN (Kriptografi Atasi Zarah), Preon, and Xifrat1-Sign.
* Isogeny Signatures (1): SQIsign.


## Implementations
At present the code contains:

* Ascon-Sign - [Ascon-Sign](https://asecuritysite.com/pqc/ascon_sign).  Ascon-Sign is a variant of the SPHINCS signature method but using Ascon-Hash and Ascon-XOF17 as building blocks. Overall, ASCON integrates authenticated encryption with associated data (AEAD) and a hashing method.
* UOV Sign - [UOV Sign](https://asecuritysite.com/pqc/uov_sign). The multivariate polynomial problem is now being applied in quantum robust cryptography, where we create a trap door to allow us to quickly solve the n variables with m equations (which are multivariate polynomials). In the following example we sign a message with the private key and verify with the public key. In this case we use the UOV (Unbalanced Oil and Vinegar) cryptography method.
* Biscuit Sign - [Biscuit Sign](https://asecuritysite.com/pqc/biscuit_sign). An MPC-in-the-Head approach [2, 3] uses non-interactive zero-knowledge proofs of knowledge and MPC (Multiparty Computation). With MPC we can split a problem into a number of computing elements, and these can be worked on in order to produce the result, and where none of the elements can see the working out at intermediate stages. It has the great advantage of this method is that we only use symmetric key methods (block ciphers and hashing functions).
* Raccoon with C Code. [Raccoon](https://asecuritysite.com/pqc/raccoon_sign). Raccoon is a Lattice-based Post Quantum Signature scheme which uses a Fiat Shamir method without aborts (as apposed to the Dilithium method which does Fiat Shamir with aborts). This method allows distributed threshold signatures to be supported [1], and also gives improved support over side channel attacks. Raccoon has been developed by PQShield and has been submitted to the NIST PQC competition for additional signatures. In this case, we will use the C code for Raccoon-128-1, Raccoon-192-3 and Raccoon-256-1 to sign a random message.
* Raccoon with Python. [Raccoon](https://asecuritysite.com/pqc/raccoon). Raccoon is a Lattice-based Post Quantum Signature scheme which uses a Fiat Shamir method, and has been submitted to the NIST PQC competition for additional signatures.
* AIMer. [AIMer](https://asecuritysite.com/pqc/aimer_sign). With AIMer we use a zero-knowledge proof of the preimage knowledge [1] for a one-way function. Both of these use symmetric primitives, and will thus likely to be relatively fast in their implementation.


## Key sizes

```
Method                           Public key size    Private key size   Signature size  Security level
------------------------------------------------------------------------------------------------------
Crystals Dilithium 2 (Lattice)        1,312              2,528              2,420         1 (128-bit) Lattice
Crystals Dilithium 3                  1,952              4,000              3,293         3 (192-bit) Lattice
Crystals Dilithium 5                  2,592              4,864              4,595         5 (256-bit) Lattice

Falcon 512 (Lattice)                    897              1,281                690         1 (128-bit) Lattice
Falcon 1024                           1,793              2,305              1,330         5 (256-bit) Lattice
Sphincs SHA256-128f Simple               32                 64             17,088         1 (128-bit) Hash-based
Sphincs SHA256-192f Simple               48                 96             35,664         3 (192-bit) Hash-based
Sphincs SHA256-256f Simple               64                128             49,856         5 (256-bit) Hash-based

RSA-2048                                256                256                256
ECC 256-bit                              64                 32                256

NIST Round 1 Additional Signatures
-----------------------------------
ASCON-Sign-128f Simple                   32                 64             17,088         1 (128-bit) Hash-based
ASCON-Sign-192f Simple                   48                 96             35,664         1 (192-bit) Hash-based

UOV Level I                         278,432            237,896                128         1 (128-bit) UOV
UOV Level III                     1,225,440          1,044,320                200         3 (192-bit) UOV
UOV Level V                       2,869,440          2,436,704                260         5 (256-bit) UOV

Biscuit 128f                             50                115              6,726         1 (128-bit) MPC
Biscuit 192f                             69                158             15,129         3 (192-bit) MPC
Biscuit 256f                             93                212             27,348         5 (256-bit) MPC

CROSS R-SDP Level 1                      61                 16             12,944         1 (128-bit) Code
CROSS R-SDP Level 3                      91                 24             37,080         3 (192-bit) Code
CROSS R-SDP Level 5                     121                 32             51,120         5 (256-bit) Code

Raccoon-128-1 (Lattice)               2,256             14,800             11,524         1 (128-bit) Lattice
Raccoon-192-1 (Lattice)               3,160             18,840             14,544         3 (192-bit) Lattice
Raccoon-256-1 (Lattice)               4,064             26,016             20,330         5 (256-bit) Lattice

AIMER L1                                 32                 16              5,904         1 (128-bit) Symmetric
AIMER L3                                 48                 24             13,080         3 (192-bit) Symmetric
AIMER L5                                 64                 32             25,152         5 (256-bit) Symmetric
```

And for performance in cycles (from paper):
```
                       Keygen            Sign         Verify
------------------------------------------------------------
Dilithium 2           97,621          281,078       108,711 †
Falcon-512        19,189,801          792,360       103,281 †
SPHINCS+           1,334,220       33,651,546     2,150,290 †

NIST Round 1 Additional Signatures (Reference implementations)
-----------------------------------
UOV-I               3,311,188         116,624         82,668 †
UOV-III            22,046,680         346,424        275,216 †
UOV-V              58,162,124         690,752        514,100 †

biscuit128f            82,505       9,653,412      8,734,302 ††††
biscuit192f           210,150      81,492,308     75,826,788 ††††
biscuit256f           353,223   1,147,099,575    137,359,832 ††††

CROSS-R-SDP-f Level 1 100,000       6,760,000      3,170,000 ††
CROSS-R-SDP-f Level 3 240,000      11,810,000      5,870,000 ††
CROSS-R-SDP-f Level 5 440,000      37,090,000     14,560,000 ††

AIMER L1               59,483       4,294,114      4,011,553 †††         
AIMER L3              131,234      10,767,276     10,222,797 †††
AIMER L5              311,887      21,217,778     20,395,571 †††

Ascon-Sign-128f     5,939,611     115,382,780      6,972,950 †††††
Ascon-Sign-192f    10,939,221     243,023,163     13,058,030 †††††

```
† Intel Xeon E3–1230L v3 1.80GHz (Haswell)
†† Intel Core i7–12700 clocked at 5.0 GHz (from CROSS paper).
††† Intel Xeon E5–1650 v3 at 3.50 GHz
†††† 11th Gen Intel(R) Core(TM) i7–1185G7 at 3.00GHz CPU
††††† Intel Core i5 10210U
