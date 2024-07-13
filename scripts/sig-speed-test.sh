#!/bin/bash
#------------------------------------------------------------------------------
root_dir=$(dirname $(pwd))
src_dir=$root_dir/src
bin_dir=$root_dir/bin
test_data_dir=$root_dir/test_data
results_dir=$test_data_dir/results
alg_list_dir=$test_data_dir/alg_lists

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
function determine_hufu_inclusion() {

    # Notify user of HUFU test length Get choice to include HUFU in testing from user
    echo -e "WARNGING! - HUFU benchmarking takes a considerable amount of time to complete, even on a high performance machine\n"

    while true; do
        read -p "Would you like to include HUFU in the benchmarking? (y/n): " hufu_choice
        hufu_choice=$(echo $hufu_choice | tr '[:upper:]' '[:lower:]')
        
        if [ $hufu_choice == "y" ]; then
            hufu_included=1
            break

        elif [ $hufu_choice == "n" ]; then
            hufu_included=0
            break
        
        else
            echo -e "Invalid input. Please enter 'y' or 'n'\n"
        fi

    done

}

#------------------------------------------------------------------------------
function cycles_test {

    for variation in "${raccoon_variations[@]}"; do
        echo -e "\nRunning raccoon test for $variation"
        variation_lower="${variation,,}"
        $bin_dir/Raccoon/pqcsign_$variation_lower >> $results_dir/sig_speed_results.txt
    done

    for variation in "${biscuit_variations[@]}"; do
        echo -e "\nRunning biscuit test for $variation"
        $bin_dir/Biscuit/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${cross_variations[@]}"; do
        echo -e "\nRunning cross test for $variation"
        $bin_dir/CROSS/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${FAEST_variations[@]}"; do
        echo -e "\nRunning FAEST test for $variation"
        $bin_dir/FAEST/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${FuLecca_variations[@]}"; do
        echo -e "\nRunning FuLecca test for $variation"
        $bin_dir/FuLecca/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${pqsigRM_variations[@]}"; do
        echo -e "\nRunning pqsigRM test for $variation"
        $bin_dir/Enhanced_pqsigRM/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${SPHINCS_ALPHA_variations[@]}"; do
        echo -e "\nRunning SPHINCS-ALPHA test for $variation"
        $bin_dir/SPHINCS_alpha/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${sqi_variations[@]}"; do
        echo -e "\nRunning sqi test for $variation"
        $bin_dir/SQIsign/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${uov_variations[@]}"; do
        echo -e "\nRunning uov test for $variation"
        $bin_dir/UOV/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${med_variations[@]}"; do
        echo -e "\nRunning MEDS-2023 test for $variation"
        $bin_dir/MEDS/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${hawk_variations[@]}"; do
        echo -e "\nRunning hawk test for $variation"
        $bin_dir/HAWK/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done
    
    for variation in "${ehtv3v4_variations[@]}"; do
        echo -e "\nRunning EHTv3v4 test for $variation"
        $bin_dir/EHTv3v4/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    if [ $hufu_included == 1 ]; then

        for variation in "${hufu_variations[@]}"; do
            echo -e "\nRunning hufu test for $variation"
            $bin_dir/HuFu/pqcsign_$variation >> $results_dir/sig_speed_results.txt
        done

    else
        echo -e "\nSkipping HUFU benchmarking"
    fi

    for variation in "${three_wise_variations[@]}"; do
        echo -e "\nRunning 3WISE test for $variation"
        $bin_dir/3WISE/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${mira_variations[@]}"; do
        echo -e "\nRunning MIRA test for $variation"
        $bin_dir/MIRA/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${perk_variations[@]}"; do
        echo -e "\nRunning PERK test for $variation"
        $bin_dir/PERK/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${ryde_variations[@]}"; do
        echo -e "\nRunning RYDE test for $variation"
        $bin_dir/RYDE/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

    for variation in "${sdith_hypercube_variations[@]}"; do
        echo -e "\nRunning SDITH-Hypercube test for $variation"
        $bin_dir/SDitH/pqcsign_$variation >> $results_dir/sig_speed_results.txt
    done

}

#------------------------------------------------------------------------------
function main() {

    # Set up environment
    if [ -d $results_dir ]; then
        rm -rf $results_dir/*
    fi
    array_util_call
    
    # Determine if HUFU is to be included in testing
    determine_hufu_inclusion

    # Perform benchmarking
    cycles_test
}
main