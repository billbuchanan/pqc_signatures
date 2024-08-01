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
    IFS=',' read -r -a kaz_sign_variations <<< "$KAZ_SIGN_VARIATIONS"
    IFS=',' read -r -a mirith_variations <<< "$MIRITH_VARIATIONS"
    IFS=',' read -r -a mqom_variations <<< "$MQOM_VARIATIONS"
    IFS=',' read -r -a preon_variations <<< "$PREON_VARIATIONS"
    IFS=',' read -r -a sdith_threshold_variations <<< "$SDITH_THRESHOLD_VARIATIONS"
    IFS=',' read -r -a squirrels_variations <<< "$SQUIRRELS_VARIATIONS"
    IFS=',' read -r -a wave_variations <<< "$WAVE_VARIATIONS"

    
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
function cycles_test() {
    # Function to run the signature speed tests for each algorithm variation for current run
    # The functionality that would go in the sig_speed_test.sh would go here

    # DEV NOTE - In the dev_setup script, output the results to the TERMINAL for testing purposes
    # You can safely comment out any algorithm you are not working on to reduce the output to the terminal

    # Set the output filename based on current run
    local output_file="sig_speed_results_run_$1.txt"

    # Run testing for KAZ_SIGN algorithm
    for variation in "${kaz_sign_variations[@]}"; do
        echo -e "\nRunning KAZ_SIGN test for $variation"
        $bin_dir/KAZ_SIGN/pqcsign_$variation
    done

    # Run testing for MiRitH algorithm
    for variation in "${mirith_variations[@]}"; do
        echo -e "\nRunning MiRitH test for $variation"
        $bin_dir/MiRitH/pqcsign_$variation
    done

    # Run testing for MQOM algorithm
    for variation in "${mqom_variations[@]}"; do
        echo -e "\nRunning MQOM test for $variation"
        $bin_dir/MQOM/pqcsign_$variation
    done

    # Run testing for Preon algorithm
    for variation in "${preon_variations[@]}"; do
        echo -e "\nRunning Preon test for $variation"
        $bin_dir/Preon/pqcsign_$variation
    done

    # Run testing for SDitH Threshold algorithm
    for variation in "${sdith_threshold_variations[@]}"; do
        echo -e "\nRunning SDitH Threshold test for $variation"
        $bin_dir/SDitH/pqcsign_$variation
    done

    # Run testing for SQUIRRELS algorithm
    for variation in "${squirrels_variations[@]}"; do
        echo -e "\nRunning SQUIRRELS test for $variation"
        $bin_dir/SQUIRRELS/pqcsign_$variation
    done

    # Run testing for Wave algorithm
    for variation in "${wave_variations[@]}"; do
        echo -e "\nRunning Wave test for $variation"
        $bin_dir/Wave/pqcsign_$variation
    done

}

