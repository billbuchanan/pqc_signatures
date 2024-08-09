#!/bin/bash
# This is the temporary development setup script for testing the integration of the algorithms within main automated compiling script.
# Its main purpose is to implement and store the output of the compilation process for the algorithms still to be implemented to easily 
# test the setup process and capture any errors outputted. It also contains a function for testing the pqcsign binary of the algorithms to 
# test if the outputted binary is also functioning correctly. Once all algorithms have been successfully implemented within the project, this
# script will be removed from the repository.

# NOTICE - If you are not developing this project, you can safely ignore this script and can instead use the main setup
# and benchmarking scripts to test the algorithms which are currently supported. A list of currently supported algorithms can be found
# on the repositories development branch README (https://github.com/billbuchanan/pqc_signatures/tree/developing)
# build

#------------------------------------------------------------------------------
# Set global path variables
root_dir=$(pwd)
src_dir=$root_dir/src
nist_src_dir=$src_dir/nist
bin_dir=$root_dir/bin
test_data_dir=$root_dir/test_data
results_dir=$test_data_dir/results
algs_list_dir=$test_data_dir/sig_algs_list
alg_variations_dir=$test_data_dir/alg_variation_lists
scripts_dir=$root_dir/scripts
num_runs=0


#---------------------------------------------------------------------------------------------------
function array_util_call() {
    # Function for calling the array utility script to set the variation arrays used in the script

    # Call the array utility script to export the variation arrays
    source "$root_dir/scripts/variation_array_util.sh" "set" "$alg_variations_dir"

    # Import the variation arrays from environment variables
    IFS=',' read -r -a three_wise_variations <<< "$THREE_WISE_VARIATIONS"
    IFS=',' read -r -a aimer_variations <<< "$AIMER_VARIATIONS"
    IFS=',' read -r -a alteq_variations <<< "$ALTEQ_VARIATIONS"
    IFS=',' read -r -a ascon_sign_variations <<< "$ASCON_SIGN_VARIATIONS"
    IFS=',' read -r -a biscuit_variations <<< "$BISCUIT_VARIATIONS"
    IFS=',' read -r -a cross_variations <<< "$CROSS_VARIATIONS"
    IFS=',' read -r -a dme_sign_variations <<< "$DME_SIGN_VARIATIONS"
    IFS=',' read -r -a eaglesign_variations <<< "$EAGLESIGN_VARIATIONS"
    IFS=',' read -r -a ehtv3v4_variations <<< "$EHTV3V4_VARIATIONS"
    IFS=',' read -r -a emle_sig_2_0_variations <<< "$EMLE_SIG_2_0_VARIATIONS"
    IFS=',' read -r -a pqsigrm_variations <<< "$PQSIGRM_VARIATIONS"
    IFS=',' read -r -a faest_variations <<< "$FAEST_VARIATIONS"
    IFS=',' read -r -a fuleeca_variations <<< "$FULEECA_VARIATIONS"
    IFS=',' read -r -a HAETAE_variations <<< "$HAETAE_VARIATIONS"
    IFS=',' read -r -a hawk_variations <<< "$HAWK_VARIATIONS"
    IFS=',' read -r -a hppc_variations <<< "$HPPC_VARIATIONS"
    IFS=',' read -r -a hufu_variations <<< "$HUFU_VARIATIONS"
    IFS=',' read -r -a kaz_sign_variations <<< "$KAZ_SIGN_VARIATIONS"
    IFS=',' read -r -a less_variations <<< "$LESS_VARIATIONS"
    IFS=',' read -r -a mayo_variations <<< "$MAYO_VARIATIONS"
    IFS=',' read -r -a med_variations <<< "$MED_VARIATIONS"
    IFS=',' read -r -a mira_variations <<< "$MIRA_VARIATIONS"
    IFS=',' read -r -a mirith_variations <<< "$MIRITH_VARIATIONS"
    IFS=',' read -r -a mqom_variations <<< "$MQOM_VARIATIONS"
    IFS=',' read -r -a perk_variations <<< "$PERK_VARIATIONS"
    IFS=',' read -r -a preon_variations <<< "$PREON_VARIATIONS"
    IFS=',' read -r -a prov_variations <<< "$PROV_VARIATIONS"
    IFS=',' read -r -a qr_uov_variations <<< "$QR_UOV_VARIATIONS"
    IFS=',' read -r -a raccoon_variations <<< "$RACCOON_VARIATIONS"
    IFS=',' read -r -a ryde_variations <<< "$RYDE_VARIATIONS"
    IFS=',' read -r -a sdith_hypercube_variations <<< "$SDITH_HYPERCUBE_VARIATIONS"
    IFS=',' read -r -a snova_variations <<< "$SNOVA_VARIATIONS"
    IFS=',' read -r -a sphincs_alpha_variations <<< "$SPHINCS_ALPHA_VARIATIONS"
    IFS=',' read -r -a sqi_variations <<< "$SQI_VARIATIONS"
    IFS=',' read -r -a squirrels_variations <<< "$SQUIRRELS_VARIATIONS"
    IFS=',' read -r -a tuov_variations <<< "$TUOV_VARIATIONS"
    IFS=',' read -r -a uov_variations <<< "$UOV_VARIATIONS"
    IFS=',' read -r -a vox_variations <<< "$VOX_VARIATIONS"
    IFS=',' read -r -a wave_variations <<< "$WAVE_VARIATIONS"
    IFS=',' read -r -a xifrat1_sign_variations <<< "$XIFRAT1_SIGN_VARIATIONS"
    
    # Call the array utility script to clear environment variables
    source "$scripts_dir/variation_array_util.sh" "clear"

}

