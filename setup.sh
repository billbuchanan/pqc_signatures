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

    # Change back to the 3WISE source code directory and clean up
    cd $three_wise_src_dir
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
    # Set the source and destination directories for the AIMer algorithm
    aimer_src_dir=$nist_src_dir/AIMer/Reference_Implementation
    aimer_dst_dir=$bin_dir/AIMer

    # Change to the AIMer source code directory
    cd $aimer_src_dir

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${aimer_variations[@]}"; do
    
        # Set the variation directory path and change to it
        variation_dir="$aimer_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "AIMer" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make
        mv "$variation_dir/pqcsign" "$aimer_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "AIMer" "$variation_dir" "$variation" "$root_dir"
    
    done

    #__________________________________________________________________________
    # Set the source and destination directories for the ALTEQ algorithm
    alteq_src_dir=$nist_src_dir/ALTEQ/Reference_Implementation
    alteq_dst_dir=$bin_dir/ALTEQ

    # Change to the ALTEQ source code directory
    cd $alteq_src_dir

    # Copy over modified files to the current variation directory
    "$scripts_dir/copy_modified_src_files.sh" "copy" "ALTEQ" "$alteq_src_dir" "ALTEQ" "$root_dir"

    # Compile and move pqcsign binary to relevant bin directory
    make distclean >> /dev/null
    make pqcsign_all -j $(nproc)
    mv $alteq_src_dir/pqcsign_* $alteq_dst_dir/
    make distclean >> /dev/null

    # Restore the original source code files
    "$scripts_dir/copy_modified_src_files.sh" "restore" "ALTEQ" "$alteq_src_dir" "ALTEQ" "$root_dir"

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
    # Set the source and destination directories for the EagleSign algorithm
    eaglesign_src_dir=$nist_src_dir/EagleSign/Reference_Implementation
    eaglesign_dst_dir=$bin_dir/EagleSign

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${eaglesign_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$eaglesign_src_dir/$variation"
        cd $variation_dir

        # Copy over aes256ctr source and headers files to the current variation directory
        cp $mod_file_dir/EagleSign/aes256ctr.* $variation_dir/

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "EagleSign" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir/pqcsign" "$eaglesign_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "EagleSign" "$variation_dir" "$variation" "$root_dir"

        # Remove the copied aes256ctr source and headers files
        rm -f $variation_dir/aes256ctr.*

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
    # Set the source and destination directories for the HAETAE algorithm
    haetae_src_dir=$nist_src_dir/HAETAE/Reference_Implementation
    haetae_dst_dir=$bin_dir/HAETAE
    haetae_dst_libs_dir=$bin_dir/HAETAE/libs

    # Change to the HAETAE source code directory
    cd $haetae_src_dir

    # Ensure build directory is not present from previous setup
    if [ -d "$haetae_src_dir/build" ]; then
        rm -rf "$haetae_src_dir/build"
    fi

    # Copy over modified files to the current variation directory
    "$scripts_dir/copy_modified_src_files.sh" "copy" "HAETAE" "$haetae_src_dir" "all" "$root_dir"

    # Compile and move pqcsign binaries and shared libraries to relevant bin directory
    cmake -S ./ -B build && cmake --build build --clean-first
    cd "$haetae_src_dir/build"
    make -j $(nproc)
    mv $haetae_src_dir/build/bin/pqcsign_* "$haetae_dst_dir/"

    # Create libs directory in the destination directory and move shared libraries
    if [ -d "$haetae_dst_libs_dir" ]; then
        rm -rf "$haetae_dst_libs_dir"
        mkdir -p "$haetae_dst_libs_dir"
    else
        mkdir -p "$haetae_dst_libs_dir"
    fi

    mv $haetae_src_dir/build/libs/* "$haetae_dst_libs_dir/"

    # Utilse patchelf to modify the paths used for shared libraries in the pqcsign binaries so that they can be moved to the bin directory
    for binary in "$haetae_dst_dir"/pqcsign_*; do
        patchelf --set-rpath '$ORIGIN/libs' "$binary"
    done

    for lib in "$haetae_dst_libs_dir"/*.so; do
        patchelf --set-rpath '$ORIGIN' "$lib"
    done

    # Restore the original source code files and remove build directory
    "$scripts_dir/copy_modified_src_files.sh" "restore" "HAETAE" "$haetae_src_dir" "all" "$root_dir"
    rm -rf "$haetae_src_dir/build"

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
    # Set the source and destination directories for the HPPC algorithm
    hppc_src_dir=$nist_src_dir/HPPC/Reference_Implementation
    hppc_dst_dir=$bin_dir/HPPC
    hppc_flint_path=$hppc_src_dir/flint

    # Change to the HPPC source code directory
    cd $hppc_src_dir

    # Ensure there is no previous build of flint library dependency
    if [ -d "$hppc_flint_path/flint" ]; then
        rm -rf "$hppc_flint_path/flint"
        rm -rf v2.9.0.tar.* && rm -rf flint-2.9.0
    fi
    mkdir $hppc_flint_path

    # Setting up flint library dependency
    wget https://github.com/flintlib/flint2/archive/refs/tags/v2.9.0.tar.gz
    tar -xf v2.9.0.tar.gz && cd flint-2.9.0
    ./configure --prefix=$hppc_flint_path
    make -j $(nproc) && make install

    # Change back to the HPPC source code directory and clean up
    cd $hppc_src_dir
    rm -rf v2.9.0.tar.gz && rm -rf flint-2.9.0

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${hppc_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$hppc_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "HPPC" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir/pqcsign" "$hppc_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "HPPC" "$variation_dir" "$variation" "$root_dir"

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

    #__________________________________________________________________________
    # Set the source and destination directories for the LESS algorithm
    # NOTE - LESS does not have a default CMakelists.txt file, so a variation of the one included in the additional implementation is used
    less_src_dir=$nist_src_dir/LESS/Reference_Implementation
    less_dst_dir=$bin_dir/LESS
    less_build_dir=$less_src_dir/build

    # Copy over modified files to the current variation directory
    "$scripts_dir/copy_modified_src_files.sh" "copy" "LESS" "$less_src_dir" "all" "$root_dir"

    # Ensure build directory is not present from previous setup
    if [ -d "$less_build_dir" ]; then
        rm -rf "$less_build_dir"
    fi

    # Compile and move pqcsign binary to relevant bin directory
    mkdir "$less_build_dir" && cd "$less_build_dir"
    cmake ../ && make -j $(nproc)
    mv $less_build_dir/bin/pqcsign_* "$less_dst_dir/"
    cd $less_src_dir && rm -rf build

    # Restore the original source code files
    "$scripts_dir/copy_modified_src_files.sh" "restore" "LESS" "$less_src_dir" "all" "$root_dir"

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

    #__________________________________________________________________________
    # Set the source and destination directories for the PROV algorithm
    prov_src_dir=$nist_src_dir/PROV/Reference_Implementation
    prov_dst_dir=$bin_dir/PROV

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${prov_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$prov_src_dir/$variation"
        cd $variation_dir

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "PROV" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir/pqcsign" "$prov_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "PROV" "$variation_dir" "$variation" "$root_dir"

    done

    #__________________________________________________________________________
    # Set the source and destination directories for the QR_UOV algorithm
    qr_uov_src_dir=$nist_src_dir/QR_UOV/Reference_Implementation/
    qr_uov_dst_dir=$bin_dir/QR_UOV
    qr_uov_ref_code_dir=$qr_uov_src_dir/ref

    # Change to the QR_UOV source code directory
    cd $qr_uov_src_dir

    # Copy over modified files to the current variation directory
    "$scripts_dir/copy_modified_src_files.sh" "copy" "QR_UOV" "$qr_uov_ref_code_dir" "QR_UOV" "$root_dir"

    # Compile the different variations source code
    make clean >> /dev/null
    make 

    # Loop throuhg the variations and get the pqcsign binary
    for variation in "${qr_uov_variations[@]}"; do

        # Set the variation build directory path and change to it
        variation_dir="$qr_uov_src_dir/$variation/ref"
        cd $variation_dir

        # Copy over the pqcsign binary to the relevant bin directory
        mv "$variation_dir/pqcsign" "$qr_uov_dst_dir/pqcsign_$variation"

    done

    # Move back to the source directory and clean up
    cd $qr_uov_src_dir
    make clean >> /dev/null

    # Restore the original source code files
    "$scripts_dir/copy_modified_src_files.sh" "restore" "QR_UOV" "$qr_uov_ref_code_dir" "QR_UOV" "$root_dir"

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
        # :TODO Not a good idea, lowercase only used here. Main script doesn't work
        variation_lower="${variation,,}"
        cp $variation_dir/pqcsign $raccoon_dst_dir/pqcsign_$variation_lower
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "Raccoon" "$variation_dir" "$variation" "$root_dir"
    
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
    # TODO: Here should be SDitH_hypercube instead of just SDitH. Main script doesn't work
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

    #__________________________________________________________________________
    # Set the source and destination directories for the SNOVA algorithm
    snova_src_dir=$nist_src_dir/SNOVA/Reference_Implementation
    snova_dst_dir=$bin_dir/SNOVA

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${snova_variations[@]}"; do

        # Set the variation directory path and change to it
        variation_dir="$snova_src_dir/$variation"
        cd $variation_dir

        # Ensure previous build dir is removed and create a new one
        if [ -d "$variation_dir/build" ]; then
            rm -rf "$variation_dir/build" && mkdir -p "$variation_dir/build"
        else
            mkdir -p "$variation_dir/build"
        fi

        # Copy over modified files to the current variation directory
        "$scripts_dir/copy_modified_src_files.sh" "copy" "SNOVA" "$variation_dir" "$variation" "$root_dir"

        # Compile and move pqcsign binary to relevant bin directory
        # :TODO SNOVA is not creating?
        make clean >> /dev/null
        make pqcsign -j $(nproc)
        mv "$variation_dir/pqcsign" "$snova_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

        # Restore the original source code files
        "$scripts_dir/copy_modified_src_files.sh" "restore" "SNOVA" "$variation_dir" "$variation" "$root_dir"

        # Remove build dir after compilation if still present
        if [ -d "$variation_dir/build" ]; then
            rm -rf "$variation_dir/build"
        fi

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
    if [ -d "$orgin_lib_dir_backup/lib" ]; then
        rm -rf "$squirrels_lib_dir"
        cp -r "$orgin_lib_dir_backup/lib" "$squirrels_lib_dir"

    else
        echo -e "\nThe modded SQUIRRELS Lib directory could not be restored to default" >> "$root_dir/last_setup_error.log"
        echo -e "please manually restore the src/nist/SQUIRRELS/lib directory to avoid committing the built lib files\n" >> "$root_dir/last_setup_error.log"

    fi

    #__________________________________________________________________________
    # Set the source and destination directories for the TUOV algorithm
    tuov_src_dir=$nist_src_dir/TUOV/Reference_Implementation
    tuov_dst_dir=$bin_dir/TUOV

    # Change to the UOV source code directory
    cd $tuov_src_dir

    # Copy over modified files to the current variation directory
    "$scripts_dir/copy_modified_src_files.sh" "copy" "TUOV" "$tuov_src_dir" "all" "$root_dir"

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${tuov_variations[@]}"; do

        # Extract the variation type from the variation name
        variation_type=$(echo "$variation" | cut -d'_' -f2-)
        variation_dir="$tuov_src_dir/$variation_type"

        # Compile and move pqcsign binary to lib directory
        make clean >> /dev/null
        make "PROJ=$variation_type" -j $(nproc)
        mv "$tuov_src_dir/pqcsign" "$tuov_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    done

    # Restore the original files (makefile will only be resorted if is it the last variation)
    "$scripts_dir/copy_modified_src_files.sh" "restore" "TUOV" "$tuov_src_dir" "all" "$root_dir"

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
    # Set the source and destination directories for the VOX algorithm
    vox_src_dir=$nist_src_dir/VOX/Reference_Implementation/vox_sign
    vox_dst_dir=$bin_dir/VOX/

    # Change to the VOX source code directory
    cd $vox_src_dir

    # Copy over modified files to the current variation directory
    "$scripts_dir/copy_modified_src_files.sh" "copy" "VOX" "$vox_src_dir" "all" "$root_dir"

    # Loop through the different variations and compile the pqcsign binary
    for variation in "${vox_variations[@]}"; do

        echo "current variation - $variation"

        # Compile and move pqcsign binary to relevant bin directory
        make clean >> /dev/null
        make "PARAM=$variation"
        mv "$vox_src_dir/pqcsign" "$vox_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    done

    # Restore the original source code files
    "$scripts_dir/copy_modified_src_files.sh" "restore" "VOX" "$vox_src_dir" "all" "$root_dir"

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