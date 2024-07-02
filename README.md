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
* FuLeeca. [FuLeeca](https://asecuritysite.com/pqc/fulecca_sign). While many code-based methods used a Hamming metric, the FuLeeca method uses the Lee metric. This leads to relatively small public key and signature sizes.  With code-based signatures, we can use a Fiat-Shamir method into a zero-knowledge identification technique. This often leads to relatively large signatures. As an alterative we can use a Hash-based approach, such as with the McEliece method, and whch leads to small signature sizes, but relatively large public keys. FuLeeca takes this approach, but modifies it with the Lee weight.
* SPHINCS-α. [SPHINCS-α](https://asecuritysite.com/pqc/sphincs_sign). SPHINCS+ is a stateless hash-based signature scheme that is PQC (Post Quantum Robust). It is generally believed to be a secure method - based on the hardness of reversing the cryptographic hashing method. Now, SPHINCS-α is proposed as a new standard for Round 1 Additional Signatures. This improves on the SPHINCS+ methods, while still keeping its core elements. The addition includes a size-optimal encoding scheme that is applied to tree-structured one-time signatures.
* FAEST. [FAEST](https://asecuritysite.com/pqc/faest_sign). NIST approved Dilithium, Falcon and SPHINCS+ for PQC digital signatures and is now looking at other alternative signatures. One of these is the FAEST digital signature algorithm [1], and which uses symmetric key primitives. This links directly to the security of AES128 (Level 1), AES192 (Level 3) and AES256 (Level 5). A key pair (pk,sk) is defined as: pk=(x,y)  and sk=k and where Ek(x)=y. Overall, E is the block cipher to use, k is the private key, and x is a plaintext block. The signature then becomes a non-interactive argument of knowledge of sk. This is similar to the Picnic method, but rather than using the MPC-in-the-Head (MPCitH) framework, it uses the VOLE-in-the-Head method [2].
* LESS.  [LESS (Linear Equivalence Signature Scheme)](https://asecuritysite.com/pqc/less_sign) LESS uses Fiat-Shamir transformation onto a zero-knowledge identification scheme. It uses a one-round Sigma protocol. The security of LESS depends on the hardness of the Linear Equivalence Problem (LEP).
* MEDS. [MEDS](https://asecuritysite.com/pqc/meds_sign). The MEDS (Matrix Equivalence Digital Signature) scheme supports PQC digital signing [1]. Its security is supported by the difficulty of finding an isometry between two equivalent matrix rank-metric codes. From this problem, it integrates a zero-knowledge identification scheme for multiple rounds of a Sigma protocol. The Fiat-Shamir method is then used to create the signature.
* Wave. [Waves](https://asecuritysite.com/pqc/wave_sign). Wave is a code-based hash-and-sign signature scheme [1]. It uses the method defined by Gentry, Peikert and Vaikuntanathan to create a trapdoor function [3]. Overall it has a relatively small signature value, but a relatively large public key.



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



Raccoon-128-1 (Lattice)               2,256             14,800             11,524         1 (128-bit) Lattice
Raccoon-192-1 (Lattice)               3,160             18,840             14,544         3 (192-bit) Lattice
Raccoon-256-1 (Lattice)               4,064             26,016             20,330         5 (256-bit) Lattice

AIMER L1                                 32                 16              5,904         1 (128-bit) Symmetric
AIMER L3                                 48                 24             13,080         3 (192-bit) Symmetric
AIMER L5                                 64                 32             25,152         5 (256-bit) Symmetric

CROSS R-SDP Level 1                      61                 16             12,944         1 (128-bit) Code
CROSS R-SDP Level 3                      91                 24             37,080         3 (192-bit) Code
CROSS R-SDP Level 5                     121                 32             51,120         5 (256-bit) Code

pqsigRM-6-13 L1                   2,129,400             24,592              1,040         1 (128-bit) Code

FuLecca1                              1,318              2,636              1,100         1 (128-bit) Code
FuLecca3                              1,982              3,964              1,620         3 (192-bit) Code
FuLecca5                              2,638              5,276              2,130         5 (256-bit) Code


SPHINCS-a-sha2-128f                      32                 64             16,720         1 (128-bit) Hash-based
SPHINCS-a-sha2-192f                      48                 96             34,896         3 (192-bit) Hash-based
SPHINCS-a-sha2-256f                      64                128             49,312         5 (256-bit) Hash-based

FAEST-128f                               32                 32              6,336         1 (128-bit) Symmetric
FAEST-192f                               64                 56             16,792         3 (192-bit) Symmetric
FAEST-256f                               64                 64             28,400         5 (256-bit) Symmetric

LESS-1b                              13,940                 32              9,286         1 (128-bit) Code
LESS-3b                              35,074                 48             18,000         3 (192-bit) Code
LESS-5b                              65,793                 64             31,896         5 (256-bit) Code

MEDS9923                              9,923               1,828             9,896        1 (128-bit) Code
MEDS41711                            41,711               4,420            41,080        3 (192-bit) Code
MEDS167717                          167,717              12,444           165,464        5 (256-bit) Code

WAVE-822                          3,677,389              18,900               822        1 (128-bit) Code
WAVE-1217                         7,867,597              27,629             1,218        3 (192-bit) Code
WAVE-1612                        13,632,308              36,359             1,644        5 (256-bit) Code
```

And for performance in cycles (from papers):
```
Method               Keygen           Sign         Verify
------------------------------------------------------------
Dilithium 2          300,751        1,081,174      327,362 (Unoptimized, Ref, Skylake)
Dilithium 3          544,232        1,713,783      522,267 
Dilithium 5          819,475        2,383,399      871,609

Falcon-512        19,872,000          386,678       82,339 (Intel e5-8259U 2.3GHz)
Falcon-1024       63,135,000          961,208      205,128


SPHINCS+128f        9,649,130     239,793,806    12,909,924 * (3.1 GHz Intel Xeon E3-1220 CPU (Haswell)
SPHINCS+192f       14,215,518     386,861,992    19,876,926 
SPHINCS+256f       36,950,136     763,942,250    19,886,032      

*Additional: SHAKE256-128f-simple

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

pqsigRM-6-13 L1 2,034,133,439       2,232,288        242,901

FuLecca1           49,354,000   1,846,779,000      1,260,000 ††††††
FuLecca3          110,918,000   2,111,156,000      2,447,000 ††††††
FuLecca5          192,388,000  12,327,726,000      3,789,000 ††††††


SPHINCS-a-128f       1,036,602     26,635,716     2,028,186 †††††††
SPHINCS-a-192f       2,199,276     45,218,790     1,744,038 †††††††
SPHINCS-a-256f       4,286,574     91,335,474     3,175,290 †††††††

FAEST-128f              92,800     27,836,800    27,836,800 ††††††††     
FAEST-192f             422,400     70,800,000    70,800,000 ††††††††  
FAEST-256f             700,800    123,648,000   123,648,000 ††††††††

LESS-1b              3,400,000    878,700,000    890,800,000  Intel Core i7-12700K at 4.9GHz
LESS-3b              9,300,000  7,224,100,000  7,315,800,000
LESS-5b             24,400,000 33,787,700,000 34,014,000,000

MEDS9923            1,900,000     518,050,000    515,580,000
MEDS41711           9,800,000   1,467,000,000  1,461,970,000 
MEDS134180         44,750,000   1,629,840,000  1,621,570,000

WAVE-822       14,468,000,043   1,160,793,621    205,829,565  Intel i5-1135G7 at 2.4GHz 
WAVE-1249      47,222,134,806   3,507,016,206    464,110,855
WAVE-1612     108,642,333,507   7,936,541,947    813,301,900 
```
† Intel Xeon E3-1230L v3 1.80GHz (Haswell)
†† Intel Core i7-12700 clocked at 5.0 GHz (from CROSS paper).
††† Intel Xeon E5-1650 v3 at 3.50 GHz
†††† 11th Gen Intel(R) Core(TM) i7-1185G7 at 3.00GHz CPU
††††† Intel Core i5 10210U
†††††† Intel Comet Lake (Intel Core i7-10700) CPU at 2.9GHz
††††††† Ryzen 5 3600 CPU
†††††††† MD Ryzen 7 5800H processor at 3.2GHz 