#---------------------------------------------------------------------------------------------------
function kaz_sign_setup() {
    # Function for performing the setup of the KAZ_SIGN algorithm

    #__________________________________________________________________________
    # Set the source and destination directories for the KAZ_SIGN algorithm
    kaz_src_dir="$nist_src_dir/KAZ_SIGN/Reference_Implementation"
    kaz_dst_dir="$bin_dir/KAZ_SIGN"

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${kaz_sign_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$kaz_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "KAZ_SIGN" "$variation_dir" "$variation" "$root_dir"

        # Compile the pqcsign binary for the current variation
        make clean >> /dev/null
        make 
        mv "$variation_dir/pqcsign" "$kaz_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "KAZ_SIGN" "$variation_dir" "$variation" "$root_dir"

    done

}

#---------------------------------------------------------------------------------------------------
function mirith_setup() {
    # Function for performing the setup of the MiRitH algorithm

    #__________________________________________________________________________
    # Set the source and destination directories for the MiRitH algorithm
    mirith_src_dir="$nist_src_dir/MiRitH/Reference_Implementation"
    mirith_dst_dir="$bin_dir/MiRitH"

    # NOTE - MiRitH uses the same structures for its makefiles across all variations, so only one copy has been made in 
    # the modified_nist_src_files directory and it copied across to all variations. This will make fixing errors easier as
    # as there would be a total of 72 copies of the makefile to fix if they were all separate. There is the main makefile and 
    # the nist makefile that need to copied over to the variations.

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${mirith_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$mirith_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "MiRitH" "$variation_dir" "$variation" "$root_dir"

        # Compile the pqcsign binary for the current variation
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir/nist/pqcsign" "$mirith_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "MiRitH" "$variation_dir" "$variation" "$root_dir"

    done

}

#---------------------------------------------------------------------------------------------------
function mqom_setup() {
    # Function for performing the setup of the MQOM algorithm

    #__________________________________________________________________________
    # Set the source and destination directories for the MQOM algorithm
    mqom_src_dir="$nist_src_dir/MQOM/Reference_Implementation"
    mqom_dst_dir="$bin_dir/MQOM"

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${mqom_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$mqom_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "MQOM" "$variation_dir" "$variation" "$root_dir"

        # Compile the pqcsign binary for the current variation
        make clean >> /dev/null
        make
        mv "$variation_dir/pqcsign" "$mqom_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "MQOM" "$variation_dir" "$variation" "$root_dir"

    done

}

#---------------------------------------------------------------------------------------------------
function preon_setup() {
    # Function for performing the setup of the PREON algorithm

    #__________________________________________________________________________
    # Set the source and destination directories for the PREON algorithm
    preon_src_dir="$nist_src_dir/Preon/Reference_Implementation"
    preon_dst_dir="$bin_dir/Preon"

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${preon_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$preon_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "Preon" "$variation_dir" "$variation" "$root_dir"

        # Compile the pqcsign binary for the current variation
        make clean >> /dev/null
        make  -j $(nproc)
        mv "$variation_dir/pqcsign" "$preon_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "Preon" "$variation_dir" "$variation" "$root_dir"

    done

}

#---------------------------------------------------------------------------------------------------
function sdith_threshold_setup() {
    # Function  for performing the setup of the SDITH algorithm

    # NOTE - When adding back to main setup script, only copy over the second loop for the threshold variants

    #__________________________________________________________________________
    # Set the source and destination directories for the SDitH algorithm
    sdith_src_dir=$nist_src_dir/SDitH/Reference_Implementation
    sdith_hybercube_src_dir="$sdith_src_dir/Hypercube_Variant"
    sdith_threshold_src_dir="$sdith_src_dir/Threshold_Variant"
    sdith_dst_dir=$bin_dir/SDitH

    # Setting up the Hypercube variants

    # # Loop through the different variations and compile the pqcsign binary
    # for variation in "${sdith_hypercube_variations[@]}"; do

    #     # Set the variation directory path and change to it
    #     variation_dir="$sdith_hybercube_src_dir/$variation"
    #     cd $variation_dir

    #     # Copy over modified files to the current variation directory
    #     "$scripts_dir/copy_modified_src_files.sh" "copy" "SDitH" "$variation_dir" "$variation" "$root_dir"

    #     # Compile and move pqcsign binary to relevant bin directory
    #     make clean >> /dev/null
    #     make
    #     mv "$variation_dir/pqcsign" "$sdith_dst_dir/pqcsign_$variation"
    #     make clean >> /dev/null

    #     # Restore the original source code files
    #     "$scripts_dir/copy_modified_src_files.sh" "restore" "SDitH" "$variation_dir" "$variation" "$root_dir"

    # done

    # Setting up the Threshold variants

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${sdith_threshold_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$sdith_threshold_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "SDitH" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make
        mv "$variation_dir/pqcsign" "$sdith_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "SDitH" "$variation_dir" "$variation" "$root_dir"

    done
}

#---------------------------------------------------------------------------------------------------
function squirrels_variations() {
    # Function for performing the setup of the SQUIRRELS algorithm

    #__________________________________________________________________________
    # Set the source and destination directories for the SQUIRRELS algorithm
    squirrels_src_dir="$nist_src_dir/SQUIRRELS/Reference_Implementation"
    squirrels_lib_dir="$nist_src_dir/SQUIRRELS/lib"
    orgin_lib_dir_backup="$root_dir/main_default_src_backup/SQUIRRELS"
    squirrels_dst_dir="$bin_dir/SQUIRRELS"

    # Check if original lib directory is present and make a copy of it
    if [ ! -f "$squirrels_lib_dir/modded-lib.flag" ]; then

        # Remove the current lib backup directory if it exists as the one in squirrels src is the default
        if [ -d "$orgin_lib_dir_backup/lib" ]; then
            rm -rf "$orgin_lib_dir_backup/lib"
            cp -r "$squirrels_lib_dir" "$orgin_lib_dir_backup/lib"
            touch "$squirrels_lib_dir/modded-lib.flag"

        else
            cp -r "$squirrels_lib_dir" "$orgin_lib_dir_backup/lib"
            touch "$squirrels_lib_dir/modded-lib.flag"

        fi
    
    else

        # Check if original lib directory backup is present and restore from that
        if [ -d "$orgin_lib_dir_backup/lib" ]; then
            rm -rf "$squirrels_lib_dir"
            cp -r "$orgin_lib_dir_backup/lib" "$squirrels_lib_dir"
            touch "$squirrels_lib_dir/modded-lib.flag"
        else
           echo -e "\nThe modded SQUIRRELS Lib directory could not be restored to default" >> "$root_dir/last_setup_error.log"
           echo -e "please manually restore the src/nist/SQUIRRELS/lib directory to avoid committing the built lib files\n" >> "$root_dir/last_setup_error.log"
        fi
    
    fi

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${squirrels_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$squirrels_src_dir/$variation"
        cd $variation_dir

        # Ensure that there is no build directory present from previous runs
        if [ -d "build" ]; then
            rm -rf build
        fi

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "SQUIRRELS" "$variation_dir" "$variation" "$root_dir"

        # Compile the pqcsign binary for the current variation
        make clean >> /dev/null
        make
        mv "$variation_dir/build/pqcsign" "$squirrels_dst_dir/pqcsign_$variation"

        # Copy over the required shared libraries to the bin directory only once
        if [ ! -d "$squirrels_dst_dir/lib" ]; then
            mkdir -p "$squirrels_dst_dir/lib"
            cp -R "$squirrels_lib_dir/build/mpfr/lib" "$squirrels_dst_dir/lib/mpfr"
            cp -R "$squirrels_lib_dir/build/gmp/lib" "$squirrels_dst_dir/lib/gmp"
            cp -R "$squirrels_lib_dir/build/flint/lib" "$squirrels_dst_dir/lib/flint"
            cp -R "$squirrels_lib_dir/build/fplll/lib" "$squirrels_dst_dir/lib/fplll"
        fi

        # Set rpath to include all relevant subdirectories in ./lib
        patchelf --set-rpath '$ORIGIN/lib/mpfr:$ORIGIN/lib/gmp:$ORIGIN/lib/flint:$ORIGIN/lib/fplll' "$squirrels_dst_dir/pqcsign_$variation"

        # Clean up and remove build directory
        make clean >> /dev/null
        rm -rf "$variation_dir/build"

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "SQUIRRELS" "$variation_dir" "$variation" "$root_dir"

    done

    # Restore the original lib directory
    if [ -d "$orgin_lib_dir_backup" ]; then
        rm -rf "$squirrels_lib_dir"
        cp -r "$orgin_lib_dir_backup/lib" "$squirrels_lib_dir"
    else
        echo -e "\nThe modded SQUIRRELS Lib directory could not be restored to default" >> "$root_dir/last_setup_error.log"
        echo -e "please manually restore the src/nist/SQUIRRELS/lib directory to avoid committing the built lib files\n" >> "$root_dir/last_setup_error.log"
    fi

}

#---------------------------------------------------------------------------------------------------
function wave_setup() {
    # Function for performing the setup of the Wave algorithm

    #__________________________________________________________________________
    # Set the source and destination directories for the Wave algorithm
    wave_src_dir="$nist_src_dir/Wave/Reference_Implementation"
    wave_dst_dir="$bin_dir/Wave"

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${wave_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$wave_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "Wave" "$variation_dir" "$variation" "$root_dir"

        # Compile the pqcsign binary for the current variation
        make clean >> /dev/null
        make
        mv "$variation_dir/pqcsign" "$wave_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "Wave" "$variation_dir" "$variation" "$root_dir"

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
    kaz_sign_setup
    mirith_setup
    mqom_setup
    preon_setup
    sdith_threshold_setup
    squirrels_variations
    wave_setup

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