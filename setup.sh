#!/bin/bash
# This is the main setup script for evaluating the round-1 signatures in a linux testing environment
# The script sets up the required environment for the automated compilation of the algorithms and compiles the relevant testing binaries.
# It calls relevant utility scripts to read in variation arrays for the different algorithms and 
# copy over modified source files to the relevant directories for compilation


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
    packages=("build-essential" "cmake" "wget" "gcc" "g++" "libssl-dev" "libgmp-dev" "libmpfr-dev")
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
function set_build_cross_flags() {
    # Function for setting the compile flag for the CROSS algorithm variations based on passed variation

    # Extract flags from variation name
    algorithm_flag=$(echo $variation | cut -d'-' -f2)
    security_level_string=$(echo $variation | cut -d'-' -f3)
    optimisation_string=$(echo $variation | cut -d'-' -f4)

    # Set the security level flag based on the size value in variation string
    if [[ "$security_level_string" == "128" ]]; then
        security_level_flag="CATEGORY_1"

    elif [[ "$security_level_string" == "192" ]]; then
        security_level_flag="CATEGORY_3"

    elif [[ "$security_level_string" == "256" ]]; then
        security_level_flag="CATEGORY_5"
    fi

    # Set the optimisation flag based on the value in the variation string
    if [[ "$optimisation_string" == "f" ]]; then
        optimisation_flag="SPEED"

    elif [[ "$optimisation_string" == "s" ]]; then
        optimisation_flag="SIG_SIZE"
    fi

}

