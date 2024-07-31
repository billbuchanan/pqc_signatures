#!/bin/bash
# This utility script is used to load in the algorithm variations from the text files in the alg_variation_lists directory
# These arrays are then converted to comma-separated strings and exported as environment variables which can be imported
# into the calling script. Once the arrays have been imported into the calling script, the environment variables are cleared
# The script is passed the following arguments when called:
#
#   - arg-1: 'set' or 'clear' - determines if the environment variables are to be set or cleared
#   - arg-2: The path to the alg_variation_lists directory containing the algorithm variation text files


#---------------------------------------------------------------------------------------------------
function export_env_vars() {
    # Function for converting the arrays to comma-separated strings and export to environment variables

    # Export the created variation arrays to environment variables
    RACCOON_VARIATIONS=$(IFS=,; echo "${RACCOON_VARIATIONS[*]}")
    export RACCOON_VARIATIONS

    BISCUIT_VARIATIONS=$(IFS=,; echo "${BISCUIT_VARIATIONS[*]}")
    export BISCUIT_VARIATIONS

    CROSS_VARIATIONS=$(IFS=,; echo "${CROSS_VARIATIONS[*]}")
    export CROSS_VARIATIONS

    FAEST_VARIATIONS=$(IFS=,; echo "${FAEST_VARIATIONS[*]}")
    export FAEST_VARIATIONS

    FULEECA_VARIATIONS=$(IFS=,; echo "${FULEECA_VARIATIONS[*]}")
    export FULEECA_VARIATIONS

    PQSIGRM_VARIATIONS=$(IFS=,; echo "${PQSIGRM_VARIATIONS[*]}")
    export PQSIGRM_VARIATIONS

    SPHINCS_ALPHA_VARIATIONS=$(IFS=,; echo "${SPHINCS_ALPHA_VARIATIONS[*]}")
    export SPHINCS_ALPHA_VARIATIONS

    SQI_VARIATIONS=$(IFS=,; echo "${SQI_VARIATIONS[*]}")
    export SQI_VARIATIONS

    UOV_VARIATIONS=$(IFS=,; echo "${UOV_VARIATIONS[*]}")
    export UOV_VARIATIONS

    MED_VARIATIONS=$(IFS=,; echo "${MED_VARIATIONS[*]}")
    export MED_VARIATIONS

    HAWK_VARIATIONS=$(IFS=,; echo "${HAWK_VARIATIONS[*]}")
    export HAWK_VARIATIONS

    EHTV3V4_VARIATIONS=$(IFS=,; echo "${EHTV3V4_VARIATIONS[*]}")
    export EHTV3V4_VARIATIONS

    HUFU_VARIATIONS=$(IFS=,; echo "${HUFU_VARIATIONS[*]}")
    export HUFU_VARIATIONS

    THREE_WISE_VARIATIONS=$(IFS=,; echo "${THREE_WISE_VARIATIONS[*]}")
    export THREE_WISE_VARIATIONS

    MIRA_VARIATIONS=$(IFS=,; echo "${MIRA_VARIATIONS[*]}")
    export MIRA_VARIATIONS

    PERK_VARIATIONS=$(IFS=,; echo "${PERK_VARIATIONS[*]}")
    export PERK_VARIATIONS

    RYDE_VARIATIONS=$(IFS=,; echo "${RYDE_VARIATIONS[*]}")
    export RYDE_VARIATIONS

    SDITH_HYPERCUBE_VARIATIONS=$(IFS=,; echo "${SDITH_HYPERCUBE_VARIATIONS[*]}")
    export SDITH_HYPERCUBE_VARIATIONS

    ASCON_SIGN_VARIATIONS=$(IFS=,; echo "${ASCON_SIGN_VARIATIONS[*]}")
    export ASCON_SIGN_VARIATIONS

    MAYO_VARIATIONS=$(IFS=,; echo "${MAYO_VARIATIONS[*]}")
    export MAYO_VARIATIONS

    EMLE_SIG_2_0_VARIATIONS=$(IFS=,; echo "${EMLE_SIG_2_0_VARIATIONS[*]}")
    export EMLE_SIG_2_0_VARIATIONS

    DME_SIGN_VARIATIONS=$(IFS=,; echo "${DME_SIGN_VARIATIONS[*]}")
    export DME_SIGN_VARIATIONS

    XIFRAT1_SIGN_VARIATIONS=$(IFS=,; echo "${XIFRAT1_SIGN_VARIATIONS[*]}")
    export XIFRAT1_SIGN_VARIATIONS

    VOX_VARIATIONS=$(IFS=,; echo "${VOX_VARIATIONS[*]}")
    export VOX_VARIATIONS

    TUOV_VARIATIONS=$(IFS=,; echo "${TUOV_VARIATIONS[*]}")
    export TUOV_VARIATIONS

    PROV_VARIATIONS=$(IFS=,; echo "${PROV_VARIATIONS[*]}")
    export PROV_VARIATIONS

    QR_UOV_VARIATIONS=$(IFS=,; echo "${QR_UOV_VARIATIONS[*]}")
    export QR_UOV_VARIATIONS

    SNOVA_VARIATIONS=$(IFS=,; echo "${SNOVA_VARIATIONS[*]}")
    export SNOVA_VARIATIONS

    HPPC_VARIATIONS=$(IFS=,; echo "${HPPC_VARIATIONS[*]}")
    export HPPC_VARIATIONS

    ALTEQ_VARIATIONS=$(IFS=,; echo "${ALTEQ_VARIATIONS[*]}")
    export ALTEQ_VARIATIONS

    AIMER_VARIATIONS=$(IFS=,; echo "${AIMER_VARIATIONS[*]}")
    export AIMER_VARIATIONS

    EAGLESIGN_VARIATIONS=$(IFS=,; echo "${EAGLESIGN_VARIATIONS[*]}")
    export EAGLESIGN_VARIATIONS

    HAETAE_VARIATIONS=$(IFS=,; echo "${HAETAE_VARIATIONS[*]}")
    export HAETAE_VARIATIONS

    KAZ_SIGN_VARIATIONS=$(IFS=,; echo "${KAZ_SIGN_VARIATIONS[*]}")
    export KAZ_SIGN_VARIATIONS

    LESS_VARIATIONS=$(IFS=,; echo "${LESS_VARIATIONS[*]}")
    export LESS_VARIATIONS

    MIRITH_VARIATIONS=$(IFS=,; echo "${MIRITH_VARIATIONS[*]}")
    export MIRITH_VARIATIONS

    MQOM_VARIATIONS=$(IFS=,; echo "${MQOM_VARIATIONS[*]}")
    export MQOM_VARIATIONS

    PREON_VARIATIONS=$(IFS=,; echo "${PREON_VARIATIONS[*]}")
    export PREON_VARIATIONS
    
}

