#!/bin/bash
# This is the temporary development setup script for testing the integration of the algorithms within main automated compiling script.
# Its main purpose is to implement and store the output of the compilation process for the algorithms still to be implemented to easily 
# test the setup process and capture any errors outputted. It also contains a function for testing the pqcsign binary of the algorithms to 
# test if the outputted binary is also functioning correctly. Once all algorithms have been successfully implemented within the project, this
# script will be removed from the repository.

# NOTICE - If you are not developing this project, you can safely ignore this script and can instead use the main setup
# and benchmarking scripts to test the algorithms which are currently supported. A list of currently supported algorithms can be found
# on the repositories development branch README (https://github.com/billbuchanan/pqc_signatures/tree/developing)


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
    IFS=',' read -r -a raccoon_variations <<< "$RACCOON_VARIATIONS"
    IFS=',' read -r -a biscuit_variations <<< "$BISCUIT_VARIATIONS"
    IFS=',' read -r -a cross_variations <<< "$CROSS_VARIATIONS"
    IFS=',' read -r -a faest_variations <<< "$FAEST_VARIATIONS"
    IFS=',' read -r -a fuleeca_variations <<< "$FULEECA_VARIATIONS"
    IFS=',' read -r -a pqsigrm_variations <<< "$PQSIGRM_VARIATIONS"
    IFS=',' read -r -a sphincs_alpha_variations <<< "$SPHINCS_ALPHA_VARIATIONS"
    IFS=',' read -r -a sqi_variations <<< "$SQI_VARIATIONS"
    IFS=',' read -r -a uov_variations <<< "$UOV_VARIATIONS"
    IFS=',' read -r -a med_variations <<< "$MED_VARIATIONS"
    IFS=',' read -r -a hawk_variations <<< "$HAWK_VARIATIONS"
    IFS=',' read -r -a ehtv3v4_variations <<< "$EHTV3V4_VARIATIONS"
    IFS=',' read -r -a hufu_variations <<< "$HUFU_VARIATIONS"
    IFS=',' read -r -a three_wise_variations <<< "$THREE_WISE_VARIATIONS"
    IFS=',' read -r -a mira_variations <<< "$MIRA_VARIATIONS"
    IFS=',' read -r -a perk_variations <<< "$PERK_VARIATIONS"
    IFS=',' read -r -a ryde_variations <<< "$RYDE_VARIATIONS"
    IFS=',' read -r -a sdith_hypercube_variations <<< "$SDITH_HYPERCUBE_VARIATIONS"
    IFS=',' read -r -a ascon_sign_variations <<< "$ASCON_SIGN_VARIATIONS"
    IFS=',' read -r -a mayo_variations <<< "$MAYO_VARIATIONS"
    IFS=',' read -r -a emle_sig_2_0_variations <<< "$EMLE_SIG_2_0_VARIATIONS"
    IFS=',' read -r -a dme_sign_variations <<< "$DME_SIGN_VARIATIONS"
    IFS=',' read -r -a xifrat1_sign_variations <<< "$XIFRAT1_SIGN_VARIATIONS"
    IFS=',' read -r -a vox_variations <<< "$VOX_VARIATIONS"
    IFS=',' read -r -a tuov_variations <<< "$TUOV_VARIATIONS"
    IFS=',' read -r -a prov_variations <<< "$PROV_VARIATIONS"
    IFS=',' read -r -a qr_uov_variations <<< "$QR_UOV_VARIATIONS"
    IFS=',' read -r -a snova_variations <<< "$SNOVA_VARIATIONS"
    IFS=',' read -r -a hppc_variations <<< "$HPPC_VARIATIONS"
    IFS=',' read -r -a alteq_variations <<< "$ALTEQ_VARIATIONS"
    
    # Call the array utility script to clear environment variables
    source "$scripts_dir/variation_array_util.sh" "clear"

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
function determine_run_nums() {
    # Function to determine the number of runs to be performed set by the user

    # Loop until valid input is received
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

}



#---------------------------------------------------------------------------------------------------
function variations_setup() {
    # Function to setup the various signature algorithms and their variations

    # Set the modified files directory path
    mod_file_dir="$root_dir/src/modified_nist_src_files/linux"

    #__________________________________________________________________________
    # Set the source and destination directories for the Raccoon algorithm




}

#---------------------------------------------------------------------------------------------------
function cycles_test() {
    # Function to run the signature speed tests for each algorithm variation for current run

    # Set the output filename based on current run
    local output_file="sig_speed_results_run_$1.txt"


}

#---------------------------------------------------------------------------------------------------
function main() {
    # Main function for setting up the required environment and compiling the various signature algorithms

    # Remove error log from any previous runs
    if [ -f "last_setup_error.log" ]; then
        rm last_setup_error.log
    fi

    # Determine if user wants to do benchmark testing before performing setup
    while true; do

        # Get user input for number of runs
        echo -e "\n"
        read -p "Enter the number of runs to be performed: " num_runs
        benchmark_choice=$(echo $benchmark_choice | tr '[:upper:]' '[:lower:]')

        # Check if input is valid and set choice flag
        if [ $benchmark_choice == "y" ]; then
            benchmark_included=1
            break

        elif [ $benchmark_choice == "n" ]; then
            benchmark_included=0
            break
        
        else
            echo -e "\nIncorrect input, please use (y/n) to indicate benchmark inclusion choice"
        fi

    done


    # Configure setup environment
    echo "Performing Environment Setup"
    environment_setup

    # Setup the various algorithms and their variations
    variations_setup

    # Output to the user that setup is complete
    echo -e "\nSetup complete, testing scripts can be found in the test_scripts directory"

    # If the error log file is present, inform user of potential issues
    if [ -f "last_setup_error.log" ]; then
        echo -e "\nIssues detected during setup, please check the last_setup_error.log file for any critical issues"
    fi

    # Perform benchmarking if user indicated it should be performed
    if [ "$benchmark_choice" == "1" ]; then 

        # Determine the number of test runs
        determine_run_nums

        # Perform benchmarking for number of specified runs
        for run_num in $(seq 1 $number_of_runs); do
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