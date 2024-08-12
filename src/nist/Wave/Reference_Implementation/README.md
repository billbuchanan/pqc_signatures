# Wave - Code-based Signature 

Wave is a digital signature based in linear codes. In this repository, we have the reference implementation for the signature. 
Wave has the most compact signatures in the code-based family. 

More information go to: http://wave-sign.org/

# Requirements
To compile the code it is needed: 
OpenSSL

# How to compile
change directory to any of Wave822/ Wave1249/ Wave1644/
'make'

# How to run

After a sucessfull compilation, it will generate three binaries:
- verify_test -> Binary to run one time the signature and verification of Wave.
- PQCgenKAT_sign -> Binary to generate the Known Answer Tests (KAT) from NIST. 
- benchmark -> Binary to run 10 iterations and save the number of cycles.

# Troubleshooting

Some executable files, in particular those in Wave1644/, require a large stack to run properly. It might be necessary to increase the the maximal stack size, e.g. on Linux platforms
ulimit -s 16384
