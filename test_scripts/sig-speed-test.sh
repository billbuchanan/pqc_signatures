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

    uov_variations=()
    while IFS= read -r line; do
        uov_variations+=("$line")
    done < "$alg_list_dir/uov_variations.txt"

    med_variations=()
    while IFS= read -r line; do
        med_variations+=("$line")
    done < "$alg_list_dir/MED-2023_variations.txt"

    hawk_variations=()
    while IFS= read -r line; do
        hawk_variations+=("$line")
    done < "$alg_list_dir/hawk_variations.txt"

}

#------------------------------------------------------------------------------
function cycles_test {

    for variation in "${raccoon_variations[@]}"; do
        echo -e "\nRunning raccoon test for $variation"
        $lib_dir/raccoon/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

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

    for variation in "${pqsigRM_variations[@]}"; do
        echo -e "\nRunning pqsigRM test for $variation"
        $lib_dir/pqsigRM/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${SPHINCS_ALPHA_variations[@]}"; do
        echo -e "\nRunning SPHINCS-ALPHA test for $variation"
        $lib_dir/SPHINCS-ALPHA/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${sqi_variations[@]}"; do
        echo -e "\nRunning sqi test for $variation"
        $lib_dir/sqi/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${uov_variations[@]}"; do
        echo -e "\nRunning uov test for $variation"
        $lib_dir/uov/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${med_variations[@]}"; do
        echo -e "\nRunning MEDS-2023 test for $variation"
        $lib_dir/MEDS-2023/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${hawk_variations[@]}"; do
        echo -e "\nRunning hawk test for $variation"
        $lib_dir/hawk/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

}

#------------------------------------------------------------------------------
function main() {

    if [ -d $results_dir ]; then
        rm -rf $results_dir/*
    fi


    create_alg_arrays
    cycles_test
}
main