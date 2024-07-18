# Round 1 Additional Signatures - Development Branch

## Development Branch Details

### Description:
This is the development branch for the pqc_signatures project which evaluates the performance of the proposed PQC signatures within round 1 of the NIST Post-Quantum Cryptography Additional Signatures project. It aims to provide a implementation of the benchmarking schemes for Linux environments and eventually will incorporate Windows environments similar to the setup in the main branch of this repository.

The development branch may not always be in a fully functioning state and documentation may still need updated. The checkboxes below indicates whether the current development version is in a basic/fully functioning state and if the documentation is accurate for its current functionality. Regardless please keep this in mind and use the main branch if possible, thank you.

- [x] Basic Functioning State
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
- Ascon_Sign
- Biscuit
- CROSS
- DME_Sign †
- EHTv3v4
- eMLE_Sig_2.0
- FAEST
- FuLeeca
- HAWK
- HuFu ††
- MAYO
- MEDS
- MIRA
- PERK †††
- Enhanced_pqsigRM
- PROV
- Raccoon
- RYDE
- SDitH - Hypercube Variations ††††
- SPHINCS_alpha
- SQIsign
- TUOV
- UOV
- VOX
- Xifrat1_Sign_I

> † IMPORTANT Dev branch Notice: All variations for DME_SIGN function correctly apart from dme-3rnds-8vars-64bits-sign. This is due to the reference code for that variation not including all the correct source code files. This may of been intentional on the authors part, as the optimised implementation contains the correct source code files. This will be reviewed further, as all other schemes implemented use the reference code submitted to NIST, so deviating for this specific algorithm/variation may be problematic for the performance metrics gathered. For the moment, the automated compiling and benchmarking scripts will skip this variation until a decision has been made on how to proceed going forward.

> †† IMPORTANT Dev branch Notice: Benchmarking HuFU can take a considerable amount of time, even when utilising a high performance machine. The option to skip testing the HuFU algorithm and its respective variations will be presented when executing the automated benchmarking script.  

> ††† IMPORTANT Dev branch Notice: The PERK algorithm variations is currently partially functional apart from perk-256-short-3 and perk-256-short-5. This is due to a memory issue causing a segmentation fault within the default reference code submitted for those variations. This will be reviewed further but for the time being the rest of the PERK variations function correctly.

> †††† IMPORTANT Dev branch Notice: The SDitH algorithm variations is currently partially functional apart from the threshold variants. This is due to issues with the code that performs the signature verification for these variations. This will be reviewed further but for the time being the rest of the SDiTH-Hypercube variations function correctly. 


### Still to be Implemented
- AIMer
- ALTEQ
- EagleSign
- HAETAE
- HPPC
- KAZ_SIGN
- LESS
- MiRitH
- MQOM
- Preon
- QR_UOV
- SDitH - Threshold Variations
- SNOVA
- SQUIRRELS
- Wave
- SQUIRRELS

### Currently being Implemented 



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

## Script Descriptions for Developers

### Main Scripts
The following are the main scripts used within the round-1 signatures benchmarking suite for Linux environments.

#### **setup.sh**

Located in the project's root directory, it has the following description:

This is the main setup script for evaluating the round-1 signatures in a linux testing environment. The script sets up the required environment for the automated compilation of the algorithms and compiles the relevant testing binaries. It calls relevant utility scripts to read in variation arrays for the different algorithms and copy over modified source files to the relevant directories for compilation

#### **sig-speed-test.sh**
Located within the `scripts` directory, it has the following description:
This script is used to run the signature speed tests for all algorithms and variations in the benchmarking suite. The script will run the tests for the number of runs specified by the user and output the results to a txt file to later be parsed by the python parser script (once implemented). The script will also prompt the user to determine if HuFu is to be included in the benchmarking due to its high run time.


### Utility Scripts
The following are utility scripts provide functionality for the previously mentioned main scripts in the repository. They are not intended to be used by themselves and should be called from within one of the main scripts.

#### **variation_array_util.sh**
Located within the `scripts` directory, it has the following description:

This utility script is used to load in the algorithm variations from the text files in the alg_variation_lists directory. These arrays are then converted to comma-separated strings and exported as environment variables which can be imported into the calling script. Once the arrays have been imported into the calling script, the environment variables are cleared. The script is passed the following arguments when called:

- arg-1 (operation-flag): 'set' or 'clear' - determines if the environment variables are to be set or cleared
- arg-2 (alg_variations_dir): The path to the alg_variation_lists directory containing the algorithm variation text files

#### **copy_modifed_src_files.sh**
Located within the `scripts` directory, it has the following description:

This utility script is called by the main-setup script for linux environments 
to copy or restore modified source files for each algorithm and their respective variations. The script is called with the following arguments:

- arg-1 (util_flag): flag to determine if modified source files are to be copied or restored
- arg-2 (current_alg): the current algorithm being processed
- arg-3 (dst_variation_dir): the destination directory where the modified source files are to be copied or restored
- arg-4 (current_variation): the current variation of the algorithm being processed
- arg-5 (root_dir): the root directory of the project