#!/bin/bash

#------------------------------------------------------------------------------
root_dir=$(pwd)
src_dir=$root_dir/src
lib_dir=$root_dir/lib
test_data_dir=$root_dir/test_data
results_dir=$test_data_dir/results
alg_list_dir=$test_data_dir/alg_lists

sig_algs=("raccoon")

#------------------------------------------------------------------------------
function create_alg_arrays() {

    # Create arrays for algorithm variations
    raccoon_variations=()
    while IFS= read -r line; do
        raccoon_variations+=("$line")
    done < "$alg_list_dir/raccoon_variations.txt"

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
function raccoon_setup() {

    raccoon_src_dir=$src_dir/raccoon
    raccoon_dst_dir=$lib_dir/raccoon

    cd $raccoon_src_dir

    # Borrowed from raccoon 
    for variation in "${raccoon_variations[@]}"; do
        make clean
        make RACCF="-D$variation -DBENCH_TIMEOUT=2.0"
        cp $raccoon_src_dir/xtest $raccoon_dst_dir/xtest_$variation
    done

    make clean 
    cd $root_dir

}

#------------------------------------------------------------------------------
function main() {

    echo "Performing Environment Setup"
    environment_setup

    echo -e "\nSetting up raccoon testing files"
    raccoon_setup

    echo -e "\nSetup complete, testing scripts can be found in the test_scripts directory"

}
main