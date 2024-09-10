# hawk-py

This directory contains a Python3 implementation of Hawk signature scheme.
Its purpose is to support the specification document and high-level understanding of Hawk.
For performance evaluation, please refer to the C implementations.

**warning**: This project makes use of Falcon `NTRUSolve` in [Python3](https://github.com/tprest/falcon.py) and hence uses floating-points.
This implementation generates valid priv/pub key pairs, but is not compliant with the reference implementation / specifications that leverage fixed-point `NTRUSolve`.  

## Structure of the repository
Next we detail the content of each files on a high level. Most of the functions contain a reference to the algorithm it implements in the specification document.
- `keygen.py`: Key generation algorithm for Hawk.
- `sign.py`: Signature generation algorithm for Hawk.
- `verify.py`: Signature verification algorithm for Hawk.
- `codec.py`: Helper function to encode keys and signatures.
- `params.py`: Parameters for Hawk-(256,512,1024).
- `poly.py`: Several helper functions for integer polynomials.
- `rngcontext.py`: Wrappers around SHAKE256.
-  `ntrugen/`: `NTRUSolve` from [Falcon Python3 implementation](https://github.com/tprest/falcon.py).
    This uses floating points, which can/should be avoided. To do so, please refer to the specifications and C implementation.
-  `ref/`: The reference C implementation of Hawk. This is used during testing.

Tests are available in `test.py`. It compares the output of the Python3 implementation with the reference implementation from `ref/`.

## License
The code under `ntrugen` is derived from [Falcon Python3 implementation](https://github.com/tprest/falcon.py) licensed under MIT.
The code under `ref` is derived from [Hawk C implementation]() licensed under MIT.
This code is licensed under MIT.

## Disclaimer

This code does not provide **any** guarantee and should not be used in production.
