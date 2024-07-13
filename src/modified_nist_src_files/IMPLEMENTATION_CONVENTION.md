# Instructions for Implementation Conventions

### Main Steps
**When implementing the algorithm, please use the following convention to ensure automated bash scripts are able to function correctly:**

1) copy the relevant algorithm directory from `src/modified_nist_src_files/still-to-implement` and place it in either `src/modified_nist_src_files/linux` or `src/modified_nist_src_files/windows`.

2) Copy any modified source files into the relevant algorithm directory and depending on how the source code is implemented, ensure that filenames have the original filename with the variation appended onto the end. If the source code has only one set of source code files and creates its variations through its makefiles, please still append the algorithm name onto the end. Examples for the possible scenarios are as can be seen referenced below.

3) Ensure that added functionality to the automated bash script temporally renames the original file so that is not used within the compilation and after deletes the modified file and restores the name of the original file so that commits to the branch retain the default source code.

### Examples of Modified File Naming Conventions
For instances where there are multiple copies of the source code for each variation refer to following examples:
- `src/modified_nist_src_files/linux/3WISE`
- `src/modified_nist_src_files/linux/HuFu`


For Instances where there is one set of source code and the variations are created using flags at compile, refer to the following examples:
- `src/modified_nist_src_files/linux/cross`


For instances where a CMakelist.txt file is used by the source code, where it may be the main CMakelist.txt file or a secondary CMakelist.txt file is used, refer to following examples:
- `src/modified_nist_src_files/linux/sqi`
