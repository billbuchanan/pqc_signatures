#!/bin/bash

#------------------------------------------------------------------------------
root_dir=$(pwd)
src_dir=$root_dir/src
lib_dir=$root_dir/lib
test_data_dir=$root_dir/test_data
results_dir=$test_data_dir/results
alg_list_dir=$test_data_dir/alg_lists

sig_algs=("raccoon" "biscuit" "cross" "FAEST" "FuLecca" "pqsigRM" "SPHINCS-ALPHA" "sqi")

#------------------------------------------------------------------------------
function create_alg_arrays() {

    # Create arrays for algorithm variations
    raccoon_variations=()
    while IFS= read -r line; do
        raccoon_variations+=("$line")
    done < "$alg_list_dir/raccoon_variations.txt"

    biscuit_variations=()
    while IFS= read -r line; do
        biscuit_variations+=("$line")
    done < "$alg_list_dir/biscuit_variations.txt"

    cross_variations=()
    while IFS= read -r line; do
        cross_variations+=("$line")
    done < "$alg_list_dir/cross_variations.txt"

    FAEST_variations=()
    while IFS= read -r line; do
        FAEST_variations+=("$line")
    done < "$alg_list_dir/FAEST_variations.txt"

    FuLecca_variations=()
    while IFS= read -r line; do
        FuLecca_variations+=("$line")
    done < "$alg_list_dir/FuLeeca_variations.txt"

    pqsigRM_variations=()
    while IFS= read -r line; do
        pqsigRM_variations+=("$line")
    done < "$alg_list_dir/pqsigRM_variations.txt"

    SPHINCS_ALPHA_variations=()
    while IFS= read -r line; do
        SPHINCS_ALPHA_variations+=("$line")
    done < "$alg_list_dir/SPHINCS-ALPHA_variations.txt"

    sqi_variations=()
    while IFS= read -r line; do
        sqi_variations+=("$line")
    done < "$alg_list_dir/sqi_variations.txt"

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

    # Create the various alg arrays to be used in setup functions
    create_alg_arrays

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
function variations_setup() {

    # # Setting up the variations of the raccoon signature algorithm
    # raccoon_src_dir=$src_dir/raccoon
    # raccoon_dst_dir=$lib_dir/raccoon

    # cd $raccoon_src_dir

    # # Borrowed from raccoon 
    # for variation in "${raccoon_variations[@]}"; do
    #     make clean >> /dev/null
    #     make RACCF="-D$variation -DBENCH_TIMEOUT=2.0"
    #     cp $raccoon_src_dir/xtest $raccoon_dst_dir/xtest_$variation
    # done

    # make clean >> /dev/null 

    # Setting up the variations of the biscuit signature algorithm
    biscuit_src_dir=$src_dir/biscuit/Reference_Implementation
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
    cross_src_dir=$src_dir/cross/Reference_Implementation
    cross_dst_dir=$lib_dir/cross

    cd $cross_src_dir

    for variation in "${cross_variations[@]}"; do

        set_build_cross_flags

        # Compile and move pqcsign binary to lib directory
        make clean >> /dev/null
        make all CFLAGS="-Iinclude -std=c11 -g -Wall -D$algorithm_flag -D$security_level_flag -D$optimisation_flag -DAES_CTR_CSPRNG -DSHA3_HASH" -j $(nproc)
        mv "$cross_src_dir/pqcsign" "$cross_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    
    done

    # Setting up variations of the FAEST signature algorithm
    FAEST_src_dir=$src_dir/FAEST/Reference_Implementation
    FAEST_dst_dir=$lib_dir/FAEST

    cd $FAEST_src_dir

    for variation in "${FAEST_variations[@]}"; do
    
        # Set directory path based on current variation
        variation_dir_path="$FAEST_src_dir/$variation"
        
        cd $variation_dir_path
    
        # Compile and move pqcsign binary to lib directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir_path/pqcsign" "$FAEST_dst_dir/pqcsign_$variation"
        make clean >> /dev/null

    
    done

    # Setting up variations of the FuLecca signature algorithm
    FuLecca_src_dir=$src_dir/FuLecca/Reference_Implementation
    FuLecca_dst_dir=$lib_dir/FuLecca

    cd $FuLecca_src_dir

    for variation in "${FuLecca_variations[@]}"; do
    
        echo "current variation - $variation"
        # Set directory path based on current variation
        variation_dir_path="$FuLecca_src_dir/$variation"
        
        cd $variation_dir_path
        
        # Compile and move pqcsign binary to lib directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir_path/pqcsign" "$FuLecca_dst_dir/pqcsign_$variation"
        make clean >> /dev/null
    
    done

    # Setting up variations of the pqsigRM signature algorithm
    pqsigRM_src_dir=$src_dir/pqsigRM/Reference_Implementation
    pqsigRM_dst_dir=$lib_dir/pqsigRM

    cd $pqsigRM_src_dir

    for variation in "${pqsigRM_variations[@]}"; do
    
        echo "current variation - $variation"
        # Set directory path based on current variation
        variation_dir_path="$pqsigRM_src_dir/$variation"
        
        cd $variation_dir_path
        
        # Compile and move pqcsign binary to lib directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir_path/pqcsign" "$pqsigRM_dst_dir/pqcsign_$variation"
        make clean >> /dev/null
    
    done

    # Setting up variations of the SPHINCS-ALPHA signature algorithm
    SPHINCS_ALPHA_src_dir=$src_dir/SPHINCS-ALPHA/Reference_Implementation
    SPHINCS_ALPHA_dst_dir=$lib_dir/SPHINCS-ALPHA

    cd $SPHINCS_ALPHA_src_dir

    for variation in "${SPHINCS_ALPHA_variations[@]}"; do
    
        echo "current variation - $variation"
        # Set directory path based on current variation
        variation_dir_path="$SPHINCS_ALPHA_src_dir/$variation"
        
        cd $variation_dir_path
        
        # Compile and move pqcsign binary to lib directory
        make clean >> /dev/null
        make -j $(nproc)
        mv "$variation_dir_path/pqcsign" "$SPHINCS_ALPHA_dst_dir/pqcsign_$variation"
        make clean >> /dev/null
    
    done

    # Setting up variations of the sqi signature algorithm
    sqi_src_dir=$src_dir/sqi/Reference_Implementation
    sqi_dst_dir=$lib_dir/sqi
    sqi_build_dir=$src_dir/sqi/Reference_Implementation/build
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

}

#------------------------------------------------------------------------------
function main() {

    echo "Performing Environment Setup"
    environment_setup

    #echo -e "\nSetting up raccoon testing files"
    #raccoon_setup
    variations_setup

    echo -e "\nSetup complete, testing scripts can be found in the test_scripts directory"

}
main