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

#---------------------------------------------------------------------------------------------------
function ensure_default_src_files() {
    # Function to ensure that the default source files are present in the destination directory

    # Set the default source code directory for current algorithm
    local default_src_dir="$root_dir/src/default_nist_src_files/linux/$current_alg"

    # Set relevant file path variables from passed arguments
    local current_working_dir="$1"
    local default_filepath="$2" # default source file
    local main_backup="$3" # file being checked
    local temp_backup="$4" # temp copy of file being checked
    local current_variation=$(echo "$current_working_dir" | sed 's|.*/||')

    # Check if the default source files are present in the destination directory
    if head -n 1 "$default_filepath" | grep -E "#modified-flag|//modified-flag" > /dev/null; then

        echo -e "\n#################################################################" >> "$root_dir/last_setup_error.log"
        echo "Default source file checker outputs"
        echo "Current Algorithm: $current_alg"
        echo -e "\nDefault source files contains modified flag\n" >> "$root_dir/last_setup_error.log"

        # Check to see if temp copy is present in current working dir
        if [ -f "$temp_backup" ]; then
            echo -e "Temp copy was present using that to restore default source files\n" >> "$root_dir/last_setup_error.log"
            mv "$temp_backup" "$default_filepath"
            
        else

            echo -e "Temp backup was not present\n" >> "$root_dir/last_setup_error.log"

            # Try to fallback to main-backup copy
            if [ -f "$main_backup" ]; then
                echo "Main backup was present using that to restore default source files" >> "$root_dir/last_setup_error.log"
                cp "$main_backup" "$default_filepath"
                cp "$main_backup" "$temp_backup"

            else
                # Output not able to automatically restore default and that user must manually restore later
                echo "Unable to automatically restore default source files for $current_alg $current_variation" >> "$root_dir/last_setup_error.log"
                echo "The testing binaries should still compile, but the default source files must be manually restored if needed" >> "$root_dir/last_setup_error.log"
                
            fi

        fi

    fi

}

