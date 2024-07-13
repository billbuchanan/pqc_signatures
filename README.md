# Round 1 Additional Signatures - Development Branch

## Development Branch Details

### Description:
This is the development branch for the pqc_signatures project which evaluates the performance of the proposed PQC signatures within round 1 of the NIST Post-Quantum Cryptography Additional Signatures project. It aims to provide a implementation of the benchmarking schemes for Linux environments and eventually will incorporate Windows environments similar to the setup in the main branch of this repository.

The development branch may not always be in a fully functioning state and documentation may still need updated. The checkboxes below indicates whether the current development version is in a basic/fully functioning state and if the documentation is accurate for its current functionality. Regardless please keep this in mind and use the main branch if possible, thank you.

- [ ] Basic Functioning State
- [ ] Fully Functioning State
- [x] Up to date documentation
- [ ] Full detailed documentation


### Main Branch Documentation Details
All documentation contained within the main branch is located within the `docs` directory when included within the development branch. The README file for the main branch can be found here:

[Main-Branch-README](docs/MAIN_BRANCH_README.md)


### Current Task Checklist for Development Branch:

A list of the current task checklist can be found here:

[Dev-Branch-Checklist](./dev-branch-tasklist.md)



## Implementation Checklist
The following is a set of checklists which detail the algorithm currently implemented within the automated compiling and benchmarking bash scripts. A copy of this list can also be referred to in the `completed.txt` file found within the `test_data/` directory.

### Implemented
- 3WISE
- Biscuit
- CROSS
- EHTv3v4
- FAEST
- FuLeeca
- HAWK
- HuFu †
- MEDS
- MIRA
- PERK ††
- Enhanced_pqsigRM
- Raccoon
- RYDE
- SDitH - Hypercube Variations †††
- SPHINCS_alpha
- SQIsign
- UOV

> † IMPORTANT Dev branch Notice: Benchmarking HuFU can take a considerable amount of time, even when utilising a high performance machine. The option to skip testing the HuFU algorithm and its respective variations will be presented when executing the automated benchmarking script.  

> †† IMPORTANT Dev branch Notice: The PERK algorithm variations is currently partially functional apart from perk-256-short-3 and perk-256-short-5. This is due to a memory issue causing a segmentation fault within the default reference code submitted for those variations. This will be reviewed further but for the time being the rest of the PERK variations function correctly.

> ††† IMPORTANT Dev branch Notice: The SDitH algorithm variations is currently partially functional apart from the threshold variants. This is due to issues with the code that performs the signature verification for these variations. This will be reviewed further but for the time being the rest of the SDiTH-Hypercube variations function correctly. 


### Still to be Implemented
- AIMer
- ALTEQ
- Ascon_Sign
- DME_Sign
- EagleSign
- eMLE_Sig_2.0
- HAETAE
- HPPC
- KAZ_SIGN
- LESS
- MAYO
- MiRitH
- MQOM
- Preon
- PROV
- QR_UOV
- SDitH - Threshold Variations
- SNOVA
- SQUIRRELS
- TUOV
- VOX
- Wave
- Xifrat1_Sign_I

### Currently being Implemented 
- SQUIRRELS


#### **NOTE to Project Developers**

When integrating any algorithm within the development branch and the relevant automated bash scripts, please refer to the implementation convention found here:

[Alg-Implementation-Convention](src/modified_nist_src_files/IMPLEMENTATION_CONVENTION.md)

## Basic Setup Instructions
To setup the testing environment and perform basic benchmarking of the algorithms that have been mentioned in the implemented list above, please conduct the following steps.

**Setup the testing environment using the automated compiling script**
```
./setup
```

**Perform benchmarking**
The automated becnmakring of the implemented algorithms can be done by entering the following commands:
```
cd scripts
./sig-speed-test.sh
```

Results will be outputted to a txt file located within the generated `test_data/results` directory. At the moment the testing script must be executed within its respective directory. Future updates to the development branch will include Python parsing scripts that will take the performance metrics outputted to the txt file and store it within a CSV file. 