#---------------------------------------------------------------------------------------------------
function determine_setup_options() {
    # Function to determine the number of runs to be performed set by the user

    # Determine if user wants to do benchmark testing before performing setup
    while true; do

        # Get user input for number of runs
        read -p "Would you like to perform benchmarking after completing compilation (y/n): " benchmark_choice
        benchmark_choice=$(echo $benchmark_choice | tr '[:upper:]' '[:lower:]')

        echo "benchmark choice: $benchmark_choice"

        # Check if input is valid and set choice flag
        if [ $benchmark_choice == "y" ]; then
            benchmark_included="1"
            break

        elif [ $benchmark_choice == "n" ]; then
            benchmark_included="0"
            break
        
        else
            echo -e "\nIncorrect input, please use (y/n) to indicate benchmark inclusion choice"

        fi

    done

    # Get the number of runs to be performed if benchmarking is included
    if [ $benchmark_included == "1" ]; then

        # Loop until valid input is received for the number of runs
        while true; do

            # Get user input for number of runs
            read -p "Enter the number of runs to be performed: " num_runs

            # Check if input is valid and set number of runs
            if [[ $num_runs =~ ^[0-9]+$ ]]; then
                break
            else
                echo -e "Invalid input. Please enter a positive integer\n"
            fi

        done

    fi

    # Loop until valid input is received for the output method
    while true; do
            
            # Get user input for output method
            echo -e "\nWhere would you like the compilation output of the script to be directed?:"
            echo -e "Option 1: terminal"
            echo -e "Option 2: file\n"
            read -p "Select Option: " output_method

            # Determine the output method based on user choice (0: terminal, 1: file)
            case $output_method in
                1)
                    output_method="0"
                    break
                    ;;
                2)
                    output_method="1"
                    break
                    ;;
                *) 
                    echo -e "Invalid input. Please enter either 1 or 2\n"
                    ;;

            esac
    
    done

}