#---------------------------------------------------------------------------------------------------
function copy_modified_src_files() {
    # Function to copy or restore modified source files for each algorithm and their respective variations
    # Note - Algorithms not listed here are assumed to follow the default structure seen in the last case

    # Set relevant path variables from passed arguments
    src_dir="$root_dir/src"
    pqcsign_src_file="$src_dir/performance_eval_scripts/pqcsign.c"
    main_alg_backup="$main_backup_dir/$current_alg"
    modified_files_path="$root_dir/src/modified_nist_src_files/linux/$current_alg"

    # Create the temp backup directory for the current algorithm if it does not exist
    if [ ! -d "$main_alg_backup" ]; then
        mkdir -p "$main_alg_backup"
    fi

    # Call relevant functionality for each algorithm, copying or restoring modified source files
    case $current_alg in

        "Biscuit")

            # Set the filepaths for the files being worked with
            make_default_filepath="$dst_variation_dir/common.mk"
            make_main_backup_file="$main_alg_backup/common.mk_$current_variation"
            make_temp_backup_file="$dst_variation_dir/temp_common_mk_copy"

            # Copy or restore modified source files for Biscuit algorithm
            if [ "$util_flag" == "copy" ]; then

                # Ensure the default source code is present for makefile
                ensure_default_src_files "$dst_variation_dir" "$make_default_filepath" "$make_main_backup_file" "$make_temp_backup_file"

                # Make temp copy of original makefile and copy over modified makefile
                cp "$make_default_filepath" "$make_main_backup_file" 
                cp "$make_default_filepath" "$make_temp_backup_file" # temp stored in working_dir

                cp "$modified_files_path/common.mk_$current_variation" "$make_default_filepath"
                cp "$pqcsign_src_file" "$dst_variation_dir/pqcsign.c"


            elif [ "$util_flag" == "restore" ]; then
                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/pqcsign.c"
                mv "$make_temp_backup_file" "$make_default_filepath"

            fi
            ;;

        "CROSS")

            # Set the filepaths for the files being worked with
            api_default_filepath="$dst_variation_dir/include/api.h"
            api_main_backup_file="$main_alg_backup/api_cross.h"
            api_temp_backup_file="$dst_variation_dir/include/temp_api_copy.h"
            make_default_filepath="$dst_variation_dir/Makefile"

            # Copy or restore modified source files for CROSS algorithm
            if [ "$util_flag" == "copy" ]; then

                # Ensure the default source code is present for makefile (only check api.h as there is no default makefile)
                ensure_default_src_files "$dst_variation_dir" "$api_default_filepath" "$api_main_backup_file" "$api_temp_backup_file"

                # Create copies of original makefile and api.h and copy
                cp "$api_default_filepath" "$api_main_backup_file"
                cp "$api_default_filepath" "$api_temp_backup_file" # temp stored in working_dir
                
                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_cross" "$make_default_filepath"
                cp "$modified_files_path/api_cross.h" "$api_default_filepath"
                cp "$pqcsign_src_file" "$dst_variation_dir/lib/pqcsign.c"


            elif [ "$util_flag" == "restore" ]; then
                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/lib/pqcsign.c"
                mv "$api_temp_backup_file" "$api_default_filepath"

            fi
            ;;

        "HuFu")

            # Set the filepaths for the files being worked with
            api_default_filepath="$dst_variation_dir/api.h"
            api_main_backup_file="$main_alg_backup/api_$current_variation.h"
            api_temp_backup_file="$dst_variation_dir/temp_api_copy.h"

            make_default_filepath="$dst_variation_dir/Makefile"
            make_main_backup_file="$main_alg_backup/Makefile_$current_variation"
            make_temp_backup_file="$dst_variation_dir/temp_make_copy"

            # Copy or restore modified source files for HuFu algorithm
            if [ "$util_flag" == "copy" ]; then

                # Ensure the default source code is present for makefile
                ensure_default_src_files "$dst_variation_dir" "$api_default_filepath" "$api_main_backup_file" "$api_temp_backup_file"
                ensure_default_src_files "$dst_variation_dir" "$make_default_filepath" "$make_main_backup_file" "$make_temp_backup_file"

                # Make temp copy of original makefile and api.h and copy over modified makefile
                cp "$api_default_filepath" "$api_main_backup_file"
                cp "$api_default_filepath" "$api_temp_backup_file" # temp stored in working_dir
                cp "$make_default_filepath" "$make_main_backup_file"
                cp "$make_default_filepath" "$make_temp_backup_file" # temp stored in working_dir

                # Copy over modified files to source code directories
                cp "$modified_files_path/api_$current_variation.h" "$api_default_filepath"
                cp "$modified_files_path/Makefile_$current_variation" "$make_default_filepath"
                cp "$pqcsign_src_file" "$dst_variation_dir/pqcsign.c"   


            elif [ "$util_flag" == "restore" ]; then
                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/pqcsign.c"
                mv "$api_temp_backup_file" "$api_default_filepath"
                mv "$make_temp_backup_file" "$make_default_filepath"

            fi
            ;;
        
        "SQIsign")

            # Set the filepaths for the files being worked with
            make_default_filepath="$dst_variation_dir/apps/CMakeLists.txt"
            make_main_backup_file="$main_alg_backup/CMakeLists_$current_variation.txt"
            make_temp_backup_file="$dst_variation_dir/apps/temp_cmakelists_copy"

            # Copy or restore modified source files for SQIsign algorithm
            if [ "$util_flag" == "copy" ]; then

                # Ensure the default source code is present for makefile
                ensure_default_src_files "$dst_variation_dir/apps" "$make_default_filepath" "$make_main_backup_file" "$make_temp_backup_file"

                # Make temp copy of original makefile and copy over modified makefile
                cp "$make_default_filepath" "$make_main_backup_file" 
                cp "$make_default_filepath" "$make_temp_backup_file" # temp stored in working_dir
                
                # Copy over modified files to source code directories
                cp "$modified_files_path/CMakeLists_sqi.txt" "$make_default_filepath"
                cp "$pqcsign_src_file" "$dst_variation_dir/apps/pqcsign.c"


            elif [ "$util_flag" == "restore" ]; then
                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/apps/pqcsign.c"
                mv "$make_temp_backup_file" "$make_default_filepath"

            fi
            ;;

        "MIRA")

            # Set the filepaths for the files being worked with
            make_default_filepath="$dst_variation_dir/Makefile"
            make_main_backup_file="$main_alg_backup/Makefile_$current_variation"
            make_temp_backup_file="$dst_variation_dir/temp_make_copy"

            # Copy or restore modified source files for MIRA algorithm
            if [ "$util_flag" == "copy" ]; then

                # Ensure the default source code is present for makefile
                ensure_default_src_files "$dst_variation_dir" "$make_default_filepath" "$make_main_backup_file" "$make_temp_backup_file"


                # Make temp copy of original makefile and copy over modified makefile
                cp "$make_default_filepath" "$make_main_backup_file" 
                cp "$make_default_filepath" "$make_temp_backup_file" # temp stored in working_dir

                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_$current_variation" "$make_default_filepath"
                cp "$pqcsign_src_file" "$dst_variation_dir/src/pqcsign.c"


            elif [ "$util_flag" == "restore" ]; then
                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/src/pqcsign.c"
                mv "$make_temp_backup_file" "$make_default_filepath"

            fi
            ;;

        "PERK")

            # Set the filepaths for the files being worked with
            make_default_filepath="$dst_variation_dir/Makefile"
            make_main_backup_file="$main_alg_backup/Makefile_$current_variation"
            make_temp_backup_file="$dst_variation_dir/temp_make_copy"

            # Copy or restore modified source files for PERK algorithm
            if [ "$util_flag" == "copy" ]; then

                # Ensure the default source code is present for makefile
                ensure_default_src_files "$dst_variation_dir" "$make_default_filepath" "$make_main_backup_file" "$make_temp_backup_file"

                # Make temp copy of original makefile and copy over modified makefile
                cp "$make_default_filepath" "$make_main_backup_file" 
                cp "$make_default_filepath" "$make_temp_backup_file" # temp stored in working_dir

                
                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_$current_variation" "$make_default_filepath"
                cp "$pqcsign_src_file" "$dst_variation_dir/src/pqcsign.c"


            elif [ "$util_flag" == "restore" ]; then
                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/src/pqcsign.c"
                mv "$make_temp_backup_file" "$make_default_filepath"

            fi
            ;;

        "RYDE")

            # Set the filepaths for the files being worked with
            make_default_filepath="$dst_variation_dir/Makefile"
            make_main_backup_file="$main_alg_backup/Makefile_$current_variation"
            make_temp_backup_file="$dst_variation_dir/temp_make_copy"

            # Copy or restore modified source files for RYDE algorithm
            if [ "$util_flag" == "copy" ]; then

                # Ensure the default source code is present for makefile
                ensure_default_src_files "$dst_variation_dir" "$make_default_filepath" "$make_main_backup_file" "$make_temp_backup_file"

                # Make temp copy of original makefile and copy over modified makefile
                cp "$make_default_filepath" "$make_main_backup_file" 
                cp "$make_default_filepath" "$make_temp_backup_file" # temp stored in working_dir

                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_$current_variation" "$make_default_filepath"
                cp "$pqcsign_src_file" "$dst_variation_dir/src/pqcsign.c"


            elif [ "$util_flag" == "restore" ]; then
                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/src/pqcsign.c"
                mv "$make_temp_backup_file" "$make_default_filepath"

            fi
            ;;

        "UOV")
            # NOTE - variation dir is just reference source code directory as modded source files are stored there, not in the variation directories

            # Set the filepaths for the files being worked with
            make_default_filepath="$dst_variation_dir/Makefile"
            make_main_backup_file="$main_alg_backup/Makefile_uov"
            make_temp_backup_file="$dst_variation_dir/temp_make_copy"

            # Copy or restore modified source files for UOV algorithm
            if [ "$util_flag" == "copy" ]; then

                # Ensure the default source code is present for makefile
                ensure_default_src_files "$dst_variation_dir" "$make_default_filepath" "$make_main_backup_file" "$make_temp_backup_file"

                # Make temp copy of original makefile and copy over modified makefile
                cp "$make_default_filepath" "$make_main_backup_file" 
                cp "$make_default_filepath" "$make_temp_backup_file" # temp stored in working_dir

                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_uov" "$dst_variation_dir/Makefile"
                cp "$pqcsign_src_file" "$dst_variation_dir/pqcsign.c"


            elif [ "$util_flag" == "restore" ]; then

                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/pqcsign.c"
                mv "$make_temp_backup_file" "$make_default_filepath"

            fi
            ;;

        "MAYO")

            # Set the filepaths for the files being worked with
            make_default_filepath="$dst_variation_dir/apps/CMakeLists.txt"
            make_main_backup_file="$main_alg_backup/CMakeLists_$current_variation.txt"
            make_temp_backup_file="$dst_variation_dir/apps/temp_cmakelists_copy"

            # Copy or restore modified source files for SQIsign algorithm
            if [ "$util_flag" == "copy" ]; then

                # Ensure the default source code is present for makefile
                ensure_default_src_files "$dst_variation_dir/apps" "$make_default_filepath" "$make_main_backup_file" "$make_temp_backup_file"

                # Make temp copy of original makefile and copy over modified makefile
                cp "$make_default_filepath" "$make_main_backup_file" 
                cp "$make_default_filepath" "$make_temp_backup_file" # temp stored in working_dir
                
                # Copy over modified files to source code directories
                cp "$modified_files_path/CMakeLists_MAYO.txt" "$make_default_filepath"
                cp "$pqcsign_src_file" "$dst_variation_dir/apps/pqcsign.c"


            elif [ "$util_flag" == "restore" ]; then
                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/apps/pqcsign.c"
                mv "$make_temp_backup_file" "$make_default_filepath"

            fi
            ;;

        "VOX")
            # NOTE - variation dir is just reference source code directory as partitions are set at compile time by passed argument

            # Set the filepaths for the files being worked with
            make_default_filepath="$dst_variation_dir/Makefile"
            make_main_backup_file="$main_alg_backup/Makefile_VOX"
            make_temp_backup_file="$dst_variation_dir/temp_make_copy"

            # Copy or restore modified source files for VOX algorithm
            if [ "$util_flag" == "copy" ]; then

                # Ensure the default source code is present for makefile
                ensure_default_src_files "$dst_variation_dir" "$make_default_filepath" "$make_main_backup_file" "$make_temp_backup_file"

                # Make temp copy of original makefile and copy over modified makefile
                cp "$make_default_filepath" "$make_main_backup_file" 
                cp "$make_default_filepath" "$make_temp_backup_file" # temp stored in working_dir

                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_VOX" "$dst_variation_dir/Makefile"
                cp "$pqcsign_src_file" "$dst_variation_dir/pqcsign.c"


            elif [ "$util_flag" == "restore" ]; then

                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/pqcsign.c"
                mv "$api_temp_backup_file" "$api_default_filepath"
                mv "$make_temp_backup_file" "$make_default_filepath"

            fi
            ;;

        "TUOV")

            # NOTE - variation dir is just reference source code directory as modded source files are stored there, not in the variation directories

            # Set the filepaths for the files being worked with
            make_default_filepath="$dst_variation_dir/Makefile"
            make_main_backup_file="$main_alg_backup/Makefile_TUOV"
            make_temp_backup_file="$dst_variation_dir/temp_make_copy"

            # Copy or restore modified source files for UOV algorithm
            if [ "$util_flag" == "copy" ]; then

                # Ensure the default source code is present for makefile
                ensure_default_src_files "$dst_variation_dir" "$make_default_filepath" "$make_main_backup_file" "$make_temp_backup_file"

                # Make temp copy of original makefile and copy over modified makefile
                cp "$make_default_filepath" "$make_main_backup_file" 
                cp "$make_default_filepath" "$make_temp_backup_file" # temp stored in working_dir

                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_TUOV" "$dst_variation_dir/Makefile"
                cp "$pqcsign_src_file" "$dst_variation_dir/pqcsign.c"


            elif [ "$util_flag" == "restore" ]; then

                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/pqcsign.c"
                mv "$make_temp_backup_file" "$make_default_filepath"

            fi
            ;;

        "AIMer")

            # Set the filepaths for the files being worked with
            make_default_filepath="$dst_variation_dir/Makefile"
            make_main_backup_file="$main_alg_backup/Makefile_$current_variation"
            make_temp_backup_file="$dst_variation_dir/temp_make_copy"

            api_default_filepath="$dst_variation_dir/api.h"
            api_main_backup_file="$main_alg_backup/api_$current_variation.h"
            api_temp_backup_file="$dst_variation_dir/temp_api_copy.h"

            # Copy or restore modified source files for AIMer algorithm
            if [ "$util_flag" == "copy" ]; then

                # Ensure the default source code is present for makefile
                ensure_default_src_files "$dst_variation_dir" "$make_default_filepath" "$make_main_backup_file" "$make_temp_backup_file"
                ensure_default_src_files "$dst_variation_dir" "$api_default_filepath" "$api_main_backup_file" "$api_temp_backup_file"

                # Make temp copy of original makefile and api.h file and copy over modified makefile
                cp "$make_default_filepath" "$make_main_backup_file" 
                cp "$make_default_filepath" "$make_temp_backup_file" # temp stored in working_dir
                cp "$api_default_filepath" "$api_main_backup_file"
                cp "$api_default_filepath" "$api_temp_backup_file" # temp stored in working_dir

                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_$current_variation" "$make_default_filepath"
                cp "$modified_files_path/api_$current_variation.h" "$api_default_filepath"
                cp "$pqcsign_src_file" "$dst_variation_dir/pqcsign.c"

            elif [ "$util_flag" == "restore" ]; then

                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/pqcsign.c"
                mv "$make_temp_backup_file" "$make_default_filepath"
                mv "$api_temp_backup_file" "$api_default_filepath"

            fi
            ;;

        "HAETAE")

            # Set the filepaths for the files being worked with
            make_default_filepath="$dst_variation_dir/CMakeLists.txt"
            make_main_backup_file="$main_alg_backup/CMakeLists_$current_alg.txt"
            make_temp_backup_file="$dst_variation_dir/temp_cmakelists_copy"

            api_default_filepath="$dst_variation_dir/kat/api.h"
            api_main_backup_file="$main_alg_backup/api_$current_alg.h"
            api_temp_backup_file="$dst_variation_dir/kat/temp_api_copy.h"

            api_src_default_filepath="$dst_variation_dir/kat/api.c"

            # Copy or restore modified source files for HAETAE algorithm
            if [ "$util_flag" == "copy" ]; then

                # Ensure the default source code is present for makefile
                ensure_default_src_files "$dst_variation_dir" "$make_default_filepath" "$make_main_backup_file" "$make_temp_backup_file"
                ensure_default_src_files "$dst_variation_dir/kat" "$api_default_filepath" "$api_main_backup_file" "$api_temp_backup_file"
                
                if [ -f "$api_src_default_filepath" ]; then
                    rm -f "$api_src_default_filepath"
                fi

                # Make temp copy of original makefile and api.h file and copy over modified makefile
                cp "$make_default_filepath" "$make_main_backup_file" 
                cp "$make_default_filepath" "$make_temp_backup_file" # temp stored in working_dir
                cp "$api_default_filepath" "$api_main_backup_file"
                cp "$api_default_filepath" "$api_temp_backup_file" # temp stored in working_dir

                # Copy over modified files to source code directories
                cp "$modified_files_path/CMakeLists_$current_alg.txt" "$make_default_filepath"
                cp "$modified_files_path/api_HAETAE.h" "$api_default_filepath"
                cp "$modified_files_path/api_HAETAE.c" "$api_src_default_filepath"
                cp "$pqcsign_src_file" "$dst_variation_dir/src/pqcsign.c"

            elif [ "$util_flag" == "restore" ]; then

                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/src/pqcsign.c" && rm -f "$api_src_default_filepath"
                mv "$make_temp_backup_file" "$make_default_filepath"
                mv "$api_temp_backup_file" "$api_default_filepath"

            fi
            ;;

        "LESS")

            # NOTE - LESS does not have a default CMakelists.txt file, so a variation of the one included in the additional implementation is used

            # Set the filepaths for the files being worked with
            make_default_filepath="$dst_variation_dir/CMakeLists.txt"

            # Copy or restore modified source files for LESS algorithm
            if [ "$util_flag" == "copy" ]; then

                # Remove any CMakelists.txt file if it exists as there is no default one
                if [ -f "$make_default_filepath" ]; then
                    rm -f "$make_default_filepath"
                fi

                # Remove pqcsign source file if it exists
                if [ -f "$dst_variation_dir/lib/pqcsign.c" ]; then
                    rm -f "$dst_variation_dir/lib/pqcsign.c"
                fi

                # Copy over modified files to source code directories
                cp "$modified_files_path/CMakeLists_LESS.txt" "$make_default_filepath"
                cp "$pqcsign_src_file" "$dst_variation_dir/lib/pqcsign.c"

            elif [ "$util_flag" == "restore" ]; then

                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/lib/pqcsign.c"

            fi
            ;;

        "MiRitH")

            # Set the filepaths for the files being worked with
            main_make_default_filepath="$dst_variation_dir/Makefile"
            nist_make_default_filepath="$dst_variation_dir/nist/Makefile"

            main_make_main_backup_file="$main_alg_backup/Makefile_$current_variation"
            nist_make_main_backup_file="$main_alg_backup/nist/Makefile_$current_variation"

            main_make_temp_backup_file="$dst_variation_dir/temp_make_copy"
            nist_make_temp_backup_file="$dst_variation_dir/nist/temp_make_copy"

            # Copy or restore modified source files for MiRitH algorithm
            if [ "$util_flag" == "copy" ]; then

                # Ensure the default source code is present for makefile
                ensure_default_src_files "$dst_variation_dir" "$main_make_default_filepath" "$main_make_main_backup_file" "$main_make_temp_backup_file"
                ensure_default_src_files "$dst_variation_dir/nist" "$nist_make_default_filepath" "$nist_make_main_backup_file" "$nist_make_temp_backup_file"

                # Make temp copy of original makefile and copy over modified makefile
                cp "$main_make_default_filepath" "$main_make_main_backup_file" 
                cp "$main_make_default_filepath" "$main_make_temp_backup_file" # temp stored in working_dir

                cp "$nist_make_default_filepath" "$nist_make_main_backup_file" 
                cp "$nist_make_default_filepath" "$nist_make_temp_backup_file" # temp stored in working_dir

                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_main_MiRitH" "$main_make_default_filepath"
                cp "$modified_files_path/Makefile_nist_MiRitH" "$nist_make_default_filepath"
                cp "$pqcsign_src_file" "$dst_variation_dir/nist/pqcsign.c"

            elif [ "$util_flag" == "restore" ]; then

                # Restore default source files
                rm -f "$main_make_default_filepath" && rm -f "$nist_make_default_filepath" && rm -f "$dst_variation_dir/nist/pqcsign.c"
                mv "$main_make_temp_backup_file" "$main_make_default_filepath"
                mv "$nist_make_temp_backup_file" "$nist_make_default_filepath"

            fi

            ;;

        *)

            # Set the filepaths for the files being worked with
            make_main_backup_file="$main_alg_backup/Makefile_$current_variation"
            make_temp_backup_file="$dst_variation_dir/temp_make_copy"

            # Check if current algorithm is DME_Sign and SNOVA as it has a different makefile name format
            if [ "$current_alg" == "DME_Sign" ] || [ "$current_alg" == "SNOVA" ]; then 
                make_default_filepath="$dst_variation_dir/makefile"
            else
                make_default_filepath="$dst_variation_dir/Makefile"
            fi

            # Copy or restore modified source files for all other algorithms
            if [ "$util_flag" == "copy" ]; then

                # Make temp copy of original makefile and copy over modified makefile if it exists
                if [ -f "$make_default_filepath" ]; then

                    # Only copy makefile if it is not for Raccoon as default source code does not contain one
                    if [ "$current_alg" != "Raccoon" ]; then
                        # Ensure the default source code is present for makefile
                        ensure_default_src_files "$dst_variation_dir" "$make_default_filepath" "$make_main_backup_file" "$make_temp_backup_file"

                        # Make temp copy of original makefile and copy over modified makefile
                        cp "$make_default_filepath" "$make_main_backup_file" 
                        mv "$make_default_filepath" "$make_temp_backup_file" # temp stored in working_dir

                    fi

                fi

                # Copy over modified files to source code directories
                cp "$modified_files_path/Makefile_$current_variation" "$make_default_filepath"
                cp "$pqcsign_src_file" "$dst_variation_dir/pqcsign.c"


            elif [ "$util_flag" == "restore" ]; then
            
                # Restore default source files
                rm -f "$make_default_filepath" && rm -f "$dst_variation_dir/pqcsign.c"

                # Only restore makefile if it is not for Raccoon as default source code does not contain one
                if [ "$current_alg" != "Raccoon" ]; then
                    mv "$make_temp_backup_file" "$make_default_filepath"
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
    main_backup_dir="$root_dir/main_default_src_backup"

    # Check arguments passed
    if [ "$util_flag" != "copy" ] && [ "$util_flag" != "restore" ] || [ -z "$current_alg" ] || [ -z "$dst_variation_dir" ] || [ -z "$current_variation" ]; then
        echo "Invalid arguments passed, arguments passed: $@"
        exit 1
    fi

    # Copy or restore passed on util flag passed
    copy_modified_src_files $util_flag $current_alg $dst_variation_dir $current_variation

}
main "$@"