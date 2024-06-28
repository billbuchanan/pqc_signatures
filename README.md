# Round 1 Additional Signatures

At present the code contains:

* Ascon-Sign - [Ascon-Sign](https://asecuritysite.com/pqc/ascon_sign).  Ascon-Sign is a variant of the SPHINCS signature method but using Ascon-Hash and Ascon-XOF17 as building blocks. Overall, ASCON integrates authenticated encryption with associated data (AEAD) and a hashing method.
* UOV Sign - [UOV Sign](https://asecuritysite.com/pqc/uov_sign). The multivariate polynomial problem is now being applied in quantum robust cryptography, where we create a trap door to allow us to quickly solve the n variables with m equations (which are multivariate polynomials). In the following example we sign a message with the private key and verify with the public key. In this case we use the UOV (Unbalanced Oil and Vinegar) cryptography method.
* Biscuit Sign - [Biscuit Sign](https://asecuritysite.com/pqc/biscuit_sign). An MPC-in-the-Head approach [2, 3] uses non-interactive zero-knowledge proofs of knowledge and MPC (Multiparty Computation). With MPC we can split a problem into a number of computing elements, and these can be worked on in order to produce the result, and where none of the elements can see the working out at intermediate stages. It has the great advantage of this method is that we only use symmetric key methods (block ciphers and hashing functions).
* Raccoon. [Raccoon](https://asecuritysite.com/pqc/raccoon). Raccoon is a Lattice-based Post Quantum Signature scheme which uses a Fiat Shamir method, and has been submitted to the NIST PQC competition for additional signatures.
