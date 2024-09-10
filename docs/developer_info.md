# Information for Developers <!-- omit from toc --> 

## Contents <!-- omit from toc --> 
- [Development Setup Script Usage](#development-setup-script-usage)
  - [Information on General Usage](#information-on-general-usage)
  - [Running the Development Setup Script](#running-the-development-setup-script)
- [Script Descriptions](#script-descriptions)
  - [Main Scripts](#main-scripts)
  - [Utility Scripts](#utility-scripts)
  - [Development Scripts](#development-scripts)
- [Instructions for Implementation Conventions](#instructions-for-implementation-conventions)
  - [Main Steps](#main-steps)
  - [Examples of Modified File Naming Conventions](#examples-of-modified-file-naming-conventions)


## Development Setup Script Usage

### Information on General Usage
The development setup script (`dev_setup.sh`) can be used to easily test the functionality needed to implement the remaining algorithms into the main setup script. This is helpful due to the current size of the main setup script alongside the length of time it takes to fully compile all of the reference code. The development setup script also includes functionality that allows the developer to test the benchmarking binaries after the algorithms and their respective variations have been compiled.

When including functionality for a algorithm, please ensure proper usage of the `copy_modifed_src_files.sh` utility script to avoid any accidental inclusion or modification of the default source code files within a commit.

### Running the Development Setup Script
The development script can be executed using the following command:

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

## Script Descriptions

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

## Instructions for Implementation Conventions

### Main Steps
**When implementing an algorithm, please use the following convention to ensure automated bash scripts are able to function correctly:**

1) copy the relevant algorithm directory from `src/modified_nist_src_files/still-to-implement` and place it in either `src/modified_nist_src_files/linux` or `src/modified_nist_src_files/windows`.

2) Copy any modified source files into the relevant algorithm directory and depending on how the source code is implemented, ensure that filenames have the original filename with the variation appended onto the end. If the source code has only one set of source code files and creates its variations through its makefiles, please still append the algorithm name onto the end. Examples for the possible scenarios are as can be seen referenced below.

3) **Ensure** that added functionality to the utility bash script (`scripts/copy_modified_src_files.sh`) temporally renames the original file so that is not used within the compilation and after deletes the modified file and restores the name of the original file so that commits to the branch retain the default source code.

4) **Ensure** that any modified contains stored contain a comment on line 1 which has the string "modified-flag", i.e `#modified-flag` or `//modified-flag`. This is vital as it used by the default source exception handling within the utility script.

### Examples of Modified File Naming Conventions
For instances where there are multiple copies of the source code for each variation refer to following examples:
- `src/modified_nist_src_files/linux/3WISE`
- `src/modified_nist_src_files/linux/HuFu`


For Instances where there is one set of source code and the variations are created using flags at compile, refer to the following examples:
- `src/modified_nist_src_files/linux/cross`


For instances where a CMakelist.txt file is used by the source code, where it may be the main CMakelist.txt file or a secondary CMakelist.txt file is used, refer to following examples:
- `src/modified_nist_src_files/linux/sqi`
