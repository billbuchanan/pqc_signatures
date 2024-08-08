#!/bin/bash
# This script is used to run the signature speed tests for all algorithms and variations in the benchmarking suite
# The script will run the tests for the number of runs specified by the user and output the results to a 
# txt file to later be parsed by the python parser script
# The script will also prompt the user to determine if HuFu is to be included in the benchmarking due to its high run time


#---------------------------------------------------------------------------------------------------
# Set global path variables
root_dir=$(dirname $(pwd))
src_dir=$root_dir/src
bin_dir=$root_dir/bin
test_data_dir=$root_dir/test_data
results_dir=$test_data_dir/results
algs_list_dir=$test_data_dir/sig_algs_list
alg_variations_dir=$test_data_dir/alg_variation_lists
num_runs=0


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
    source "$root_dir/scripts/variation_array_util.sh" "clear"

}

#---------------------------------------------------------------------------------------------------
function determine_alg_inclusion() {
    # Function to determine if HUFU is to be included in the benchmarking

    # Notify user of HUFU test length Get choice to include HUFU in testing from user
    echo -e "WARNGING! - HUFU benchmarking takes a considerable amount of time to complete, even on a high performance machine\n"

    # Loop until valid input is received
    while true; do

        # Get user choice to include HUFU in benchmarking
        read -p "Would you like to include HUFU in the benchmarking? (y/n): " hufu_choice
        hufu_choice=$(echo $hufu_choice | tr '[:upper:]' '[:lower:]')
        
        # Check if input is valid and set choice flag
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

    # Notify user of SNOVA test length Get choice to include SNOVA in testing from user
    echo -e "\nWARNGING! - SNOVA benchmarking takes a considerable amount of time to complete, even on a high performance machine\n"

    # Loop until valid input is received
    while true; do

        # Get user choice to include SNOVA in benchmarking
        read -p "Would you like to include SNOVA in the benchmarking? (y/n): " snova_choice
        snova_choice=$(echo $snova_choice | tr '[:upper:]' '[:lower:]')
        
        # Check if input is valid and set choice flag
        if [ $snova_choice == "y" ]; then
            snova_included=1
            break

        elif [ $snova_choice == "n" ]; then
            snova_included=0
            break
        
        else
            echo -e "Invalid input. Please enter 'y' or 'n'\n"
            
        fi

    done

}

#---------------------------------------------------------------------------------------------------
function determine_run_nums() {
    # Function to determine the number of runs to be performed set by the user

    # Loop until valid input is received
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

}

