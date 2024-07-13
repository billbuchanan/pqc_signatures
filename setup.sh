#!/bin/bash

#------------------------------------------------------------------------------
root_dir=$(pwd)
src_dir=$root_dir/src
nist_src_dir=$src_dir/nist
lib_dir=$root_dir/lib
test_data_dir=$root_dir/test_data
results_dir=$test_data_dir/results
alg_list_dir=$test_data_dir/alg_lists

sig_algs=(
    "raccoon"
    "biscuit"
    "cross"
    "FAEST"
    "FuLecca"
    "pqsigRM"
    "SPHINCS_ALPHA"
    "sqi"
    "uov"
    "MEDS_2023"
    "hawk"
    "EHTv3v4"
    "HuFu"
    "3WISE"
    "MIRA"
    "perk"
    "ryde"
    "SDitH"
)

#------------------------------------------------------------------------------
function array_util_call() {

    # Call the array utility script to export the variation arrays
    source "$root_dir/scripts/variation_array_util.sh" "set" "$alg_list_dir"

    # Import the variation arrays from environment variables
    IFS=',' read -r -a raccoon_variations <<< "$RACCON_VARIATIONS"
    IFS=',' read -r -a biscuit_variations <<< "$BISCUIT_VARIATIONS"
    IFS=',' read -r -a cross_variations <<< "$CROSS_VARIATIONS"
    IFS=',' read -r -a faest_variations <<< "$FAEST_VARIATIONS"
    IFS=',' read -r -a fulecca_variations <<< "$FULECCA_VARIATIONS"
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

    # Call the array utility script to clear environment variables
    source "$root_dir/scripts/variation_array_util.sh" "clear"

}

#------------------------------------------------------------------------------
function environment_setup() {

    # Check if the base required directories exist
    required_dirs=($lib_dir $results_dir)

    for dir in "${required_dirs[@]}"; do

        if [ ! -d $dir ]; then
            mkdir -p $dir
        else
            rm -rf $dir && mkdir -p $dir
        fi

    done

    # Check if the required lib directories exist for the various signature algorithms
    for sig_alg in "${sig_algs[@]}"; do

        if [ ! -d $lib_dir/$sig_alg ]; then
            mkdir -p $lib_dir/$sig_alg
        else
            rm -rf $lib_dir/$sig_alg && mkdir -p $lib_dir/$sig_alg
        fi

    done

    # Install required dependencies
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

    # Call the array utility script to get the alg variation arrays
    array_util_call

}


