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
}

#------------------------------------------------------------------------------
function cycles_test {

    # for variation in "${raccoon_variations[@]}"; do
    #     echo -e "\nRunning raccoon test for $variation"
    #     $lib_dir/raccoon/xtest_$variation >> $results_dir/sig_speed_results.txt
    # done

    for variation in "${biscuit_variations[@]}"; do
        echo -e "\nRunning biscuit test for $variation"
        $lib_dir/biscuit/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${cross_variations[@]}"; do
        echo -e "\nRunning cross test for $variation"
        $lib_dir/cross/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${FAEST_variations[@]}"; do
        echo -e "\nRunning FAEST test for $variation"
        $lib_dir/FAEST/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${FuLecca_variations[@]}"; do
        echo -e "\nRunning FuLecca test for $variation"
        $lib_dir/FuLecca/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

}

#------------------------------------------------------------------------------
function main() {
    create_alg_arrays
    cycles_test
}
main