#---------------------------------------------------------------------------------------------------
function variations_setup() {
    # Function to setup the various signature algorithms and their variations

    # Set the modified files directory path
    mod_file_dir="$root_dir/src/modified_nist_src_files/linux"

    #__________________________________________________________________________
    # Set the source and destination directories for the Raccoon algorithm
    raccoon_src_dir=$nist_src_dir/Raccoon/Reference_Implementation
    raccoon_dst_dir=$bin_dir/Raccoon
    
    # Loop through the different variations and compile the pqcsign binary
    for variation in "${raccoon_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$raccoon_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "Raccoon" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make -j $(nproc)
        variation_lower="${variation,,}"
        cp $variation_dir/pqcsign $raccoon_dst_dir/pqcsign_$variation_lower
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "Raccoon" "$variation_dir" "$variation" "$root_dir"
    
    done

    #__________________________________________________________________________
    # Set the source and destination directories for the Biscuit algorithm
    biscuit_src_dir=$nist_src_dir/Biscuit/Reference_Implementation
    biscuit_dst_dir=$bin_dir/Biscuit

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${biscuit_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$biscuit_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "Biscuit" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir/pqcsign" "$biscuit_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "Biscuit" "$variation_dir" "$variation" "$root_dir"

    done

    #__________________________________________________________________________
    # Set the source and destination directories for the CROSS algorithm
    cross_src_dir=$nist_src_dir/CROSS/Reference_Implementation
    cross_dst_dir=$bin_dir/CROSS

    # Change to the CROSS source code directory
    cd $cross_src_dir

    # Copy over modified files to the current variation directory
    "$scripts_dir/copy_modified_src_files.sh" "copy" "CROSS" "$cross_src_dir" "all" "$root_dir"

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${cross_variations[@]}"; do

        # Set the build flags for the current variation
        set_build_cross_flags

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make all CFLAGS="-Iinclude -std=c11 -g -Wall -D$algorithm_flag \
            -D$security_level_flag -D$optimisation_flag -DAES_CTR_CSPRNG -DSHA3_HASH" -j $(nproc)
            
        mv "$cross_src_dir/pqcsign" "$cross_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    done

    # Restore the original files
    "$scripts_dir/copy_modified_src_files.sh" "restore" "CROSS" "$cross_src_dir" "all" "$root_dir"

    #__________________________________________________________________________
    # Set the source and destination directories for the FAEST algorithm
    faest_src_dir=$nist_src_dir/FAEST/Reference_Implementation
    faest_dst_dir=$bin_dir/FAEST

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${faest_variations[@]}"; do
    
        # Set the variation directory path and change to it
        variation_dir="$faest_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "FAEST" "$variation_dir" "$variation" "$root_dir"
    
        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir/pqcsign" "$faest_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "FAEST" "$variation_dir" "$variation" "$root_dir"

    done

    #__________________________________________________________________________
    # Set the source and destination directories for the FuLeeca algorithm
    fuleeca_src_dir=$nist_src_dir/FuLeeca/Reference_Implementation
    fuleeca_dst_dir=$bin_dir/FuLeeca

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${fuleeca_variations[@]}"; do
    
        # Set the variation directory path and change to it
        variation_dir="$fuleeca_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "FuLeeca" "$variation_dir" "$variation" "$root_dir"
        
        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir/pqcsign" "$fuleeca_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "FuLeeca" "$variation_dir" "$variation" "$root_dir"
    
    done

    #__________________________________________________________________________
    # Setting up variations of the Enhanced_pqsigRM algorithm
    pqsigrm_src_dir=$nist_src_dir/Enhanced_pqsigRM/Reference_Implementation
    pqsigrm_dst_dir=$bin_dir/Enhanced_pqsigRM

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${pqsigrm_variations[@]}"; do
    
        # Set the variation directory path and change to it
        variation_dir="$pqsigrm_src_dir/$variation"    
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "Enhanced_pqsigRM" "$variation_dir" "$variation" "$root_dir"
        
        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir/pqcsign" "$pqsigrm_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "Enhanced_pqsigRM" "$variation_dir" "$variation" "$root_dir"
    
    done

    #__________________________________________________________________________
    # Set the source and destination directories for the SPHINCS_alpha algorithm
    sphincs_alpha_src_dir=$nist_src_dir/SPHINCS_alpha/Reference_Implementation
    sphincs_alpha_dst_dir=$bin_dir/SPHINCS_alpha

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${sphincs_alpha_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$sphincs_alpha_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "SPHINCS_alpha" "$variation_dir" "$variation" "$root_dir"
        
        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir/pqcsign" "$sphincs_alpha_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "SPHINCS_alpha" "$variation_dir" "$variation" "$root_dir"
    
    done

    #__________________________________________________________________________
    # Set the source and destination directories for the SQIsignsign algorithm
    sqi_src_dir=$nist_src_dir/SQIsign/Reference_Implementation
    sqi_dst_dir=$bin_dir/SQIsign
    sqi_build_dir=$nist_src_dir/SQIsign/Reference_Implementation/build
    sqi_apps_dir=$sqi_build_dir/apps

    # Change to source directory and ensure that the build directory is present and empty
    cd $sqi_src_dir

    if [ ! -d build ]; then
        mkdir build
    else
        rm -rf build && mkdir build
    fi

    # Copy over modified files to the relevant source code directories
    "$scripts_dir/copy_modified_src_files.sh" "copy" "SQIsign" "$sqi_src_dir" "all" "$root_dir"

    # Compile and move pqcsign binary to relevant bin directory
    cd $sqi_build_dir
    cmake -DSQISIGN_BUILD_TYPE=ref -DCMAKE_BUILD_TYPE=Release ../ 
    make -j $(nproc)
    mv $sqi_apps_dir/pqcsign_* "$sqi_dst_dir/"
    make clean >> /dev/null && cd $sqi_src_dir && rm -rf build

    # Restore the original source code files
    "$scripts_dir/copy_modified_src_files.sh" "restore" "SQIsign" "$sqi_src_dir" "all" "$root_dir"

    #__________________________________________________________________________
    # Set the source and destination directories for the UOV algorithm
    uov_src_dir=$nist_src_dir/UOV/Reference_Implementation
    uov_dst_dir=$bin_dir/UOV

    # Change to the UOV source code directory
    cd $uov_src_dir

    # Copy over modified files to the current variation directory
    "$scripts_dir/copy_modified_src_files.sh" "copy" "UOV" "$uov_src_dir" "all" "$root_dir"

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${uov_variations[@]}"; do

        # Extract the variation type from the variation name
        variation_type=$(echo "$variation" | cut -d'_' -f2-)
        variation_dir="$uov_src_dir/$variation_type"

        # Compile and move pqcsign binary to lib directory
        make clean >> /dev/null
        make "PROJ=$variation_type" -j $(nproc)
        mv "$uov_src_dir/pqcsign" "$uov_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    done

    # Restore the original files (makefile will only be resorted if is it the last variation)
    "$scripts_dir/copy_modified_src_files.sh" "restore" "UOV" "$uov_src_dir" "all" "$root_dir"

    #__________________________________________________________________________
    # Set the source and destination directories for the MEDS algorithm
    med_src_dir=$nist_src_dir/MEDS/Reference_Implementation
    med_dst_dir=$bin_dir/MEDS

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${med_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$med_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "MEDS" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make
        mv "$variation_dir/pqcsign" "$med_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "MEDS" "$variation_dir" "$variation" "$root_dir"

    done

    #__________________________________________________________________________
    # Set the source and destination directories for the HAWK algorithm
    hawk_src_dir=$nist_src_dir/HAWK/Reference_Implementation
    hawk_dst_dir=$bin_dir/HAWK

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${hawk_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$hawk_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "HAWK" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir/pqcsign" "$hawk_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "HAWK" "$variation_dir" "$variation" "$root_dir"

    done

    #__________________________________________________________________________
    # Set the source and destination directories for the EHTv3v4 algorithm
    ehtv3v4_src_dir=$nist_src_dir/EHTv3v4/Reference_Implementation
    eht3v4_dst_dir=$bin_dir/EHTv3v4

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${ehtv3v4_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$ehtv3v4_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "EHTv3v4" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir/pqcsign" "$eht3v4_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "EHTv3v4" "$variation_dir" "$variation" "$root_dir"

    done

    #__________________________________________________________________________
    # Set the source and destination directories for the HuFu algorithm
    hufu_src_dir=$nist_src_dir/HuFu/Reference_Implementation
    hufu_dst_dir=$bin_dir/HuFu

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${hufu_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$hufu_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "HuFu" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir/pqcsign" "$hufu_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "HuFu" "$variation_dir" "$variation" "$root_dir"

    done

    #__________________________________________________________________________
    # Set the source and destination directories for the 3WISE algorithm
    three_wise_src_dir=$nist_src_dir/3WISE/Reference_Implementation
    three_wise_dst_dir=$bin_dir/3WISE
    three_wise_flint_path=$three_wise_src_dir/flint

    # Change to the 3WISE source code directory
    cd $three_wise_src_dir

    # Ensure there is no previous build of flint library dependency
    if [ -d "$three_wise_flint_path" ]; then
        rm -rf "$three_wise_flint_path"
        rm -rf v2.9.0.tar.* && rm -rf flint-2.9.0
    fi
    mkdir $three_wise_flint_path

    # Setting up flint library dependency
    wget https://github.com/flintlib/flint2/archive/refs/tags/v2.9.0.tar.gz
    tar -xf v2.9.0.tar.gz && cd flint-2.9.0
    ./configure --prefix=$three_wise_flint_path
    make -j $(nproc) && make install
    rm -rf v2.9.0.tar.gz && rm -rf flint-2.9.0

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${three_wise_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$three_wise_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "3WISE" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir/pqcsign" "$three_wise_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "3WISE" "$variation_dir" "$variation" "$root_dir"

    done

    #__________________________________________________________________________
    # Set the source and destination directories for the MIRA algorithm
    mira_src_dir=$nist_src_dir/MIRA/Reference_Implementation
    mira_dst_dir=$bin_dir/MIRA

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${mira_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$mira_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "MIRA" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make all -j $(nproc)
        mv "$variation_dir/bin/pqcsign" "$mira_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "MIRA" "$variation_dir" "$variation" "$root_dir"

    done

    #__________________________________________________________________________
    # Set the source and destination directories for the PERK algorithm
    perk_src_dir=$nist_src_dir/PERK/Reference_Implementation
    perk_dst_dir=$bin_dir/PERK

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${perk_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$perk_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "PERK" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make all -j $(nproc)
        mv "$variation_dir/pqcsign" "$perk_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "PERK" "$variation_dir" "$variation" "$root_dir"

    done

    #__________________________________________________________________________
    # Set the source and destination directories for the RYDE algorithm
    ryde_src_dir=$nist_src_dir/RYDE/Reference_Implementation
    ryde_dst_dir=$bin_dir/RYDE

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${ryde_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$ryde_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "RYDE" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make all -j $(nproc)
        mv "$variation_dir/bin/pqcsign" "$ryde_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "RYDE" "$variation_dir" "$variation" "$root_dir"

    done

    #__________________________________________________________________________
    # Set the source and destination directories for the SDitH algorithm
    sdith_src_dir=$nist_src_dir/SDitH/Reference_Implementation
    sdith_hybercube_src_dir="$sdith_src_dir/Hypercube_Variant"
    sdith_threshold_src_dir="$sdith_src_dir/Threshold_Variant"
    sdith_dst_dir=$bin_dir/SDitH

    # Setting up the Hypercube variants

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${sdith_hypercube_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$sdith_hybercube_src_dir/$variation"
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

    #__________________________________________________________________________
    # Set the source and destination directories for the Ascon_sign algorithm
    ascon_sign_src_dir=$nist_src_dir/Ascon_Sign/Reference_Implementation
    ascon_sign_dst_dir=$bin_dir/Ascon_Sign


    # Loop through the different variations and compile the pqcsign binary
    for variation in "${ascon_sign_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$ascon_sign_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "Ascon_Sign" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make all -j $(nproc)
        mv "$variation_dir/pqcsign" "$ascon_sign_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "Ascon_sign" "$variation_dir" "$variation" "$root_dir"

    done


    #__________________________________________________________________________
    # Set the source and destination directories for the MAYO algorithm 
    mayo_src_dir=$nist_src_dir/MAYO/Reference_Implementation
    mayo_dst_dir=$bin_dir/MAYO
    mayo_build_dir=$mayo_src_dir/build
    mayo_apps_dir=$mayo_build_dir/apps

    # Change to source directory and ensure that the build directory is present and empty
    cd $mayo_src_dir

    if [ ! -d build ]; then
        mkdir build
    else
        rm -rf build && mkdir build
    fi

    # Copy over modified files to the relevant source code directories
    "$scripts_dir/copy_modified_src_files.sh" "copy" "MAYO" "$mayo_src_dir" "all" "$root_dir"

    # Compile and move pqcsign binary to relevant bin directory
    cd $mayo_build_dir
    cmake -DMAYO_BUILD_TYPE=ref -DENABLE_AESNI=OFF ../ # can change to ENABLE_AESNI=ON if needed (refer to MAYO readme)
    make 
    mv $mayo_apps_dir/pqcsign_* "$mayo_dst_dir/"
    make clean >> /dev/null && cd $mayo_src_dir && rm -rf build

    # Restore the original source code files
    "$scripts_dir/copy_modified_src_files.sh" "restore" "MAYO" "$mayo_src_dir" "all" "$root_dir"

    #__________________________________________________________________________
    # Set the source and destination directories for the eMLE_Sig_2.0 algorithm
    emle_sig_2_0_src_dir=$nist_src_dir/eMLE_Sig_2.0/Reference_Implementation
    emle_sig_2_0_dst_dir=$bin_dir/eMLE_Sig_2.0

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${emle_sig_2_0_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$emle_sig_2_0_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "eMLE_Sig_2.0" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make all
        mv "$variation_dir/pqcsign" "$emle_sig_2_0_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "eMLE_Sig_2.0" "$variation_dir" "$variation" "$root_dir"

    done

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

    #__________________________________________________________________________
    # Set the source and destination directories for the Xifrat1_Sign algorithm
    xifrat1_sign_src_dir=$nist_src_dir/Xifrat1_Sign_I/Reference_Implementation
    xifrat1_sign_dst_dir=$bin_dir/Xifrat1_Sign_I

    # Change to the Xifrat1_Sign source code directory
    cd $xifrat1_sign_src_dir

    # Copy over modified files to the current variation directory
    "$scripts_dir/copy_modified_src_files.sh" "copy" "Xifrat1_Sign_I" "$xifrat1_sign_src_dir" "Xifrat1_Sign_I" "$root_dir"

    # Compile and move pqcsign binary to relevant bin directory
    make clean >> /dev/null
    make 
    mv "$xifrat1_sign_src_dir/pqcsign" "$xifrat1_sign_dst_dir/pqcsign_Xifrat1_Sign_I"
    make clean >> /dev/null

    # Restore the original source code files
    "$scripts_dir/copy_modified_src_files.sh" "restore" "Xifrat1_Sign_I" "$xifrat1_sign_src_dir" "Xifrat1_Sign_I" "$root_dir"


}

#---------------------------------------------------------------------------------------------------
function main() {
    # Main function for setting up the required environment and compiling the various signature algorithms

    # Remove error log from any previous runs
    if [ -f "last_setup_error.log" ]; then
        rm last_setup_error.log
    fi

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

}
main