#---------------------------------------------------------------------------------------------------
function clear_env_vars(){
    # Function for clearing the enviroment variables after they have been loaded into the calling script

    # Clear environment variables
    unset RACCOON_VARIATIONS
    unset BISCUIT_VARIATIONS
    unset CROSS_VARIATIONS
    unset FAEST_VARIATIONS
    unset FULEECA_VARIATIONS
    unset PQSIGRM_VARIATIONS
    unset SPHINCS_ALPHA_VARIATIONS
    unset SQI_VARIATIONS
    unset UOV_VARIATIONS
    unset MED_VARIATIONS
    unset HAWK_VARIATIONS
    unset EHTV3V4_VARIATIONS
    unset HUFU_VARIATIONS
    unset THREE_WISE_VARIATIONS
    unset MIRA_VARIATIONS
    unset PERK_VARIATIONS
    unset RYDE_VARIATIONS
    unset SDITH_HYPERCUBE_VARIATIONS
    unset ASCON_SIGN_VARIATIONS
    unset MAYO_VARIATIONS
    unset EMLE_SIG_2_0_VARIATIONS
    unset DME_SIGN_VARIATIONS
    unset XIFRAT1_SIGN_VARIATIONS
    unset VOX_VARIATIONS
    unset TUOV_VARIATIONS
    unset PROV_VARIATIONS
    unset QR_UOV_VARIATIONS
    unset SNOVA_VARIATIONS
    unset HPPC_VARIATIONS
    unset ALTEQ_VARIATIONS
    unset AIMER_VARIATIONS
    unset EAGLESIGN_VARIATIONS
    unset HAETAE_VARIATIONS
    unset KAZ_SIGN_VARIATIONS
    unset LESS_VARIATIONS
    unset MIRITH_VARIATIONS
    unset MQOM_VARATIONS
    unset PREON_VARIATIONS

}