#---------------------------------------------------------------------------------------------------
function cycles_test() {
    # Function to run the signature speed tests for each algorithm variation for current run

    # Set the output filename based on current run
    local output_file="sig_speed_results_run_$1.txt"

    # Ensure that the output file is empty and remove if present
    if [ -f $output_file ]; then
        rm $output_file
    fi

    # 3WISE variation testing
    for variation in "${three_wise_variations[@]}"; do
        echo -e "\nRunning 3WISE test for $variation"
        $bin_dir/3WISE/pqcsign_$variation >> $results_dir/$output_file
    done

    # AIMER variation testing
    for variation in "${aimer_variations[@]}"; do
        echo -e "\nRunning AIMER test for $variation"
        $bin_dir/AIMer/pqcsign_$variation >> $results_dir/$output_file
    done

    # ALTEQ variation testing
    for variation in "${alteq_variations[@]}"; do
        echo -e "\nRunning ALTEQ test for $variation"
        $bin_dir/ALTEQ/pqcsign_$variation >> $results_dir/$output_file
    done

    # Ascon_Sign variation testing
    for variation in "${ascon_sign_variations[@]}"; do
        echo -e "\nRunning Ascon_Sign test for $variation"
        $bin_dir/Ascon_Sign/pqcsign_$variation >> $results_dir/$output_file
    done

    # Biscuit variation testing
    for variation in "${biscuit_variations[@]}"; do
        echo -e "\nRunning biscuit test for $variation"
        $bin_dir/Biscuit/pqcsign_$variation >> $results_dir/$output_file
    done

    # CROSS variation testing
    for variation in "${cross_variations[@]}"; do
        echo -e "\nRunning cross test for $variation"
        $bin_dir/CROSS/pqcsign_$variation >> $results_dir/$output_file
    done

    # DME_Sign variation testing
    for variation in "${dme_sign_variations[@]}"; do

        # Only test the first two variations as the third does not have complete reference code (will be reviewed in future)
        if [ $variation != "dme-3rnds-8vars-64bits-sign" ]; then
            echo -e "\nRunning DME_Sign test for $variation"
            $bin_dir/DME_Sign/pqcsign_$variation >> $results_dir/$output_file
        else
            echo -e "\nSkipping DME_Sign test for $variation, due to lack of reference implementation code, will be reviewed in future"
        fi

    done

    # EagleSign variation testing
    for variation in "${eaglesign_variations[@]}"; do
        echo -e "\nRunning EagleSign test for $variation"
        $bin_dir/EagleSign/pqcsign_$variation >> $results_dir/$output_file
    done

    # EHTv3v4 variation testing
    for variation in "${ehtv3v4_variations[@]}"; do
        echo -e "\nRunning EHTv3v4 test for $variation"
        $bin_dir/EHTv3v4/pqcsign_$variation >> $results_dir/$output_file
    done

    # eMLE_Sig_2.0 variation testing
    for variation in "${emle_sig_2_0_variations[@]}"; do
        echo -e "\nRunning eMLE_Sig_2.0 test for $variation"
        $bin_dir/eMLE_Sig_2.0/pqcsign_$variation >> $results_dir/$output_file
    done

    # Enhanced_pqsigRM variation testing
    for variation in "${pqsigRM_variations[@]}"; do
        echo -e "\nRunning pqsigRM test for $variation"
        $bin_dir/Enhanced_pqsigRM/pqcsign_$variation >> $results_dir/$output_file
    done

    # FAEST variation testing
    for variation in "${FAEST_variations[@]}"; do
        echo -e "\nRunning FAEST test for $variation"
        $bin_dir/FAEST/pqcsign_$variation >> $results_dir/$output_file
    done

    # FuLeeca variation testing
    for variation in "${FuLeeca_variations[@]}"; do
        echo -e "\nRunning FuLeeca test for $variation"
        $bin_dir/FuLeeca/pqcsign_$variation >> $results_dir/$output_file
    done

    # HAETAE variation testing
    for variation in "${HAETAE_variations[@]}"; do
        echo -e "\nRunning HAETAE test for $variation"
        $bin_dir/HAETAE/pqcsign_$variation >> $results_dir/$output_file
    done

    # HAWK variation testing
    for variation in "${hawk_variations[@]}"; do
        echo -e "\nRunning hawk test for $variation"
        $bin_dir/HAWK/pqcsign_$variation >> $results_dir/$output_file
    done

    # HPPC variation testing
    for variation in "${hppc_variations[@]}"; do
        echo -e "\nRunning HPPC test for $variation"
        $bin_dir/HPPC/pqcsign_$variation >> $results_dir/$output_file
    done

    # HUFU variation testing if included is true
    if [ $hufu_included -eq 1 ]; then

        for variation in "${hufu_variations[@]}"; do
            echo -e "\nRunning hufu test for $variation"
            $bin_dir/HuFu/pqcsign_$variation >> $results_dir/$output_file
        done

    else
        echo -e "\nSkipping HUFU benchmarking"
    fi

    # Run testing for KAZ_SIGN algorithm
    for variation in "${kaz_sign_variations[@]}"; do
        echo -e "\nRunning KAZ_SIGN test for $variation"
        $bin_dir/KAZ_SIGN/pqcsign_$variation >> $results_dir/$output_file
    done

    # LESS variation testing
    for variation in "${less_variations[@]}"; do
        echo -e "\nRunning LESS test for $variation"
        $bin_dir/LESS/pqcsign_$variation >> $results_dir/$output_file
    done

    # MAYO variation testing
    for variation in "${mayo_variations[@]}"; do
        echo -e "\nRunning MAYO test for $variation"
        $bin_dir/MAYO/pqcsign_$variation >> $results_dir/$output_file
    done

    # MEDS variation testing
    for variation in "${med_variations[@]}"; do
        echo -e "\nRunning MEDS-2023 test for $variation"
        $bin_dir/MEDS/pqcsign_$variation >> $results_dir/$output_file
    done

    # MIRA variation testing
    for variation in "${mira_variations[@]}"; do
        echo -e "\nRunning MIRA test for $variation"
        $bin_dir/MIRA/pqcsign_$variation >> $results_dir/$output_file
    done

    # MiRitH variation testing
    for variation in "${mirith_variations[@]}"; do
        echo -e "\nRunning MiRitH test for $variation"
        $bin_dir/MiRitH/pqcsign_$variation >> $results_dir/$output_file
    done

    # MQOM variation testing
    for variation in "${mqom_variations[@]}"; do
        echo -e "\nRunning MQOM test for $variation"
        $bin_dir/MQOM/pqcsign_$variation >> $results_dir/$output_file
    done

    # PERK variation testing
    for variation in "${perk_variations[@]}"; do
        echo -e "\nRunning PERK test for $variation"
        $bin_dir/PERK/pqcsign_$variation >> $results_dir/$output_file
    done

    # Preon variation testing
    for variation in "${preon_variations[@]}"; do
        echo -e "\nRunning Preon test for $variation"
        $bin_dir/Preon/pqcsign_$variation >> $results_dir/$output_file
    done

    # PROV variation testing
    for variation in "${prov_variations[@]}"; do
        echo -e "\nRunning PROV test for $variation"
        $bin_dir/PROV/pqcsign_$variation >> $results_dir/$output_file
    done

    # QR_UOV variation testing
    for variation in "${qr_uov_variations[@]}"; do
        echo -e "\nRunning QR_UOV test for $variation"
        $bin_dir/QR_UOV/pqcsign_$variation >> $results_dir/$output_file
    done

    # Raccoon variation testing
    for variation in "${raccoon_variations[@]}"; do
        echo -e "\nRunning raccoon test for $variation"
        variation_lower="${variation,,}"
        $bin_dir/Raccoon/pqcsign_$variation_lower >> $results_dir/$output_file
    done

    # RYDE variation testing
    for variation in "${ryde_variations[@]}"; do
        echo -e "\nRunning RYDE test for $variation"
        $bin_dir/RYDE/pqcsign_$variation >> $results_dir/$output_file
    done

    # SDITH-Hypercube variation testing
    for variation in "${sdith_hypercube_variations[@]}"; do
        echo -e "\nRunning SDITH-Hypercube test for $variation"
        $bin_dir/SDitH/pqcsign_$variation >> $results_dir/$output_file
    done

    # SDitH-Threshold variation testing
    for variation in "${sdith_threshold_variations[@]}"; do
        echo -e "\nRunning SDitH Threshold test for $variation"
        $bin_dir/SDitH/pqcsign_$variation >> $results_dir/$output_file
    done

    # SNOVA variation testing if included is true
    if [ $snova_included -eq 1 ]; then

        for variation in "${snova_variations[@]}"; do
            echo -e "\nRunning SNOVA test for $variation"
            $bin_dir/SNOVA/pqcsign_$variation >> $results_dir/$output_file
        done

    else
        echo -e "\nSkipping SNOVA benchmarking"
    fi

    # SPHINCS_ALPHA variation testing
    for variation in "${SPHINCS_ALPHA_variations[@]}"; do
        echo -e "\nRunning SPHINCS-ALPHA test for $variation"
        $bin_dir/SPHINCS_alpha/pqcsign_$variation >> $results_dir/$output_file
    done

    # SQIsign variation testing
    for variation in "${sqi_variations[@]}"; do
        echo -e "\nRunning sqi test for $variation"
        $bin_dir/SQIsign/pqcsign_$variation >> $results_dir/$output_file
    done

    # SQURRIELS variation testing
    for variation in "${squirrels_variations[@]}"; do
        echo -e "\nRunning SQUIRRELS test for $variation"
        $bin_dir/SQUIRRELS/pqcsign_$variation >> $results_dir/$output_file
    done

    # TUOV variation testing
    for variation in "${tuov_variations[@]}"; do
        echo -e "\nRunning TUOV test for $variation"
        $bin_dir/TUOV/pqcsign_$variation >> $results_dir/$output_file
    done

    # UOV variation testing
    for variation in "${uov_variations[@]}"; do
        echo -e "\nRunning uov test for $variation"
        $bin_dir/UOV/pqcsign_$variation >> $results_dir/$output_file
    done

    # VOX variation testing
    for variation in "${vox_variations[@]}"; do
        echo -e "\nRunning VOX test for $variation"
        $bin_dir/VOX/pqcsign_$variation >> $results_dir/$output_file
    done

    # Wave variation testing
    for variation in "${wave_variations[@]}"; do
        echo -e "\nRunning Wave test for $variation"
        $bin_dir/Wave/pqcsign_$variation >> $results_dir/$output_file
    done

    # Xifrat1_Sign variation testing
    for variation in "${xifrat1_sign_variations[@]}"; do
        echo -e "\nRunning Xifrat1_Sign test for $variation"
        $bin_dir/Xifrat1_Sign_I/pqcsign_$variation >> $results_dir/$output_file
    done

}

#---------------------------------------------------------------------------------------------------
function main() {
    # Main function to control the signature speed test runs

    # Set up environment
    if [ -d $results_dir ]; then
        rm -rf $results_dir/*
    fi
    array_util_call
    
    # Determine if HUFU is to be included in testing and number of test runs
    determine_alg_inclusion
    determine_run_nums

    # Perform benchmarking for number of specified runs
    # :TODO Iteration over the runs doesn't work
    for run_num in $(seq 1 $number_of_runs); do
        echo "Performing Run $run_num"
        cycles_test "$run_num"
    done

    # Output that benchmarking has completed to user
    echo -e "\nBenchmarking has completed. Results can be found in the test_data/results directory\n"
    
}
main