#	nist-ref

Copyright (c) 2023 Raccoon Signature Team. See LICENSE.

For information about NIST PQC Signature submission requirements, see the [call for proposals](https://csrc.nist.gov/projects/pqc-dig-sig/standardization/call-for-proposals) and [submission file documentation](https://csrc.nist.gov/Projects/pqc-dig-sig/standardization/example-files). 

#	Root level directory structure of this repository:

*	[ref-c](ref-c): Written in the style of a "portable ANSI C reference implementation" for NIST, and supports NIST API and KAT generation. This code has options for 32-bit CRT NTT arithmetic (default: off) and LFSR vs ASCON masking random (default: lfsr), used for various discussions.
*	[ref-py](ref-py): Python implementation, also with NIST KAT testbench.
*	[doc](doc): Documentation, including [doc/raccoon.pdf](doc/raccoon.pdf), the current specification.

Currently we are not delivering an AVX2, Cortex M4, or FPGA code to NIST; those aspects will be just discussed in the delivered document.


#	Generating KATs

*	The C implementation generates the KATs with `./genkats.sh`. 
*	With the C implementation can also obtain benchmarks with `./bench.sh`. The standard `./xtest` test binary generated by `make` is also useful. 
*	The Python implementation takes a some minutes with `python3 test_genkat.py` since each KAT file contains a set of 100 test vectors (can be modified).

Running `sha256sum *.rsp` in the C implementation currently gives the following hashes for 100-vector response files generated by both implementations (albeit with Python one can may want to opt for 10 test vectors). See the directory [ref-data](ref-data) `rsp` hashes for 10 vectors, preliminary benchmarks, and additional data.
```
Raccoon-128-1   039383b9d9b29c5a9cda63cb93666771c7c09791afaadc941341e0df670229e0  PQCsignKAT_14800.rsp
Raccoon-128-2   71586c2fd1ae47f17cb5c44c2b5351ab48531344041a76357ffc695098d2506c  PQCsignKAT_14816.rsp
Raccoon-128-4   ae6e775feaf9d26eac5d10bec3c742fb7ab8f6716ee96a2ce3cf2c3aa23b8ef0  PQCsignKAT_14848.rsp
Raccoon-128-8   ffbd4df642d15da96624e2b8489b5303a97a7f6a5d60416c72108880746394ea  PQCsignKAT_14912.rsp
Raccoon-128-16  579fbaafde26049c4f4993b28568abfb657da76e5cd0c7a83239e37d4cc43325  PQCsignKAT_15040.rsp
Raccoon-128-32  dff454bf03e9c027d70d4443bb394cae3c5af23ed81179889a62bf98a8a916d8  PQCsignKAT_15296.rsp
Raccoon-192-1   bb577467a15ff20d6ac88c3eb7ba3fd6b3a3e7bf8e5bc627890bb027bba8bda5  PQCsignKAT_18840.rsp
Raccoon-192-2   1543992c77e4a3ee08cd93daf1044e2d7816efbb6c572f167e500ee5b6e68d02  PQCsignKAT_18864.rsp
Raccoon-192-4   82f2b834889bacdbcbb48d51f99c15639a235a764714ba858b415fdf546c9dbc  PQCsignKAT_18912.rsp
Raccoon-192-8   b21ecba12cafa88a8337a813e9dac131a50f043f860241f7cd36f8b502233971  PQCsignKAT_19008.rsp
Raccoon-192-16  57e3c6d014c7283806f4cd3d9c83737c6d381202a1649042c499c5c354f7606b  PQCsignKAT_19200.rsp
Raccoon-192-32  49a552559d6a68175996de373232e0863496834c16b4d2772781f0e01469b621  PQCsignKAT_19584.rsp
Raccoon-256-1   031d4976f4c09b90ecec5c535b5ab3bcb020b9cb4f95e17dfdcedb10de1425fc  PQCsignKAT_26016.rsp
Raccoon-256-2   8936afaf3fd6cf5b43716e006977e1c14a2624913bfd23adb850aa141ef2ae91  PQCsignKAT_26048.rsp
Raccoon-256-4   2e3ae8a29435ce8621a98390874fa2193756c87741f02934018650163c57e369  PQCsignKAT_26112.rsp
Raccoon-256-8   893bf614327740610c29781db7973bbfa7069010039bfa9b2ba02a9a675a78ab  PQCsignKAT_26240.rsp
Raccoon-256-16  663ce05beb35184b0012e638ed8c918f945b379a9bd35a97e37141798c320acf  PQCsignKAT_26496.rsp
Raccoon-256-32  594169ee1ddc6238fbbfae0178d0ed8fab9eb0205066fe382f6ff788c775bd58  PQCsignKAT_27008.rsp
```
