SNOVA
=======
This repository contains the official reference implementation of the SNOVA signature scheme in C language.

Build instructions
-------
These implementations include multiple test programs and an easy-to-compile Makefile.

Prerequisites for compiling this repository
-------
If you need to use the OpenSSL library, you should install the development files of the OpenSSL library, which are usually called openssl-devel in Linux. The installation method depends on your Linux distribution and version. Here are some installation commands for common Linux distributions:
- Debian/Ubuntu: sudo apt-get install libssl-dev
- Red Hat/CentOS/Fedora: sudo yum install openssl-devel
- SUSE/OpenSUSE: sudo zypper install openssl-devel


Test programs
-------
The steps to compile and run the SSS signature scheme's test program on Linux are as follows:

1. test keypair, sign and verify
```bash
make clean
make test
```
2. test nist api
```bash
make clean
make test_api
```
Set the parameters
-------
Only need to modify the SNOVA_V, SNOVA_O, SNOVA_L, SK_IS_SEED in the Makefile file.

Example: (makefile line 10)
```make
SNOVA_V = 24
SNOVA_O = 5
SNOVA_L = 4
SK_IS_SEED = 1
```

Genarate KAT
-------
```bash
make clean
make PQCgenKAT
```



