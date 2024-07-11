This folder contains the digital content of the proposal.

*****  QR-UOV  *****,

submitted to the NIST Post-Quantum Standardization Process

by

Hiroki Furue,
Yasuhiko Ikematsu,
Fumitaka Hoshino,
Tsuyoshi Takagi,
Kan Yasuda,
Toshiyuki Miyazawa,
Tsunekazu Saito,
Akira Nagai


This folder contains the subfolders.

・Alternative_Implementation
・IP_Statement
・KAT
・Optimized_Implementation 
・Reference_Implementation
・Supporting_Documentation

--------------------------------------------------------------------
Alternative_Implementation
This folder contains the c-code of additional implementations of QR-UOV for avx2 and avx512 instructions. More information on the content of this folder as well as an explanation of how to use the code can be found in Alternative_Implementation/README.txt.
--------------------------------------------------------------------
IP_Statement
This folder contains IP statements by each submitter.
--------------------------------------------------------------------
KAT
This folder contains the Known Answer Test values of our implementation.
--------------------------------------------------------------------
Optimized_Implementation
This folder contains the c-code of our implementations of QR-UOV optimized for amd64 architecture. More information on the content of this folder as well as an explanation of how to use the code can be found in Optimized_Implementation/README.txt.
--------------------------------------------------------------------
Reference_Implementation
This folder contains the c-code of our Reference implementation of QR-UOV. More information on the content of this folder as well as an explanation of how to use the code, can be found in Reference_Implementation/README.txt.
--------------------------------------------------------------------
Supporting Documentation
This folder contains the file QRUOV_Documentation.pdf, containing the algorithm specification, implementation details, performance data, security analysis as well as advantages and limitations of our scheme. This folder also contains intermediate values, which can be used to check the implementation.
--------------------------------------------------------------------

Package Contents

QR_UOV
├─ Alternative_Implementation		-- Additional C implementation
│   ├── avx2
│   ├── avx512
│   ├── Makefile
│   ├── README.txt
│   └── qruov_config.src
│ 
├─ IP_Statement
│   └── qruov_IP_Statement.pdf
│
├─ KAT			        -- One set KATs per parameter set
│   ├── qruov1q7L10v740m100
│   ├── qruov1q31L3v165m60
│   ├── qruov1q31L10v600m70
│   ├── qruov1q127L3v156m54
│   ├── qruov3q7L10v1100m140
│   ├── qruov3q31L3v246m87
│   ├── qruov3q31L10v890m100
│   ├── qruov3q127L3v228m78
│   ├── qruov5q7L10v1490m190
│   ├── qruov5q31L3v324m114
│   ├── qruov5q31L10v1120m120
│   └── qruov5q127L3v306m105
│
├─ Optimized_Implementation		-- Optimized C implementation
│   ├── portable64
│   ├── Makefile
│   ├── README.txt
│   └── qruov_config.src
│
├─ Reference_Implementation		-- Unoptimized reference implementation
│   ├── ref
│   ├── Makefile
│   ├── README.txt
│   └── qruov_config.src
│
├─ Supporting_Documentation
│   ├── Intermediate_Values         -- Intermediate values folder
│   ├── qruov_Cover_Sheet.pdf   	-- Cover Sheet
│   └── qruov_Spec-v1.0.pdf	        -- Specification document
│
└── README.txt                  	-- This file
