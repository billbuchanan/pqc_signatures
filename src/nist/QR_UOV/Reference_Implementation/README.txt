# Build instructions
Please install gcc, make, and libssl-dev previously.
In this README.txt folder, type "make" to build the QR-UOV library and generate known-answer tests for NIST. 

#File description
Reference_Implementation      -- Reference C implementation
├─ ref
│   ├── api_h_gen.c
│   ├── Fql.c
│   ├── Fql.h
│   ├── License
│   ├── Makefile
│   ├── matrix.c
│   ├── matrix.h
│   ├── mgf.c
│   ├── mgf.h
│   ├── PQCgenKAT_sign.c
│   ├── qruov.c
│   ├── qruov.h
│   ├── qruov_config_h_gen.c
│   ├── qruov_misc.h
│   ├── refcount.h
│   ├── rng.c
│   ├── rng.h
│   └── sign.c
│
├── Makefile
├── qruov_config.src
└── README.txt         -- This file