#---------------------------------------------------------------------------------------------------
function create_alg_arrays() {
    # Function for creating the algorithm variation arrays from the text files in the alg_variation_lists directory

    # Declare the alg_variation array
    local alg_variations_dir=$1

    # Create arrays for algorithm variations
    while IFS= read -r line; do
        RACCOON_VARIATIONS+=("$line")
    done < "$alg_variations_dir/Raccoon_variations.txt"

    while IFS= read -r line; do
        BISCUIT_VARIATIONS+=("$line")
    done < "$alg_variations_dir/Biscuit_variations.txt"

    while IFS= read -r line; do
        CROSS_VARIATIONS+=("$line")
    done < "$alg_variations_dir/CROSS_variations.txt"

    while IFS= read -r line; do
        FAEST_VARIATIONS+=("$line")
    done < "$alg_variations_dir/FAEST_variations.txt"

    while IFS= read -r line; do
        FULEECA_VARIATIONS+=("$line")
    done < "$alg_variations_dir/FuLeeca_variations.txt"

    while IFS= read -r line; do
        PQSIGRM_VARIATIONS+=("$line")
    done < "$alg_variations_dir/Enhanced_pqsigRM_variations.txt"

    while IFS= read -r line; do
        SPHINCS_ALPHA_VARIATIONS+=("$line")
    done < "$alg_variations_dir/SPHINCS_alpha_variations.txt"

    while IFS= read -r line; do
        SQI_VARIATIONS+=("$line")
    done < "$alg_variations_dir/SQIsign_variations.txt"

    while IFS= read -r line; do
        UOV_VARIATIONS+=("$line")
    done < "$alg_variations_dir/UOV_variations.txt"

    while IFS= read -r line; do
        MED_VARIATIONS+=("$line")
    done < "$alg_variations_dir/MEDS_variations.txt"

    while IFS= read -r line; do
        HAWK_VARIATIONS+=("$line")
    done < "$alg_variations_dir/HAWK_variations.txt"

    while IFS= read -r line; do
        EHTV3V4_VARIATIONS+=("$line")
    done < "$alg_variations_dir/EHTv3v4_variations.txt"

    while IFS= read -r line; do
        HUFU_VARIATIONS+=("$line")
    done < "$alg_variations_dir/HuFu_variations.txt"

    while IFS= read -r line; do
        THREE_WISE_VARIATIONS+=("$line")
    done < "$alg_variations_dir/3WISE_variations.txt"

    while IFS= read -r line; do
        MIRA_VARIATIONS+=("$line")
    done < "$alg_variations_dir/MIRA_variations.txt"

    while IFS= read -r line; do
        PERK_VARIATIONS+=("$line")
    done < "$alg_variations_dir/PERK_variations.txt"

    while IFS= read -r line; do
        RYDE_VARIATIONS+=("$line")
    done < "$alg_variations_dir/RYDE_variations.txt"

    while IFS= read -r line; do
        SDITH_HYPERCUBE_VARIATIONS+=("$line")
    done < "$alg_variations_dir/SDitH_hypercube_variations.txt"

    while IFS= read -r line; do
        ASCON_SIGN_VARIATIONS+=("$line")
    done < "$alg_variations_dir/Ascon_sign_variations.txt"

    while IFS= read -r line; do
        MAYO_VARIATIONS+=("$line")
    done < "$alg_variations_dir/MAYO_variations.txt"

    while IFS= read -r line; do
        EMLE_SIG_2_0_VARIATIONS+=("$line")
    done < "$alg_variations_dir/eMLE_Sig_2.0_variations.txt"

    while IFS= read -r line; do
        DME_SIGN_VARIATIONS+=("$line")
    done < "$alg_variations_dir/DME_Sign_variations.txt"

    while IFS= read -r line; do
        XIFRAT1_SIGN_VARIATIONS+=("$line")
    done < "$alg_variations_dir/Xifrat1_Sign_I_variations.txt"

    while IFS= read -r line; do
        VOX_VARIATIONS+=("$line")
    done < "$alg_variations_dir/VOX_variations.txt"

    while IFS= read -r line; do
        TUOV_VARIATIONS+=("$line")
    done < "$alg_variations_dir/TUOV_variations.txt"

    while IFS= read -r line; do
        PROV_VARIATIONS+=("$line")
    done < "$alg_variations_dir/PROV_variations.txt"

    while IFS= read -r line; do
        QR_UOV_VARIATIONS+=("$line")
    done < "$alg_variations_dir/QR_UOV_variations.txt"

    while IFS= read -r line; do
        SNOVA_VARIATIONS+=("$line")
    done < "$alg_variations_dir/SNOVA_variations.txt"

    while IFS= read -r line; do
        HPPC_VARIATIONS+=("$line")
    done < "$alg_variations_dir/HPPC_variations.txt"

    while IFS= read -r line; do
        ALTEQ_VARIATIONS+=("$line")
    done < "$alg_variations_dir/ALTEQ_variations.txt"

    while IFS= read -r line; do
        AIMER_VARIATIONS+=("$line")
    done < "$alg_variations_dir/AIMer_variations.txt"

    while IFS= read -r line; do
        EAGLESIGN_VARIATIONS+=("$line")
    done < "$alg_variations_dir/EagleSign_variations.txt"

    while IFS= read -r line; do
        HAETAE_VARIATIONS+=("$line")
    done < "$alg_variations_dir/HAETAE_variations.txt"

    while IFS= read -r line; do
        KAZ_SIGN_VARIATIONS+=("$line")
    done < "$alg_variations_dir/KAZ_SIGN_variations.txt"

    while IFS= read -r line; do
        LESS_VARIATIONS+=("$line")
    done < "$alg_variations_dir/LESS_variations.txt"

    while IFS= read -r line; do
        MIRITH_VARIATIONS+=("$line")
    done < "$alg_variations_dir/MiRitH_variations.txt"

    while IFS= read -r line; do
        MQOM_VARIATIONS+=("$line")
    done < "$alg_variations_dir/MQOM_variations.txt"

    while IFS= read -r line; do
        PREON_VARIATIONS+=("$line")
    done < "$alg_variations_dir/Preon_variations.txt"


}

#---------------------------------------------------------------------------------------------------
function main() {
    # Main function for setting or clearing the environment variables for the algorithm variations

    # Set or clear depending on the argument passed
    if [ "$1" == "set" ]; then

        # Ensure alg_variations_dir argument passed is a dir and exists
        if [ ! -d "$2" ]; then
            echo "Alg Array Creator Util Script Error: alg_variation_lists directory variables passed as argument in script is not a directory or does not exist"
            exit 1
        fi

        # Set the environment algorithm variations variables and export
        create_alg_arrays "$2"
        export_env_vars
        
    elif [ "$1" == "clear" ]; then
        clear_env_vars
    
    else
        echo -e "\nInvalid argument passed from calling script. Args passed to script must be either 'set' or 'clear'"
        echo -e "Args passed: arg-1: $1, arg-2: $2\n" 
    fi

}
main "$@"