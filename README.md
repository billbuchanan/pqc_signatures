# Round 1 Additional Signatures - Development Branch <!-- omit from toc --> 

- [Development Branch Details](#development-branch-details)
  - [Description](#description)
  - [Main Branch Documentation Details](#main-branch-documentation-details)
  - [Current Task Checklist for Development Branch](#current-task-checklist-for-development-branch)
  - [Documentation for Developers](#documentation-for-developers)
- [Implementation Checklist](#implementation-checklist)
  - [Implemented](#implemented)
  - [Marked for Team Debugging](#marked-for-team-debugging)
  - [Algorithm Team Debugging Implementation Stage Table](#algorithm-team-debugging-implementation-stage-table)
  - [Info Table for Schemes Marked for Team Debugging that have been fixed](#info-table-for-schemes-marked-for-team-debugging-that-have-been-fixed)
- [Basic Setup Instructions](#basic-setup-instructions)
- [Information for Developers](#information-for-developers)
  - [Development Setup Script Usage](#development-setup-script-usage)
  - [Script Descriptions](#script-descriptions)
  - [Main Scripts](#main-scripts)
  - [Utility Scripts](#utility-scripts)
  - [Development Scripts](#development-scripts)



## Development Branch Details

### Description
This is the development branch for the pqc_signatures project which evaluates the performance of the proposed PQC signatures within round 1 of the NIST Post-Quantum Cryptography Additional Signatures project. It aims to provide a implementation of the benchmarking schemes for Linux environments and eventually will incorporate Windows environments similar to the setup in the main branch of this repository.

The development branch may not always be in a fully functioning state and documentation may still need updated. The checkboxes below indicates whether the current development version is in a basic/fully functioning state and if the documentation is accurate for its current functionality. Regardless please keep this in mind and use the main branch if possible, thank you.

- [x] Basic Functioning State
- [ ] Fully Functioning State
- [x] Up to date documentation
- [ ] Full detailed documentation


### Main Branch Documentation Details
All documentation contained within the main branch is located within the `docs` directory when included within the development branch. The README file for the main branch can be found here:

[Main-Branch-README](docs/MAIN_BRANCH_README.md)


### Current Task Checklist for Development Branch

A list of the current task checklist can be found here:

[Dev-Branch-Checklist](./dev_branch_tasklist.md)


### Documentation for Developers
This README also contains documentation on the functionality of the various scripts used within this version of the project for any developer working on this branch of the repository found here - [Developer-Docs](#information-for-developers). This section covers details of the each of the automated scripts included within the project alongside usage instructions for the temporary `dev_setup.sh` script used for testing the functionality of the remaining algorithms still to be implemented.


## Implementation Checklist
The following is a set of checklists which detail the algorithm currently implemented within the automated compiling and benchmarking bash scripts. A copy of this list can also be referred to in the `completed.txt` file found within the `test_data/` directory.

### Implemented
- 3WISE
- AIMer
- ALTEQ
- Ascon_Sign
- Biscuit
- CROSS
- DME_Sign †
- EagleSign
- EHTv3v4
- eMLE_Sig_2.0
- FAEST
- FuLeeca
- HAETAE
- HAWK
- HPPC
- HuFu ††
- KAZ_SIGN
- LESS
- MAYO
- MEDS
- MIRA
- MiRitH
- PERK †††
- Enhanced_pqsigRM
- PROV
- QR_UOV
- Raccoon
- RYDE
- SDitH - Hypercube Variations ††††
- SNOVA †††††
- SPHINCS_alpha
- SQIsign
- TUOV
- UOV
- VOX
- Wave
- Xifrat1_Sign_I

> † IMPORTANT Dev branch Notice: All variations for DME_SIGN function correctly apart from dme-3rnds-8vars-64bits-sign. This is due to the reference code for that variation not including all the correct source code files. This may of been intentional on the authors part, as the optimised implementation contains the correct source code files. This will be reviewed further, as all other schemes implemented use the reference code submitted to NIST, so deviating for this specific algorithm/variation may be problematic for the performance metrics gathered. For the moment, the automated compiling and benchmarking scripts will skip this variation until a decision has been made on how to proceed going forward.
>
> It has been included within the implemented section due to the majorty of the variations functioning, but the scheme as also been marked for team debugging to resolve the issues with the remaining variations.

> †† IMPORTANT Dev branch Notice: Benchmarking HuFU can take a considerable amount of time, even when utilising a high performance machine. The option to skip testing the HuFU algorithm and its respective variations will be presented when executing the automated benchmarking script.  

> ††† IMPORTANT Dev branch Notice: The PERK algorithm variations is currently partially functional apart from perk-256-short-3 and perk-256-short-5. This is due to a memory issue causing a segmentation fault within the default reference code submitted for those variations. This will be reviewed further but for the time being the rest of the PERK variations function correctly.
>
> It has been included within the implemented section due to the majorty of the variations functioning, but the scheme as also been marked for team debugging to resolve the issues with the remaining variations.

> †††† IMPORTANT Dev branch Notice: The SDitH algorithm variations is currently partially functional apart from the threshold variants. This is due to issues with the code that performs the signature verification for these variations. This will be reviewed further but for the time being the rest of the SDiTH-Hypercube variations function correctly. 

> ††††† IMPORTANT Dev branch Notice: Benchmarking SNOVA can take a considerable amount of time, even when utilising a high performance machine. The option to skip testing the SNOVA algorithm and its respective variations will be presented when executing the automated benchmarking script.  

### Marked for Team Debugging
- MQOM
- Preon
- SDitH - Threshold Variations
- SQUIRRELS
- DME_Sign
- PERK

### Algorithm Team Debugging Implementation Stage Table
| **Algorithm**                | **Default Source Code in Modified Dir** | **Modifications Being Made to Default Files** | **Algorithm Able to Compile** | **PQCSign Binary Functioning** | **Notes**                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
|------------------------------|-----------------------------------------|-----------------------------------------------|-------------------------------|--------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| DME_SIGN                     | X                                       | X                                             | X                             |                                | All variations for DME_SIGN function correctly apart from dme-3rnds-8vars-64bits-sign. This is due to the reference code for that variation not including all the correct source code files. This may of been intentional on the authors part, as the optimised implementation contains the correct source code files. This will be reviewed further, as all other schemes implemented use the reference code submitted to NIST, so deviating for this specific algorithm/variation may be problematic for the performance metrics gathered. For the moment, the automated compiling and benchmarking scripts will skip this variation until a decision has been made on how to proceed going forward. |
| MQOM                         | X                                       | X                                             | X                             |                                | The pqcsign binary is able to compile, is able to perform key generation/signing operations, but an error occurs when verifying the signature. The default benchmarking and PQCKat_gen scripts are able to function correctly, so the problem most likely lies with the pqcsign script or how it is being compiled. The parameters displayed by the pqcsign script also appear to be inaccurate as the CRYPTO_BYTES macro is showing a higher value than shown in the algorithm specification, but this needs to be verified. This algorithm has been added to the dev_seutp.sh script for team debugging.                                                                                             |
| PERK                         | X                                       | X                                             | X                             |                                | The PERK algorithm variations is currently partially functional apart from perk-256-short-3 and perk-256-short-5. This is due to a memory issue causing a segmentation fault within the default reference code submitted for those variations. This will be reviewed further but for the time being the rest of the PERK variations function correctly.                                                                                                                                                                                                                                                                                                                                                |
| Preon                        | X                                       | X                                             | X                             |                                | The pqcsign binary is able to compile, is able to perform key generation/signing operations, but a segmentation fault occurs when performing verification. This algorithm has been added to the dev_seutp.sh script for team debugging.                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| SDitH - Threshold Variations | X                                       | X                                             | X                             |                                | The pqcsign binary is able to compile, is able to perform key generation/signing operations, but an error occurs when verifying the signature. The default benchmarking and PQCKat_gen scripts are able to function correctly, so the problem most likely lies with the pqcsign script or how it is being compiled. This algorithm has been added to the dev_seutp.sh script for team debugging.                                                                                                                                                                                                                                                                                                       |
| SQUIRRELS                    | X                                       | X                                             | X                             |                                | The pqcsign binary is able to compile, is able to perform key generation/signing operations, but an error occurs when verifying the signature. This algorithm has been added to the dev_seutp.sh script for team debugging.                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |


### Info Table for Schemes Marked for Team Debugging that have been fixed
| **Algorithm** | **Default Source Code in Modified Dir** | **Modifications Being Made to Default Files** | **Algorithm Able to Compile** | **PQCSign Binary Functioning** | **Notes**                                                                                                                                                                                                                                                                                                                                                                                                                                       |
|---------------|-----------------------------------------|-----------------------------------------------|-------------------------------|--------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| AIMer         | X                                       | X                                             | X                             | X                              | NIST API files in default reference code did not have the correct signature size for parameter sets for each security level apart from parameter set 1.                                                                                                                                                                                                                                                                                         |
| EagleSign     | X                                       | X                                             | X                             | X                              | The NIST API files differed enough that special cases needed to be added for parameter definitions and AES256ctr source code files were not present and had to be included to compile correctly                                                                                                                                                                                                                                                 |
| HAETAE        | X                                       | X                                             | X                             | X                              | The NIST API differed too greatly from the other schemes, a refactored version of the API file and creation of API.c file was needed to compile correctly. Also the shared libraries needed to be moved with pqcsign binaries to function correctly.                                                                                                                                                                                            |
| KAZ_SIGN      | X                                       | X                                             | X                             | X                              | A separate function within pqcsign had to be created for the scheme due the significant difference in API usage.                                                                                                                                                                                                                                                                                                                                |
| LESS          | X                                       | X                                             | X                             | X                              | The reference implementation of the LESS scheme did not come with a executable binary and a CMakeLists.txt file. A CMakeLists file had to be created based of the one included with the additional implementations code and modified to work with the pqcsign binary.                                                                                                                                                                           |
| MiRitH        | X                                       | X                                             | X                             | X                              | A separate function within pqcsign had to be created that closely followed the functionality seen in the test_mirith.c file in the default source code due to the significant difference in API usage.                                                                                                                                                                                                                                          |
| Wave          | X                                       | X                                             | X                             | X                              | A separate function within pqcsign had to be created that closely followed the functionality seen in the benchmark.c file in the default source code due to the significant difference in API usage. IMPORTANT NOTE - In order to get the wave1644 variation to work, a temporary fix which sets an unlimted stack size had to be used to allow the pqcsign binary to run. This will need to be resolved going forward, but it works for now.   |




## Basic Setup Instructions
To setup the testing environment and perform basic benchmarking of the algorithms that have been mentioned in the implemented list above, please conduct the following steps.

**Setup the testing environment using the automated compiling script**
```
./setup
```

**Perform benchmarking**
The automated benchmarking of the implemented algorithms can be done by entering the following commands:
```
cd scripts
./sig_speed_test.sh
```

Results will be outputted to a txt file located within the generated `test_data/results` directory. At the moment the testing script must be executed within its respective directory. Future updates to the development branch will include Python parsing scripts that will take the performance metrics outputted to the txt file and store it within a CSV file. 

## Information for Developers

#### **NOTE to Project Developers on Algorithm Implementation**

When integrating any algorithm within the development branch and the relevant automated bash scripts, please refer to the implementation convention found here:

[Alg-Implementation-Convention](src/modified_nist_src_files/IMPLEMENTATION_CONVENTION.md)

### Development Setup Script Usage
#### **Information on General Usage**
The development setup script (`dev_setup.sh`) can be used to easily test the functionality needed to implement the remaining algorithms into the main setup script. This is helpful due to the current size of the main setup script alongside the length of time it takes to fully compile all of the reference code. The development setup script also includes functionality that allows the developer to test the benchmarking binaries after the algorithms and their respective variations have been compiled.

When including functionality for a algorithm, please ensure proper usage of the `copy_modifed_src_files.sh` utility script to avoid any accidental inclusion or modification of the default source code files within a commit.

#### Running the Development Setup Script
The development script can be exucted using the following command:

```
./dev_setup.sh
```

The development setup script will also provide the option on where to direct the output of compilation portion of the script:

- Option-1: Direct the output to terminal
- Option-2: Direct the output to a text file

**Option 1** would be used if there is a small number of algorithms currently included with the development script, or storing all the outputs is not necessary. 
 
**Option 2** is ideal if there are few algorithms that are currently included within the development setup script or there are errors that require further review. This option can also be beneficial as it can improve readability and make it easier to debug issues with compilation.


**It is important to note** that the output filename in the script will be the same each time, so previous outputs will be overwritten. There is no issue in using different filenames, but these are not included within the .gitignore file by default, so if additional copies are used, please either create an entry within the .gitignore file or store it outwith the projects directory to avoid unnecessary files in the repository.

**Including Benchmarking**

After starting the development script, the user will also be presented with a (y/n) option to test the benchmarking binaries after the algorithms have been compiled before starting. This can be useful in determining if even though the algorithm compiles successfully, does any further modification to the `pqcsign.c` script need to be made to allow for successful benchmarking.  

### Script Descriptions
### Main Scripts
The following are the main scripts used within the round-1 signatures benchmarking suite for Linux environments.

#### **setup.sh**

Located in the project's root directory, it has the following description:

This is the main setup script for evaluating the round-1 signatures in a linux testing environment. The script sets up the required environment for the automated compilation of the algorithms and compiles the relevant testing binaries. It calls relevant utility scripts to read in variation arrays for the different algorithms and copy over modified source files to the relevant directories for compilation

#### **pqcsign.c**
Located in the `src/performance_eval_scripts` directory, it has the following description:

This is the main benchmarking code used for gathering performance metrics for the signature schemes. This script is copied over to the relevant source code location for the current algorithm/variation currently being compiled. Outputted binaries are stored in `bin` directory at the project's root which is created during the setup. The compiled binary for the current algorithm variation is stored within the relevant directory for the scheme within the `bin` directory. This binary is then used by the `sig_speed_test.sh` script to automatically gather CPU cycle performance metrics for all the implemented algorithms and their variations.

#### **sig_speed_test.sh**
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

### Development Scripts
These are temporary scripts present in the repositories development branch to aid in the integration of the proposed schemes into the automated compiling and benchmarking scripts. If you are not developing the project you can safely ignore these scripts and instead use the main scripts previously described to compile and benchmark the currently supported Algorithms. The following is a description of the current `dev_setup.sh` script:

#### **dev-script.sh**
Located within the repositories root directory, it has the following description:

This is the temporary development setup script for testing the integration of the algorithms within main automated compiling script. Its main purpose is to implement and store the output of the compilation process for the algorithms still to be implemented to easily test the setup process and capture any errors outputted. It also contains a function for testing the pqcsign binary of the algorithms to test if the outputted binary is also functioning correctly. Once all algorithms have been successfully implemented within the project, this script will be removed from the repository.