#------------------------------------------------------------------------------
function set_build_cross_flags() {

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

#------------------------------------------------------------------------------
function copy_modifed_src_files() {

    

}


#------------------------------------------------------------------------------
function variations_setup() {

    mod_file_dir="$root_dir/src/modified_nist_src_files/linux"

    # Setting up the variations of the raccoon signature algorithm
    raccoon_src_dir=$nist_src_dir/Raccoon/Reference_Implementation
    raccoon_dst_dir=$lib_dir/raccoon

    echo "raccon_variatios array call from a diferent funciton: ${raccon_variations[@]}"
    
    # Loop through the variation dirs
    for variation in "${raccoon_variations[@]}"; do
        variation_dir="$raccoon_src_dir/$variation"
        cd $variation_dir
        make clean >> /dev/null
        make -j $(nproc)
        variation_lower="${variation,,}"
        cp $variation_dir/pqcsign $raccoon_dst_dir/pqcsign_$variation_lower
        make clean >> /dev/null
    done

    # Setting up the variations of the biscuit signature algorithm
    biscuit_src_dir=$nist_src_dir/biscuit/Reference_Implementation
    biscuit_dst_dir=$lib_dir/biscuit

    cd $biscuit_src_dir

    # Loop through the variation dirs
    for variation_dir in "${biscuit_variations[@]}"; do
        variation_dir_path="$biscuit_src_dir/$variation_dir"
        cd $variation_dir_path
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir_path/pqcsign" "$biscuit_dst_dir/pqcsign_$variation_dir"
        make clean >> /dev/null
    done


    # Setting up variations of the cross signing algorithm
    cross_src_dir=$nist_src_dir/cross/Reference_Implementation
    cross_dst_dir=$lib_dir/cross

    cd $cross_src_dir

    for variation in "${cross_variations[@]}"; do

        set_build_cross_flags

        # Compile and move pqcsign binary to lib directory
        make clean >> /dev/null
        make all CFLAGS="-Iinclude -std=c11 -g -Wall -D$algorithm_flag \
            -D$security_level_flag -D$optimisation_flag -DAES_CTR_CSPRNG -DSHA3_HASH" -j $(nproc)
            
        mv "$cross_src_dir/pqcsign" "$cross_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    
    done

    # Setting up variations of the FAEST signature algorithm
    faest_src_dir=$nist_src_dir/FAEST/Reference_Implementation
    faest_dst_dir=$lib_dir/FAEST

    cd $faest_src_dir

    for variation in "${faest_variations[@]}"; do
    
        # Set directory path based on current variation
        variation_dir_path="$faest_src_dir/$variation"
        
        cd $variation_dir_path
    
        # Compile and move pqcsign binary to lib directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir_path/pqcsign" "$faest_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    
    done

    # Setting up variations of the FuLecca signature algorithm
    fulecca_src_dir=$nist_src_dir/FuLecca/Reference_Implementation
    fulecca_dst_dir=$lib_dir/FuLecca

    cd $FuLecca_src_dir

    for variation in "${fulecca_variations[@]}"; do
    
        echo "current variation - $variation"
        # Set directory path based on current variation
        variation_dir_path="$fulecca_src_dir/$variation"
        
        cd $variation_dir_path
        
        # Compile and move pqcsign binary to lib directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir_path/pqcsign" "$fulecca_dst_dir/pqcsign_$variation"
        make clean >> /dev/null
    
    done

    # Setting up variations of the pqsigRM signature algorithm
    pqsigrm_src_dir=$nist_src_dir/pqsigRM/Reference_Implementation
    pqsigrm_dst_dir=$lib_dir/pqsigRM

    cd $pqsigrm_src_dir

    for variation in "${pqsigrm_variations[@]}"; do
    
        echo "current variation - $variation"
        # Set directory path based on current variation
        variation_dir_path="$pqsigrm_src_dir/$variation"
        
        cd $variation_dir_path
        
        # Compile and move pqcsign binary to lib directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir_path/pqcsign" "$pqsigrm_dst_dir/pqcsign_$variation"
        make clean >> /dev/null
    
    done

    # Setting up variations of the SPHINCS-ALPHA signature algorithm
    sphincs_alpha_src_dir=$nist_src_dir/SPHINCS_ALPHA/Reference_Implementation
    sphincs_alpha_dst_dir=$lib_dir/SPHINCS_ALPHA

    cd $sphincs_alpha_src_dir

    for variation in "${sphincs_alpha_variations[@]}"; do

        echo "current variation - $variation"
        # Set directory path based on current variation
        variation_dir_path="$sphincs_alpha_src_dir/$variation"
        
        cd $variation_dir_path
        
        # Compile and move pqcsign binary to lib directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir_path/pqcsign" "$sphincs_alpha_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    
    done

    # Setting up variations of the sqi signature algorithm
    sqi_src_dir=$nist_src_dir/sqi/Reference_Implementation
    sqi_dst_dir=$lib_dir/sqi
    sqi_build_dir=$nist_src_dir/sqi/Reference_Implementation/build
    sqi_apps_dir=$sqi_build_dir/apps

    cd $sqi_src_dir

    if [ ! -d build ]; then
        mkdir build
    else
        rm -rf build && mkdir build
    fi

    cd $sqi_build_dir
    cmake -DSQISIGN_BUILD_TYPE=ref -DCMAKE_BUILD_TYPE=Release ../ 
    make -j $(nproc)
    mv $sqi_apps_dir/pqcsign_* "$sqi_dst_dir/"
    make clean >> /dev/null && cd $sqi_src_dir && rm -rf build


    # Setting up variations of the uov signature algorithm
    uov_src_dir=$nist_src_dir/uov/Reference_Implementation
    uov_dst_dir=$lib_dir/uov

    cd $uov_src_dir

    for variation in "${uov_variations[@]}"; do


        variation_type=$(echo "$variation" | cut -d'_' -f2-)
        
        # Compile and move pqcsign binary to lib directory
        make clean >> /dev/null
        make "PROJ=$variation_type" -j $(nproc)
        mv "$uov_src_dir/pqcsign" "$uov_dst_dir/pqcsign_$variation"
        make clean >> /dev/null
    
    done

    # Setting up variations of the MED-2023 signature algorithm
    med_src_dir=$nist_src_dir/MEDS-2023/Reference_Implementation
    med_dst_dir=$lib_dir/MEDS_2023

    cd $med_src_dir

    for variation in "${med_variations[@]}"; do

        variation_dir_path="$med_src_dir/$variation"
        cd $variation_dir_path
        # current_dir=$(pwd)
        # echo -e "\ncurrent dir - $current_dir\n"
        make clean >> /dev/null
        make
        mv "$variation_dir_path/pqcsign" "$med_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    done

    # Setting up variations of the hawk signature algorithm
    hawk_src_dir=$nist_src_dir/hawk/Reference_Implementation
    hawk_dst_dir=$lib_dir/hawk

    cd $hawk_src_dir

    for variation in "${hawk_variations[@]}"; do

        variation_dir_path="$hawk_src_dir/$variation"
        cd $variation_dir_path
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir_path/pqcsign" "$hawk_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    done

    # Setting up variations of the eht3v4 signature algorithm
    ehtv3v4_src_dir=$nist_src_dir/EHTv3v4/Reference_Implementation
    eht3v4_dst_dir=$lib_dir/EHTv3v4

    cd $ehtv3v4_src_dir

    for variation in "${ehtv3v4_variations[@]}"; do

        variation_dir_path="$ehtv3v4_src_dir/$variation"
        cd $variation_dir_path
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir_path/pqcsign" "$eht3v4_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    done

    # Setting up variations of the hufu signature algorithm
    hufu_src_dir=$nist_src_dir/HuFu/Reference_Implementation
    hufu_dst_dir=$lib_dir/HuFu

    for variation in "${hufu_variations[@]}"; do

        variation_dir_path="$hufu_src_dir/$variation"
        cd $variation_dir_path
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir_path/pqcsign" "$hufu_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    done

    # Setting up variations of the 3wise signature algorithm
    three_wise_src_dir=$nist_src_dir/3WISE/Reference_Implementation
    three_wise_dst_dir=$lib_dir/3WISE
    three_wise_flint_path=$three_wise_src_dir/flint

    cd $three_wise_src_dir

    # Ensure there is no previous build of flint library
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

    echo -e "\nFlint library setup complete\n"

    # Setting up the 3WISE variations
    for variation in "${three_wise_variations[@]}"; do

        variation_dir_path="$three_wise_src_dir/$variation"
        cd $variation_dir_path
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir_path/pqcsign" "$three_wise_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    done

    # Setting up variations of the MIRA signature algorithm
    mira_src_dir=$nist_src_dir/MIRA/Reference_Implementation
    mira_dst_dir=$lib_dir/MIRA

    cd $mira_src_dir

    for variation in "${mira_variations[@]}"; do

        variation_dir_path="$mira_src_dir/$variation"
        cd $variation_dir_path
        make clean >> /dev/null
        make all -j $(nproc)
        mv "$variation_dir_path/bin/pqcsign" "$mira_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    done

    # Setting up variations of the PERK signature algorithm
    perk_src_dir=$nist_src_dir/perk/Reference_Implementation
    perk_dst_dir=$lib_dir/perk

    cd $perk_src_dir

    for variation in "${perk_variations[@]}"; do

        variation_dir_path="$perk_src_dir/$variation"
        cd $variation_dir_path
        make clean >> /dev/null
        make all -j $(nproc)
        mv "$variation_dir_path/pqcsign" "$perk_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    done

    # Setting up variations of the ryde signature algorithm
    ryde_src_dir=$nist_src_dir/ryde/Reference_Implementation
    ryde_dst_dir=$lib_dir/ryde

    cd $ryde_src_dir

    for variation in "${ryde_variations[@]}"; do

        echo "current variation - $variation"

        variation_dir_path="$ryde_src_dir/$variation"
        cd $variation_dir_path
        make clean >> /dev/null
        make all -j $(nproc)
        mv "$variation_dir_path/bin/pqcsign" "$ryde_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    done

    # Setting up variations of the sdith signature algorithm
    sdith_src_dir=$nist_src_dir/SDitH/Reference_Implementation
    sdith_hybercube_src_dir="$sdith_src_dir/Hypercube_Variant"
    sdith_threshold_src_dir="$sdith_src_dir/Threshold_Variant"
    sdith_dst_dir=$lib_dir/SDitH

    # Setting up the Hypercube variants
    cd $sdith_hybercube_src_dir

    for variation in "${sdith_hypercube_variations[@]}"; do

        variation_dir_path="$sdith_hybercube_src_dir/$variation"
        cd $variation_dir_path
        make clean >> /dev/null
        make
        mv "$variation_dir_path/pqcsign" "$sdith_dst_dir/pqcsign_$variation"
        make clean >> /dev/null


    done

}

#------------------------------------------------------------------------------
function main() {

    # Configure setup environment
    echo "Performing Environment Setup"
    environment_setup

    # Setup the various algorithms and their variations
    variations_setup

    echo -e "\nSetup complete, testing scripts can be found in the test_scripts directory"

}
main