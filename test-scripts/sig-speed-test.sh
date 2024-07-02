#!/bin/bash
#------------------------------------------------------------------------------
root_dir=$(dirname $(pwd))
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
function raccoon_test {

    for variation in "${raccoon_variations[@]}"; do
        echo -e "\nRunning raccoon test for $variation"
        $lib_dir/raccoon/xtest_$variation >> $results_dir/sig_speed_results.txt
    done

}

#------------------------------------------------------------------------------
function main() {
    create_alg_arrays
    raccoon_test
}
main