#---------------------------------------------------------------------------------------------------
function environment_setup() {
    # Function to setup the required environment for the automated compilation of the algorithms

    # Read in all algs list file to create the sig_algs array
    while IFS= read -r line; do
        sig_algs+=("$line")
    done < "$algs_list_dir/sig_algs.txt"

    # Check if the base required directories exist and create them if they do not
    required_dirs=($bin_dir $results_dir)

    for dir in "${required_dirs[@]}"; do

        if [ ! -d $dir ]; then
            mkdir -p $dir
        else
            rm -rf $dir && mkdir -p $dir
        fi

    done

    # Check if the required bin directories exist for the various signature algorithms
    for sig_alg in "${sig_algs[@]}"; do

        if [ ! -d $bin_dir/$sig_alg ]; then
            mkdir -p $bin_dir/$sig_alg
        else
            rm -rf $bin_dir/$sig_alg && mkdir -p $bin_dir/$sig_alg
        fi

    done

    # Declare arrays for the required packages
    packages=(
        "build-essential" 
        "cmake" 
        "wget" 
        "gcc" 
        "g++" 
        "libssl-dev" 
        "libgmp-dev" 
        "libmpfr-dev" 
        "libgf2x-dev" 
        "libgf2x3" 
        "libm4ri-0.0.20200125" 
        "libm4ri-dev"
        "libntl-dev"
        "libntl44"
        "patchelf"
    )
    not_installed=()

    # Check if the required packages are installed
    for package in "${packages[@]}"; do

        if ! dpkg -s "$package" >/dev/null 2>&1; then
            not_installed+=("$package")
        fi

    done

    # Install any missing dependency packages
    if [[ ${#not_installed[@]} -ne 0 ]]; then
        sudo apt-get update && sudo apt upgrade -y
        sudo apt-get install -y "${not_installed[@]}"
    fi

    # Call the array utility script to get the algorithm variation arrays
    array_util_call
    
}

#---------------------------------------------------------------------------------------------------
function dme_sign_setup() {
    # Function for performing the setup of the DME_SIGN algorithm

    #__________________________________________________________________________
    # Set the source and destination directories for the DME_SIGN algorithm
    dme_sign_src_dir=$nist_src_dir/DME_Sign
    dme_sign_dst_dir=$bin_dir/DME_Sign

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${dme_sign_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$dme_sign_src_dir/$variation/Reference_Implementation"
        cd $variation_dir

        # Only compile the first two variations as the third does not have complete reference code (will be reviewed in future)
        if [ $variation != "dme-3rnds-8vars-64bits-sign" ]; then

            # Copy over modified files to the current variation directory
            "$scripts_dir/copy_modified_src_files.sh" "copy" "DME_Sign" "$variation_dir" "$variation" "$root_dir"

            # Compile and move pqcsign binary to relevant bin directory
            make clean >> /dev/null
            make -j $(nproc)
            mv "$variation_dir/pqcsign" "$dme_sign_dst_dir/pqcsign_$variation"
            make clean >> /dev/null

            # Restore the original source code files
            "$scripts_dir/copy_modified_src_files.sh" "restore" "DME_Sign" "$variation_dir" "$variation" "$root_dir"

        else
            echo -e "\nSkipping DME_Sign variation: $variation due to lack of reference code implementation, will be reviewed in future"
        fi

    done

}

#---------------------------------------------------------------------------------------------------
function cycles_test() {
    # Function to run the signature speed tests for each algorithm variation for current run
    # The functionality that would go in the sig_speed_test.sh would go here

    # DEV NOTE - In the dev_setup script, output the results to the TERMINAL for testing purposes
    # You can safely comment out any algorithm you are not working on to reduce the output to the terminal

    # Set the output filename based on current run
    local output_file="sig_speed_results_run_$1.txt"

    # # Redirect all output and errors to the output file
    # exec &>> $results_dir/$output_file

    # DME_Sign variation testing
    for variation in "${dme_sign_variations[@]}"; do

        # Only test the first two variations as the third does not have complete reference code (will be reviewed in future)
        if [ $variation != "dme-3rnds-8vars-64bits-sign" ]; then
            echo -e "\nRunning DME_Sign test for $variation"
            $bin_dir/DME_Sign/pqcsign_$variation
        else
            echo -e "\nSkipping DME_Sign test for $variation, due to lack of reference implementation code, will be reviewed in future"
        fi

    done

}

#---------------------------------------------------------------------------------------------------
function variations_setup() {
    # Function to setup the various signature algorithms and their variations
    # The setup functionality that is in the setup.sh script would go here

    # Set the modified files directory path
    mod_file_dir="$root_dir/src/modified_nist_src_files/linux"

    # NOTE - To improve development, in this script the setups have been placed in separate functions
    # so that they can be tested individually. You can comment out the algorithms which you are not currently working on.
    # When integrating into the main setup script, please follow the current structure of the setup.sh script.

    # Call setup functions
    dme_sign_setup

}   

#---------------------------------------------------------------------------------------------------
function main() {
    # Main function for setting up the required environment and compiling the various signature algorithms

    # Outputting to the user what version of setup this is
    echo -e "\n#############################################"
    echo -e "Development Automated Setup Script\n"

    # Remove error log from any previous runs
    if [ -f "last_setup_error.log" ]; then
        rm last_setup_error.log
    fi

    # Determine the setup options
    determine_setup_options

    # # Echo separator to specified output location
    if [ "$output_method" == "0" ]; then
        echo -e "\n#############################################"
        echo -e "Performing Environment Setup"
    else
        echo -e "\n#############################################" > "dev_setup_output.txt"
        echo -e "Performing Environment Setup" >> dev_setup_output.txt
    fi

    # Perform setup based on output method
    if [ "$output_method" == "1" ]; then

        # Configure setup environment and setup the variations
        environment_setup >> "dev_setup_output.txt" 2>&1
        variations_setup >> "dev_setup_output.txt" 2>&1
        
    else

        # Configure setup environment and setup the variations without redirection
        environment_setup
        variations_setup

    fi

    # Output to the user that setup is complete
    echo -e "\nSetup complete, testing scripts can be found in the test_scripts directory"

    # If the error log file is present, inform user of potential issues
    if [ -f "last_setup_error.log" ]; then
        echo -e "\nIssues detected during setup, please check the last_setup_error.log file for any critical issues"
    fi

    # Perform benchmarking if user indicated it should be performed
    if [ "$benchmark_included" == "1" ]; then

        # Echo separator to specified output location
        echo -e "\n#############################################"
        echo -e "Performing Benchmarking"

        # Perform benchmarking for number of specified runs
        for run_num in $(seq 1 $num_runs); do
            echo "Performing Run $run_num"
            cycles_test "$run_num"
        done

        # Output results location
        echo -e "\nBenchmarking runs completed, results can be found in the test_data/results directory"

    else
        echo -e "\nSkipping benchmarking"

    fi

}
main