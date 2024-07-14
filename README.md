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

The current standards are:

* Falcon. [Falcon](https://asecuritysite.com/pqc/falcon_sign). Falcon is a NIST approved standard PQC (Post Quantum Cryptography) digital signatures. It is derived from NTRU ((Nth degree‐truncated polynomial ring units) and is a lattice-based methods for quantum robust digital signing. Falcon is based on the Gentry, Peikert and Vaikuntanathan method for generating lattice-based signature schemes, along with a trapdoor sampler - Fast Fourier sampling. We select three parameters: N, p and q. To generate the key pair, we select two polynomials: f and g. From these we compute: F=fq=f−1(modq) and where f and fq are the private keys. The public key is h=p⋅fq.f(modq). With Falcon-512 (which has an equivalent security to RSA-2048), we generate a public key of 897 bytes, and a signature size of 666 bytes, while FALCON-1024 gives a public key of 1,793 bytes and a signature size of 1,280 bytes.
* Dilithium. [Dilithium](https://asecuritysite.com/pqc/dilithium_sign). At present, CRYSTALS (Cryptographic Suite for Algebraic Lattices) supports two quantum robust mechanisms: Kyber for key-encapsulation mechanism (KEM) and key exchange; and Dilithium for a digital signature algorithm. CRYSTALS Dilithium uses lattice-based Fiat-Shamir schemes, and produces one of the smallest signatures of all the post-quantum methods, and with relatively small public and private key sizes. The three main implements for the parameters used are: Dilithium 2, Dilithium 3 and Dilithium 5. Overall, Dilithium 3 is equivalent to a 128-bit signature, and is perhaps the starting point for an implementation.

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
* Eagle. [Eagle](https://asecuritysite.com/pqc/eagle_sign). NIST has standardised two lattice methods of Dilithium and Falcon for Post Quantum Cryptography digital signing. Dilithium is a MLWE (Module-Learning With Errors)-based signature while Falcon uses a NTRU-based signature. Now there are new lattice methods which are going forward as part of the Additional Signature round. One of these is EagleSign, and which uses a variation of the ElGamal signature method, but uses structured lattices.
* EHT v3 and v4 - short signatures. [EHT v3 and v4 - short signatures](https://asecuritysite.com/pqc/eht_sign). EHT defines a post quantum cryptography lattice-based method for digital signatures. With EHT v3 Level 1 security, it has a 368-byte private key, and a relaively small signature of 169 bytes. Unfortunately its public key for Level 1 security is 83,490 bytes. For EHVTv4 Level 1, we have a much smaller public key of 1,107 bytes, a private key of 419 bytes, and a signature size of 369 bytes. Both EHTv3 and EHTv4 have much smaller signatures compared with Dilithium.
* HAETAE. [HAETAE](https://asecuritysite.com/pqc/haetae_sign). HAETAE is a lattice-based Post Quantum Cryptography (PQC) method that is based on the methods of Dilithium. It thus uses the “Fiat-Shamir with Aborts” approach. It differs in two respects: a bimodal distribution for the rejection sampling (similar to the BLISS signature scheme) and sampling from and reject to hyperball uniform distributions.
*  HAWK. [HAWK](https://asecuritysite.com/pqc/hawk_sign). HAWK is a lattice-based signature method that creates signatures using the lattice isomorphism problem (LIP) [1,2]. It is faster than Dilithium for signing and verification. It also has a low memory footprint and is supported on a range of hardware. There is currently no masking function, which could be suspectable to side-channel analysis. There are some worries about the security proofs involved with HAWK.
* HuFu. [HuFu](https://asecuritysite.com/pqc/hufu_sign). There are two main approaches to lattice-based signatures: Fiat-Shamir and Hash-and-sign. Overall, Dilithium uses a Fiat-Shamir approach, while Falcon takes a Hash-and-sign approach using the GPV lattice trapdoor framework [1]. A new proposed standard is HuFu (Hash-and-Sign Signatures From Powerful Gadgets) and which uses the hardness of standard worst-case problems on generic lattices. As with Falcon, it uses a hash-and-sign signature method with the GPV lattice trapdoor framework. Falcon uses an NTRU lattice approach [here], while HuFu uses a gadget approach to represent the lattice. With the gadget approach, the trapdoor is created using a linear relation between the public lattice and a gadget lattice (and which does not represent the full basis of the lattice). Unfortunately, the gadget approach leads to much larger public and secret keys, but can be the basis of other cryptographic primitives (such as for identity-based encryption and aggregate signatures).
*  MiRitH. [MiRitH](https://asecuritysite.com/pqc/mirith_sign). MiRitH (MinRank in the Head) is a quantum robust digital signature method for MPC-in-the-Head Signatures. It is based on the hardness of the MinRank problem. This is a problem that must find a non-trivial low-rank linear combination of defined matrices over a finite field. Mirith uses a MPC-in-the-Head (MPCitH) based Zero-Knowledge Proof of Knowledge (ZKPoK) that relates to a solution of the MinRank problem, and then is converted to a non-interactive signature scheme using the Fiat–Shamir transform. For Level I-a, it has a relatively small public and private key of 129 and 145 bytes, and a resonable size of signature of 7,877 bytes.
* PERK. [PERK](https://asecuritysite.com/pqc/perk_sign). PERK is a quantum robust digital signature method that uses a zero-knowledge proof based on a variant of the Permuted Kernel Problem (PKP) and with hash functions as random oracles. This is an MPC-in-the-head approach, and which is then converted into a Fiat-Shamir transform. PERK has a relatively small public and private key, and moderate signature sizes. It also uses symmetric key methods, and thus has good performance levels.
*  Ryde. [Ryde](https://asecuritysite.com/pqc/ryde_sign). The Ryde signature method [1] uses the Rank Syndrome Decoding problem as a hard problem. With this, we can create a proof of knowledge of a witness to a Rank-SD instance [2], and then transform this with a Fiat-Shamir transform. It produces relatively short keys, but a larger signature companed with Dilithium.
* PROV (PRovable unbalanced Oil and Vinegar). [PROV](https://asecuritysite.com/pqc/prov_sign)). PROV (PRovable unbalanced Oil and Vinegar) [1] uses a multivariate cryptography-based approach to create a Post Quantum Robust (PQC) digital signature. While there have been recent attacks on multivariate methods, PROV provides security proof. The proof is similar to the MAYO signature scheme where there is a larger oil space than the output of the scheme (defined often as UOV (Unbalanced Oil and Vinegar). The UOV approach was first defined by Kipnis, Patarin and Goubin [2] and integrates a hash-and-sign signature scheme into the GPV framework [3]. It was adapted in [4] for multivariate cryptography methods.
* TUOV (Triangular Unbalanced Oil and Vinegar) . [TUOV](https://asecuritysite.com/pqc/touv_sign). Within Round 1 of the NIST PQC digital assessment, Rainbow was cracked [here] and took around 50 hours on an eight core laptop. Overall, Rainbow has a multivariate cryptograpphy approach, but did not have strong security proofs and a weak parameter set. TUOV (Triangular Unbalanced Oil and Vinegar) is a PQC signature method that is based on UOV.The UOV approach was first defined by Kipnis, Patarin and Goubin [2] and integrates a hash-and-sign signature scheme into the GPV framework [3]. It was adapted in [4] for multivariate cryptography methods. Generally multivariate cryptography generates relatively short signatures, but have relative long public and private keys.
* VOX. [VOX](https://asecuritysite.com/pqc/vox_sign). VOX implements a UOV (Unblanced Oil and Vinegar)-based hash-and-sign signature scheme using Multivariate Quadratic (MQ) methods. It uses the Faugère, Macario-Rat, Patarin, and Perret method [1] which an UOV approach with a QR (Quotient Ring). Overall, for VOX-128, it produces a small signature size of 102 bytes, and public key of 9,104 bytes and a private key of 35,120 bytes. Gernally UOV methods produce short signatures, are fairly fast, and are fairly simple to implement.


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

UOV Level I                         278,432            237,896                128         1 (128-bit) Multivariate
UOV Level III                     1,225,440          1,044,320                200         3 (192-bit) Multivariate
UOV Level V                       2,869,440          2,436,704                260         5 (256-bit) Multivariate

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

EagleSign 3                           1,824                 576             2,335        3 (192-bit) Lattice
EagleSign 5                           3,616               1,600             3,488        5 (192-bit) Lattice

EHVTv3 Level 1                       83,490                 368               169         1 (128-bit) Lattice
EHVTv3 Level 3                      191,574                 532               255         3 (192-bit) Lattice
EHVTv3 Level 5                      348,975                 701               344         5 (256-bit) Lattice

EHVTv4 Level 1                        1,107                 419               369         1 (128-bit) Lattice
EHVTv4 Level 5                        2,623                 925               875         5 (256-bit) Lattice

HAETAE Level 2                          992               1,408             1,463         2 (128-bit) Lattice
HAETAE Level 3                        1,472               2,112             2,337         3 (192-bit) Lattice
HAETAE Level 5                        2,080               2,752             2,908         5 (256-bit) Lattice

HAWK 256                                450                  96               249         Cryptography test
HAWK 512                              1,024                 184               555         1 (128-bit) Lattice
HAWK 1024                             2,440                 360              1,221        5 (256-bit) Lattice

HuFu 1                            1,083,424          11,417,440              2,495       1 (128-bit)  Lattice
HuFu 3                            2,228,256          23,172,960              3,580       3 (192-bit) Lattice
HuFu 5                            3,657,888          37,418,720              4,560       5 (256-bit) Lattice

Mirith Ia-fast                          129                 145              7,877        1 (128-bit) Symmetric
Mirith IIIa-fast                        205                 229             17,139        3 (192-bit) Symmetric
Mirith Va-fast                          253                 285             30,458        5 (256-bit) Symmetric

PERK-I-fast3                            164                  16              8,345        1 (128-bit) Symmetric
PERK-III-fast3                          246                  24             19,251        3 (192-bit) Symmetric
PERK-V-fast3                            318                  32             34,100        5 (256-bit) Symmetric

Ryde-128f                                86                  32              7,446        1 (128-bit) Symmetric
Ryde-192f                               131                  48             16,380        3 (192-bit) Symmetric
Ryde-256f                               188                  64             29,802        5 (255-bit) Symmetric

SDitH-L1-hyp                            120                 404              8 241        1 (128-bit) Symmetric
SDitH-L3-hyp                            183                 616             19,161        3 (192-bit) Symmetric
SDitH-L5-hyp                            234                 234             33,370        5 (255-bit) Symmetric

dme-3rnds-8vars-32bits-sign            1449                 369                 32        1 (128-bit) Multivariate
dme-3rnds-8vars-48bits-sign            2169                 545                 48        1 (128-bit) Multivariate
dme-3rnds-8vars-64bits-sign            2880                 721                 64        1 (128-bit) Multivariate

PROV-1                                68,326             203,752                160       1 (128-bit) Multivariate
PROV-2                               215,694             666,216                232       3 (192-bit) Multivariate
PROV-3                               524,192           1,597,568                304       5 (256-bit) Multivariate

QR-UOV-I                             20,657                 256                 331       1 (128-bit) Multivariate
QR-UOV-III                           55,173                 385                 489       3 (192-bit) Multivariate
QR-UOV-V                            135,439                 512                 662       5 (256-bit) Multivariate

SNOVA-I                              1,016                   48                 240        1 (128-bit) Multivariate
SNOVA-III                            4,104                   64                 376        3 (192-bit) Multivariate
SNOVA-IV                             8,016                   80                 576        5 (256-bit) Multivariate

TUOV-I                              278,432             239,391                 112        1 (128-bit) Multivariate
TUOV-III                          1,048,279           1,225,440                 184        3 (128-bit) Multivariate
TUOV-V                            2,869,440           2,443,711                 244        5 (128-bit) Multivariate

VOX-128                              9,104               35,120                 102       1 (128-bit) Multivariate
VOX-192                             30,351              111,297                 184       3 (192-bit) Multivariate
VOX-256                             82,400              292,160                 300       5 (256-bit) Multivariate

ALTEQ-I (balanced)                   8,024                   16              15,896       1 (128-bit) Other
ALTEQ-III (balanced)                31,944                   24              49,000       3 (192-bit) Other
ALTEQ-V (balanced)                  73,632                   32             122,336       5 (256-bit) Other

```

And for performance in cycles (from papers):
```
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

EagleSign3          1,020,723       1,283,454        955,956  1Intel Core i7-1260P  at 2.40GGz  
EagleSign5          3,443,617       2,358,603      1,602,340

EHTv3 Level 1     465,600,000   181,920,000      1,968,000    Intel Core(TM) i7-12800H at 2.40GGz  
EHTv3 Level 3   1,432,800,000   494,400,000      4,272,000        
EHTv3 Level 5   3,672,000,000   732,000,000      7,584,000       

EHTv4 Level 1      29,040,000    21,600,000      9,240,000      
EHTv4 Level 5     276,000,000   142,320,000     62,880,000

HAWK 512            8,430,000       85,400         181,000 
HAWK 1024          43,700,000      148,000         303,000

HuFu 1          1,193,896,000    7,322,000       1,804,000  Intel Core i9-12900K at 3.20 GHz
HuFu 3          8,916,915,000   18,413,000       6,105,000        
HuFu 5          9,727,510,000   31,896,000      10,424,000

Mirith Ia-fast        108,903    8,703,311       7,311,069  Intel(R) Core(TM) i7-11850H at 2.50GHz 
Mirith IIIa-fast      246,740   22,485,807      18,431,919               
Mirith Va-fast        508,607   36,361,915      36,665,342


PERK-I-fast3           79,000    7,300,000       5,100,000  Intel® Core i9-13900K at 3.00 GHz     
PERK-III-fast3        169,000   15,000,000      12,000,000
PERK-V-fast3          297,000   34,000,000      27,000,000

Ryde-128f              86,000   88,900,000      82,800,000  Intel 13th Gen Intel (R) Core(TM) i9-13900K at 3GHz
Ryde-192f             141,100  221,100,000     204,000,000 
Ryde-256f             186,800  415,400,000     387,700,000

SDitH-L1-hyp      10,712,000    13,400,000      12,500,000  Intel Xeon E-2378 at 2.6GHz
SDitH-L3-hyp      12,714,000    11,770,000      27,700,000    
SDitH-L5-hyp      22,750,000    22,680,000      54,400,000

PROV-1         1,171,537,400    38,962,000      52,797,800  Intel Core i5-7260U CPU at 2.20GHz
PROV-3         5,558,390,200   129,890,200     173,978,200
PROV-5        16,845,708,000   300,641,000     401,456,000

QR-UOV-I          96,380,000    64,885,000      13,607,000 AMD EPYC 7763 at 2.45GHz
QR-UOV-III       370,704,000   245,199,000      45,522,000
QR-UOV-V       1,172,189,000   755,475,000     104,209,000

SNOVA-I            2,447,831    17,538,089       8,086,815 tel Core i7-6700 CPU at 3.40GHz
SNOVA-III         15,315,591    68,739,319      31,084,401
SNOVA-V           59,367,845   186,358,881      90,472,932

TUOV-I            10,682,834       220,792         127,722 Intel Core i5-10210U CPU at 1.60GHz
TUOV-III          57,322,074       608,604         442,770
TUOV-V           139,948,218     1,133,958         786,450

VOX-128            2,350,064       679,237         396,311 intel(R) Core(TM) i7-1185G7 at 3.00GHz
VOX-192            9,334,246     2,937,318       1,548,342
VOX-256           26,097,363    13,226,130       3,734,592

ALTEQ-I              385,015     2,847,138       4,226,834
ALTEQ-III          1,964,572    36,460,660      46,054,633
ALTEQ-V            4,859,458    59,708,622     103,463,869   
```
† Intel Xeon E3-1230L v3 1.80GHz (Haswell)
†† Intel Core i7-12700 clocked at 5.0 GHz (from CROSS paper).
††† Intel Xeon E5-1650 v3 at 3.50 GHz
†††† 11th Gen Intel(R) Core(TM) i7-1185G7 at 3.00GHz CPU
††††† Intel Core i5 10210U
†††††† Intel Comet Lake (Intel Core i7-10700) CPU at 2.9GHz
††††††† Ryzen 5 3600 CPU
†††††††† MD Ryzen 7 5800H processor at 3.2GHz 
