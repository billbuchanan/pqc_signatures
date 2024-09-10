# Round 1 Additional Signatures - Development Branch <!-- omit from toc --> 

## Contents <!-- omit from toc --> 
- [Repository Overview](#repository-overview)
- [Compatibility Information](#compatibility-information)
  - [Supported Hardware](#supported-hardware)
  - [Supported PQC Signature Algorithms](#supported-pqc-signature-algorithms)
- [Installation Instructions](#installation-instructions)
- [Running Performance Benchmarking](#running-performance-benchmarking)
- [Automated Benchmarking with Bash](#automated-benchmarking-with-bash)
  - [Script Usage](#script-usage)
  - [Results Output](#results-output)
- [Automated Benchmarking with Golang](#automated-benchmarking-with-golang)
  - [Golang Setup](#golang-setup)
  - [Script Usage](#script-usage-1)
  - [Results Output](#results-output-1)
- [Data Visualiser Tool](#data-visualiser-tool)
- [Information for Developers](#information-for-developers)
- [Useful Links](#useful-links)

## Repository Overview

### Project Description <!-- omit from toc --> 
This is the development branch for the pqc_signatures project which evaluates the performance of the proposed PQC signatures within round 1 of the NIST Post-Quantum Cryptography Additional Signatures project. It aims to provide a implementation of the benchmarking schemes for Linux environments and eventually will incorporate Windows environments similar to the setup in the main branch of this repository.

Dilithium, FALCON and SPHINCS+ have become NIST standards for digital signatures, and with an aim to remove RSA, ECDSA and EdDSA. But, NIST wants alternatives to these, especially so that we are not too dependent on lattice-based approaches (such as with Dilithium and FALCON). These are [here](https://csrc.nist.gov/projects/pqc-dig-sig/round-1-additional-signatures):

* Multivariate Signatures (10): 3WISE, DME-Sign, HPPC (Hidden Product of Polynomial Composition), MAYO, PROV (PRovable unbalanced Oil and Vinegar), QR-UOV, SNOVA, TUOV (Triangular Unbalanced Oil and Vinegar), UOV (Unbalanced Oil and Vinegar), and VOX.
* MPC-in-the-Head Signatures (7): Biscuit, MIRA, MiRitH (MinRank in the Head), MQOM (MQ on my Mind), PERK, RYDE, and SDitH (Syndrome Decoding in the Head).
* Lattice-based Signatures (6): EagleSign, EHTv3 and EHTv4, HAETAE, HAWK, HuFu (Hash-and-Sign Signatures From Powerful Gadgets), and SQUIRRELS ( Square Unstructured Integer Euclidean Lattice Signature).
* Code-based Signatures (5): CROSS (Codes and Restricted Objects Signature), Enhanced pqsigRM, FuLeeca, LESS (Linear Equivalence), MEDS (Matrix Equivalence Digital Signature).
* Symmetric-based Signatures (4): AIMer, Ascon-Sign, FAEST, and SPHINCS-alpha.
* Other Signatures (4): ALTEQ, eMLE-Sig 2.0 (Embedded Multilayer Equations with Heavy Layer Randomization), KAZ-SIGN (Kriptografi Atasi Zarah), Preon, and Xifrat1-Sign.
* Isogeny Signatures (1): SQIsign.

The current standards are:

* Falcon. [Falcon](https://asecuritysite.com/pqc/falcon_sign). Falcon is a NIST approved standard PQC (Post Quantum Cryptography) digital signatures. It is derived from NTRU ((Nth degree‐truncated polynomial ring units) and is a lattice-based methods for quantum robust digital signing. Falcon is based on the Gentry, Peikert and Vaikuntanathan method for generating lattice-based signature schemes, along with a trapdoor sampler - Fast Fourier sampling. We select three parameters: N, p and q. To generate the key pair, we select two polynomials: f and g. From these we compute: F=fq=f−1(modq) and where f and fq are the private keys. The public key is h=p⋅fq.f(modq). With Falcon-512 (which has an equivalent security to RSA-2048), we generate a public key of 897 bytes, and a signature size of 666 bytes, while FALCON-1024 gives a public key of 1,793 bytes and a signature size of 1,280 bytes.
* Dilithium. [Dilithium](https://asecuritysite.com/pqc/dilithium_sign). At present, CRYSTALS (Cryptographic Suite for Algebraic Lattices) supports two quantum robust mechanisms: Kyber for key-encapsulation mechanism (KEM) and key exchange; and Dilithium for a digital signature algorithm. CRYSTALS Dilithium uses lattice-based Fiat-Shamir schemes, and produces one of the smallest signatures of all the post-quantum methods, and with relatively small public and private key sizes. The three main implements for the parameters used are: Dilithium 2, Dilithium 3 and Dilithium 5. Overall, Dilithium 3 is equivalent to a 128-bit signature, and is perhaps the starting point for an implementation.

### Dev Branch Status <!-- omit from toc --> 
The development branch may not always be in a fully functioning state and documentation may still need updated. The checkboxes below indicates whether the current development version is in a basic/fully functioning state and if the documentation is accurate for its current functionality. Regardless please keep this in mind and use the main branch if possible, thank you.

- [x] Basic Functioning State
- [ ] Fully Functioning State
- [x] Up to date documentation

Details on the current development branch tasks can be found here:

[Development Branch Task List](docs/dev_branch_tasklist.md)



## Compatibility Information

### Supported Hardware
The additional signatures benchmarking tool is currently only supported on the following devices:

- x86 Linux Machines using a Debian based operating system
<!-- ARM Linux devices using a 64-bit Debian based Operating System -->

As the project progresses, the goal will be that both Linux and Windows based operating systems will be supported by this tool and will allow for benchmarking on a wide variety of system types.

### Supported PQC Signature Algorithms
The following PQC signature algorithms are currently supported by this tool:

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
- MQOM
- PERK †††
- Preon
- Enhanced_pqsigRM
- PROV
- QR_UOV
- Raccoon
- RYDE
- SDitH
- SNOVA †††
- SPHINCS_alpha
- SQIsign
- SQUIRRELS
- TUOV
- UOV
- VOX
- Wave
- Xifrat1_Sign_I

> † IMPORTANT Dev branch Notice: All variations for DME_SIGN function correctly apart from dme-3rnds-8vars-64bits-sign. This is due to the reference code for that variation not including all the correct source code files. This may of been intentional on the authors part, as the optimised implementation contains the correct source code files. This will be reviewed further, as all other schemes implemented use the reference code submitted to NIST, so deviating for this specific algorithm/variation may be problematic for the performance metrics gathered. For the moment, the automated compiling and benchmarking scripts will skip this variation until a decision has been made on how to proceed going forward.
>
> It has been included within the implemented section due to the majorty of the variations functioning, but the scheme as also been marked for team debugging to resolve the issues with the remaining variations.

> †† IMPORTANT Dev branch Notice: Benchmarking HuFU can take a considerable amount of time, even when utilising a high performance machine. The option to skip testing the HuFU algorithm and its respective variations will be presented when executing the automated benchmarking script.  

> ††† IMPORTANT Dev branch Notice: Benchmarking SNOVA can take a considerable amount of time, even when utilising a high performance machine. The option to skip testing the SNOVA algorithm and its respective variations will be presented when executing the automated benchmarking script.

More detailed information on the algorithms can be found in the following documentation:

[Algorithm Details Documentation](docs/algorithm_details.md)

## Installation Instructions
To install and configure the developing branch version of the benchmarking tool which supports operations on Linux operating, please conduct the following steps.

Clone the repository and change to the developing branch:
```
git clone https://github.com/billbuchanan/pqc_signatures.git
cd pqc_signatures
git checkout developing
```

Once the branch has been changed, it is now possible to begin the setup process. Within the projects root, there is a main setup bash script, which will automatically compile the additional signatures and their variations. The benching binaries are then outputted to the generated bin dir at the projects root. To initiate the setup process, execute the following command:

```
./setup.sh
```

This setup may take some time to complete, especially on more limited systems. As the project continues to progress, this process will be further optimised. After the setup script has finished running, it is now possible to begin running the automated benchmarking scripts to gather performance data on the PQC schemes.

## Running Performance Benchmarking
At the current moment, there are two methods that can be used in order to gather computational performance metrics for the PQC signature schemes:

1) **An automated bash script**
2) **An automated Golang script**

Both options are valid methods of gathering the data depending on the requirements of the user.

> **IMPORTANT:**  
> Run `./setup` in the root directory before executing any other scripts. All scripts should be run from the `scripts/` directory. This is due to a current lack of exception handling for scripts being executed from other directories. This will be addressed in future updates to the project.

**NOTE:** At the current moment, the benchmarking binary is only able to output the number of CPU cycles required to complete the various cryptographic operations for each algorithm. In the future, the tool will be able to produce performance metrics such as time to complete and memory usage.

## Automated Benchmarking with Bash

### Script Usage
Whilst the automated bash script does not offer as much functionality compared to the Golang version and it cannot be used with the data visualiser currently. It does offer the benefit of not requiring any additional dependencies and may be useful for those who wish to use their own parsing scripts using languages such as Python. the automated process for gathering performance data by looping through and running the benchmarking binary for each algorithm and all of their variations.

To execute the performance using the bash script version, the following commands should be executed from the project's root directory:

```
cd scripts 
./sig_speed_test.sh
```

During the scripts operation the user will be presented to skip the HuFu and/or Snova algorithms due to the significant amount of time it takes to complete their operations. Furthermore, the script will also present the user with the option to select the number of runs that the script will perform. This is to allow for the generation of performance data averages across multiple runs for the supported signature algorithms.

### Results Output
Upon completing the benchmarking, the automated bash script will store the results in the `test_data/results` directory in the form of text files. These text files can later be parsed using a language such as Python to extract the performance metrics.

**NOTE:** **Currently there is no handling to deal with previous results being present when running the performance script. Upon execution, all previous results will be DELETED! Please make a copy of the results if you wish to keep them before executing the script again!**


## Automated Benchmarking with Golang
Whilst the automated bash script can offer a simpler approach, it can at times be considerable slow. However, The Golang version of the automated benchmarking has the benefit of running the various benchmarks in parallel, where multiple binaries can be executed at once, reducing the overall testing time. Furthermore, the Golang version outputs the results in txt, CSV, and JSON format unlike the bash script version which outputs only text files.

For more information on how the Golang version of the automated benchmarking script works, please refer to the its documentation which can be found here:

[Automated Golang Benchmarking Solution Overview]()

### Golang Setup
In order to use the Golang version of the automated benchmarking script, Golang must be installed and configured on the system. You can verify if the Golang is present on the system and is accessible from the PATH by executing the following command:

```
go version
```

If this command completes and produces an output similar to the example below. Then the the required setup is already done for being able to use the Golang script provided by this project.

```
go version go1.22.6 linux/amd64
```

If Golang is not already present on your system, then it can be downloaded from their [official website](https://go.dev/dl/). Detailed documentation on how to setup Golang on your system can also be found on the [Go Install Documentation](https://go.dev/doc/install) page.

### Script Usage
With Golang present on the system, the automated  benchmarking script can now be compiled and executed. This must be done from within the `scripts` directory as currently there is no exception handling to deal with execution from different directories in this project. Going forward this issue will be addressed.

To build the project and then perform the benchmarking, execute the following commands from the project's root directory:

```
cd scripts
go build -o output_filename
./output_filename
```

It is also possible to run the script directly using the following commands from the project's root directory:

```
cd scripts
go run main.go
```

Similar 


> **Note to Developers**: There is currently a .gitignore entry for the built binary being called sig_bench. If you decide to user another filename please either create an entry in the .gitignore file for that name or delete the binary before committing any changes. This is to avoid any unessecery files being present on the branch.

### Results Output
The Golang version outputs the results in txt, CSV, and JSON format which can be used with the data visualiser tool. The script has the following results output information:

- Results are stored in the `test_data/results/` directory with filenames in the format `results_<run_number>_<YYYYMMDD>`, where `<run_number>` represents the run number and `<YYYYMMDD>` represents the current date. Files are overwritten if the script is executed on the same date.

- Output formats include `.json`, `.csv`, and `.txt` files.


**NOTE:** **Currently there is no handling to deal with previous results being present when running the performance script. Upon execution, all previous results will be DELETED! Please make a copy of the results if you wish to keep them before executing the script again!**

## Data Visualiser Tool
This project also a data visualiser tool to review the performance data that was outputted by the automated Golang benchmarking script. This tool takes the JSON file version of the performance results outputted by the script and provides an interface for viewing the CPU cycles required to complete the cryptographic operations of the various algorithms.

When the the benchmarking has completed, the results can be viewed in the data visualiser tool by opening the **page.html** (located in the `scripts` directory) in a web browser. Once opened, a JSON file containing the results can be selected to be reviewed using the GUI present in the web interface. These files can be found in the `testd_data/results` directory. After a results file has been selected for review, the user can select one of the various signature algorithms that were benchmarked. This will display the CPU cycles performance data for each of that algorithms variations for the three cryptographic operations:

- Key Generation
- Signing
- Verifying

## Information for Developers
If you are contributing to the project, detailed information that is relevant to developers can be found in the following documentation:

- [Main Developer Information Documentation](docs/developer_info.md)
- [Automated Golang Benchmarking Solution Overview](docs/golang_solution_documentation.md)
- [Algorithm Details Documentation](docs/algorithm_details.md)
- [Development Branch Task List](docs/dev_branch_tasklist.md)


## Useful Links
- [Asecuritysite PQC Signatures](https://asecuritysite.com/pqc_sign/)
- [NIST Round 1 Additional Signatures](https://csrc.nist.gov/projects/pqc-dig-sig/round-1-additional-signatures)
- [Main NIST PQC Standardisation Project](https://csrc.nist.gov/projects/post-quantum-cryptography)