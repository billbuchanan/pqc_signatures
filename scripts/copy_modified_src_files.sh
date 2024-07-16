#!/bin/bash
# This utility script is called by the main-setup script for linux environments 
# to copy or restore modified source files for each algorithm and their respective variations. 
# The script is called with the following arguments:
#
#   - util_flag: flag to determine if modified source files are to be copied or restored
#   - current_alg: the current algorithm being processed
#   - dst_variation_dir: the destination directory where the modified source files are to be copied or restored
#   - current_variation: the current variation of the algorithm being processed
#   - root_dir: the root directory of the project

# #---------------------------------------------------------------------------------------------------
# function ensure_default_src_files() {
#     # Function to ensure that the default source files are present in the destination directory

#     # Set the default source code directory for current algorithm
#     local default_src_dir="$root_dir/src/default_nist_src_files/linux/$current_alg"


#     # Set relevant file path variables from passed arguments
#     local current_working_dir="$1"
#     local file_path_1="$2" # makefile/cmakelists file path
#     local file_path_2="$3" # api.h file path if needed

#     # Check if the default source files are present in the destination directory



# }

#---------------------------------------------------------------------------------------------------
function copy_modified_src_files() {
    # Function to copy or restore modified source files for each algorithm and their respective variations
    # Note - Algorithms not listed here are assumed to follow the default structure seen in the last case

    # Set relevant path variables from passed arguments
    src_dir="$root_dir/src"
    pqcsign_src_file="$src_dir/performance-eval-scripts/pqcsign.c"
    temp_alg_backup="$temp_backup_dir/$current_alg"
    modified_files_path="$root_dir/src/modified_nist_src_files/linux/$current_alg"

    # Create the temp backup directory for the current algorithm if it does not exist
    echo "temp backup dir: $temp_alg_backup"
    if [ ! -d "$temp_alg_backup" ]; then
        mkdir -p "$temp_alg_backup"
    fi

    # Call relevant functionality for each algorithm, copying or restoring modified source files
    case $current_alg in

        "Biscuit")

            # Copy or restore modified source files for Biscuit algorithm
            if [ "$util_flag" == "copy" ]; then

                # Make temp copy of original makefile and copy over modified makefile
                cp "$dst_variation_dir/common.mk" "$temp_alg_backup/common.mk_$current_variation" # temp
                cp "$dst_variation_dir/common.mk" "$dst_variation_dir/temp_common_mk_copy"

                cp "$modified_files_path/common.mk_$current_variation" "$dst_variation_dir/common.mk"
                cp "$pqcsign_src_file" "$dst_variation_dir/pqcsign.c"

            elif [ "$util_flag" == "restore" ]; then
                rm -f "$dst_variation_dir/common.mk" && rm -f "$dst_variation_dir/pqcsign.c"
                mv "$dst_variation_dir/temp_common_mk_copy" "$dst_variation_dir/common.mk"

            fi
            ;;

        "CROSS")

            # Copy or restore modified source files for CROSS algorithm
            if [ "$util_flag" == "copy" ]; then

                # Make temp copy of original makefile and copy over modified makefile
                cp "$dst_variation_dir/include/api.h" "$temp_alg_backup/api_cross.h" # temp
                cp "$dst_variation_dir/include/api.h" "$dst_variation_dir/include/temp_api_copy.h"s

                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_cross" "$dst_variation_dir/Makefile"
                cp "$modified_files_path/api_cross.h" "$dst_variation_dir/include/api.h"
                cp "$pqcsign_src_file" "$dst_variation_dir/lib/pqcsign.c"

            elif [ "$util_flag" == "restore" ]; then
                # Restore default source files
                rm -f "$dst_variation_dir/Makefile" && rm -f "$dst_variation_dir/lib/pqcsign.c"
                mv "$dst_variation_dir/include/temp_api_copy.h"s "$dst_variation_dir/include/api.h"

            fi
            ;;

        "HuFu")

            # Copy or restore modified source files for HuFu algorithm
            if [ "$util_flag" == "copy" ]; then

                # Make temp copy of original makefile and copy over modified makefile
                cp "$dst_variation_dir/api.h" "$temp_alg_backup/api_$current_variation.h" # temp
                cp "$dst_variation_dir/api.h" "$dst_variation_dir/temp_api_copy.h"
                
                cp "$dst_variation_dir/Makefile" "$temp_alg_backup/Makefile_$current_variation" # temp
                cp "$dst_variation_dir/Makefile" "$dst_variation_dir/temp_make_copy"

                # Copy over modified files to source code directories
                cp "$modified_files_path/api_$current_variation.h" "$dst_variation_dir/api.h"
                cp "$modified_files_path/Makefile_$current_variation" "$dst_variation_dir/Makefile"
                cp "$pqcsign_src_file" "$dst_variation_dir/pqcsign.c"   

            elif [ "$util_flag" == "restore" ]; then
                # Restore default source files
                rm -f "$dst_variation_dir/Makefile" && rm -f "$dst_variation_dir/pqcsign.c"
                mv "$dst_variation_dir/temp_api_copy.h" "$dst_variation_dir/api.h"
                mv "$dst_variation_dir/temp_make_copy" "$dst_variation_dir/Makefile"

            fi
            ;;
        
        "SQIsign")

            # Copy or restore modified source files for SQIsign algorithm
            if [ "$util_flag" == "copy" ]; then

                # Make temp copy of original makefile and copy over modified makefile
                cp "$dst_variation_dir/apps/CMakeLists.txt" "$temp_alg_backup/CMakeLists_$current_variation.txt" # temp
                cp "$dst_variation_dir/apps/CMakeLists.txt" "$dst_variation_dir/apps/temp_cmakelists_copy"
                
                # Copy over modified files to source code directories
                cp "$modified_files_path/CMakeLists_sqi.txt" "$dst_variation_dir/apps/CMakeLists.txt"
                cp "$pqcsign_src_file" "$dst_variation_dir/apps/pqcsign.c"

            elif [ "$util_flag" == "restore" ]; then
                # Restore default source files
                rm -f "$dst_variation_dir/apps/CMakeLists.txt" && rm -f "$dst_variation_dir/apps/pqcsign.c"
                mv "$dst_variation_dir/apps/temp_cmakelists_copy" "$dst_variation_dir/apps/CMakeLists.txt"

            fi
            ;;

        "MIRA")

            # Copy or restore modified source files for MIRA algorithm
            if [ "$util_flag" == "copy" ]; then

                # Make temp copy of original makefile and copy over modified makefile
                cp "$dst_variation_dir/Makefile" "$temp_alg_backup/Makefile_$current_variation" # temp
                cp "$dst_variation_dir/Makefile" "$dst_variation_dir/temp_make_copy"

                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_$current_variation" "$dst_variation_dir/Makefile"
                cp "$pqcsign_src_file" "$dst_variation_dir/src/pqcsign.c"

            elif [ "$util_flag" == "restore" ]; then
                # Restore default source files
                rm -f "$dst_variation_dir/Makefile" && rm -f "$dst_variation_dir/src/pqcsign.c"
                mv "$dst_variation_dir/temp_make_copy" "$dst_variation_dir/Makefile"

            fi
            ;;

        "PERK")

            # Copy or restore modified source files for PERK algorithm
            if [ "$util_flag" == "copy" ]; then

                # Make temp copy of original makefile and copy over modified makefile
                cp "$dst_variation_dir/Makefile" "$temp_alg_backup/Makefile_$current_variation" # temp
                cp "$dst_variation_dir/Makefile" "$dst_variation_dir/temp_make_copy"
                
                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_$current_variation" "$dst_variation_dir/Makefile"
                cp "$pqcsign_src_file" "$dst_variation_dir/src/pqcsign.c"

            elif [ "$util_flag" == "restore" ]; then
                # Restore default source files
                rm -f "$dst_variation_dir/Makefile" && rm -f "$dst_variation_dir/src/pqcsign.c"
                mv "$dst_variation_dir/temp_make_copy" "$dst_variation_dir/Makefile"

            fi
            ;;

        "RYDE")

            # Copy or restore modified source files for RYDE algorithm
            if [ "$util_flag" == "copy" ]; then
                # Make temp copy of original makefile and copy over modified makefile
                cp "$dst_variation_dir/Makefile" "$temp_alg_backup/Makefile_$current_variation" # temp
                cp "$dst_variation_dir/Makefile" "$dst_variation_dir/temp_make_copy"

                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_$current_variation" "$dst_variation_dir/Makefile"
                cp "$pqcsign_src_file" "$dst_variation_dir/src/pqcsign.c"

            elif [ "$util_flag" == "restore" ]; then
                # Restore default source files
                rm -f "$dst_variation_dir/Makefile" && rm -f "$dst_variation_dir/src/pqcsign.c"
                mv "$dst_variation_dir/temp_make_copy" "$dst_variation_dir/Makefile"

            fi
            ;;

        "UOV")
            # NOTE - variation dir is just reference source code directory as modded source files are stored there, not in the variation directories

            # Copy or restore modified source files for UOV algorithm
            if [ "$util_flag" == "copy" ]; then

                # Make temp copy of original makefile and copy over modified makefile
                cp "$dst_variation_dir/Makefile" "$temp_alg_backup/Makefile_uov" # temp
                cp "$dst_variation_dir/Makefile" "$dst_variation_dir/temp_make_copy"

                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_uov" "$dst_variation_dir/Makefile"
                cp "$pqcsign_src_file" "$dst_variation_dir/pqcsign.c"


            elif [ "$util_flag" == "restore" ]; then

                # Restore default source files
                rm -f "$dst_variation_dir/Makefile" && rm -f "$dst_variation_dir/pqcsign.c"
                mv "$dst_variation_dir/temp_make_copy" "$dst_variation_dir/Makefile"

            fi
            ;;
        
        *)

            # Copy or restore modified source files for all other algorithms
            if [ "$util_flag" == "copy" ]; then

                # Make temp copy of original makefile and copy over modified makefile if it exists
                if [ -f "$dst_variation_dir/Makefile" ]; then

                    # Only copy makefile if it is not for Raccoon as default source code does not contain one
                    if [ "$current_alg" != "Raccoon" ]; then
                        cp "$dst_variation_dir/Makefile" "$temp_alg_backup/Makefile_$current_variation" # temp
                        mv "$dst_variation_dir/Makefile" "$dst_variation_dir/temp_make_copy"
                    fi

                fi

                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_$current_variation" "$dst_variation_dir/Makefile"
                cp "$pqcsign_src_file" "$dst_variation_dir/pqcsign.c"


            elif [ "$util_flag" == "restore" ]; then
            
                # Restore default source files
                rm -f "$dst_variation_dir/Makefile" && rm -f "$dst_variation_dir/pqcsign.c"

                # Only restore makefile if it is not for Raccoon as default source code does not contain one
                if [ "$current_alg" != "Raccoon" ]; then
                    mv "$dst_variation_dir/temp_make_copy" "$dst_variation_dir/Makefile"
                fi
            
            fi
            ;;
    esac

}

#---------------------------------------------------------------------------------------------------
function main() {
    # Main function used to control the copying or restoring of modified source files for each algorithm and their respective variations

    # Set local variables from passed arguments
    util_flag=$1
    current_alg=$2
    dst_variation_dir=$3
    current_variation=$4
    root_dir=$5

    # Set the temp backup directory path after getting root_dir
    temp_backup_dir="$root_dir/temp_src_backup"

    # Check arguments passed
    if [ "$util_flag" != "copy" ] && [ "$util_flag" != "restore" ] || [ -z "$current_alg" ] || [ -z "$dst_variation_dir" ] || [ -z "$current_variation" ]; then
        echo "Invalid arguments passed, arguments passed: $@"
        exit 1
    fi

    # # Create the temp backup directory if it does not exist recreate it if it does
    # temp_backup_util "create"

    # Copy or restore passed on util flag passed
    copy_modified_src_files $util_flag $current_alg $dst_variation_dir $current_variation

    # Clear the temp backup directory
    #temp_backup_util "clear"

}
main "$@"