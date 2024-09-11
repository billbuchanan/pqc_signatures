"""
This script is used to parse the results of the test runs JSON files and generate formatted CSV and xlsx files for the results.
The script also handles the generation of the averages for the performance results across all runs for each of the test groups.
"""

import os
import sys
import json
import shutil
import pandas as pd

#---------------------------------------------------------------------------------------------------
def get_repo_root_dir():
    """ Function for setting up the basic global variables for the test suite. This includes setting the root directory
        and the global library paths for the test suite. The function establishes the root path by determining the path of the script and 
        using this, determines the root directory of the project """

    # Get the script dir location, set current directory, and set the marker filename
    script_dir = os.path.dirname(os.path.abspath(__file__))
    current_dir = script_dir
    marker_filename = ".repo_root_dir_marker.tmp"

    # Loop until the project's root directory is found or the system root directory is reached
    while True:

        # Check if the marker file is present in the current directory and break if found
        marker_path = os.path.join(current_dir, marker_filename)

        if os.path.isfile(marker_path):
            root_dir = current_dir
            return root_dir

        # Move up one directory and check again for 
        current_dir = os.path.dirname(current_dir)

        # If the root directory is reached and the file is not found, exit the script
        if current_dir == "/":
            print("Root directory path file not present, please ensure the path is correct and try again.")
            sys.exit(1)

#---------------------------------------------------------------------------------------------------
def setup_base_env():
    """ Function for setting up the base environment for the parser script. This includes setting up the paths for the various directories
        used in the parsing process. The function also determines the test batches that are present in the results directory and returns
        these as a dictionary containing the test batches and all the results files associated with it"""

    # Get the root directory of the repository
    root_dir = get_repo_root_dir()

    # Create the central paths dictionary
    paths = {
        "root_dir": root_dir,
        "test_data_dir": os.path.join(root_dir, "test_data"),
        "alg_variation_lists_dir": os.path.join(root_dir, "test_data", "alg_variation_lists"),
        "sig_alg_lists_dir": os.path.join(root_dir, "test_data", "sig_alg_lists"),
        "results_dir": os.path.join(root_dir,"test_data", "results"),
        "parsed_results_dir": os.path.join(root_dir, "test_data", "parsed_results"),
    }

    # Create the parsed results directory if it does not exist
    if not os.path.isdir(paths["parsed_results_dir"]):
        os.makedirs(paths["parsed_results_dir"])
    
    else:
        shutil.rmtree(paths["parsed_results_dir"])
        os.makedirs(paths["parsed_results_dir"])
    
    # Get all the files present in the results directory
    results_dir_listing = os.listdir(paths["results_dir"])
    results_dir_listing.sort()
    
    # Determine the results files to be processed that are in results directory
    test_batches = {}
    result_file_present_flag = False
    file_found_flag = False

    for file in results_dir_listing:

        # Check if the current file meets the requirements for processing
        if "results" in file and ".json" in file:

            # Get the test batch based on the date in the filename
            results_date = file.split("_")[1].split(".")[0]

            # Add the file to the test batch if it is not already present and increment the run count
            if results_date not in test_batches.keys():
                test_batches[results_date] = [file]

            elif file not in test_batches[results_date]:
                test_batches[results_date].append(file)

    if len(test_batches) != 0:

        return paths, test_batches

    else:
        print("No results files found in the results directory that can be parsed")
        print("please ensure the files are present and try again.")
        sys.exit(1)

#---------------------------------------------------------------------------------------------------
def convert_json_to_formatted_csv(paths, test_group, test_group_name):
    """ Function for converting the JSON results files to formatted CSV files which are 
        easier to work with for the current test group """

    # Import the results from the JSON files for the current test group
    test_results = {}

    for test_file in test_group:

        # Set the filepath for the current results file 
        results_filepath = os.path.join(paths["results_dir"], test_file)

        # Load the JSON data from the results file and add it to the test results dictionary
        with open(results_filepath, "r") as results_file:
            results_data = json.load(results_file)
        
        test_results[test_file] = results_data
    
    # Loop through the results files and convert the JSON data to formatted CSV files
    for file in test_results.keys():

        # Create a new dataframe the results data for the current test run
        results_df = pd.DataFrame(columns=["Algorithm", "Keygen Cycles", "Sign Cycles", "Verify Cycles"])

        # Remove the "alg_" key from the results dictionary
        del test_results[file]["alg_"]

        # Loop through the algorithms and variations and add the results to the dataframe
        for alg_group in test_results[file].keys():

            # Loop through the variations and add the results to the dataframe
            for current_variation_dict in test_results[file][alg_group]:

                # Get the algorithm key, keygen, sign, and verify cycles for the current variation
                algorithm = list(current_variation_dict.keys())[0]
                keygen_cycles = int(current_variation_dict[algorithm]["Keygen"])
                sign_cycles = int(current_variation_dict[algorithm]["Sign"])
                verify_cycles = int(current_variation_dict[algorithm]["Verify"])

                # Create the row for the dataframe and concatenate it to the dataframe
                df_row = [algorithm, keygen_cycles, sign_cycles, verify_cycles]
                results_df.loc[len(results_df)] = df_row

        # Save the results dataframe to a CSV file and xlsx file
        run_num = file.split("_")[0][-1]
        results_df.to_csv(os.path.join(paths["parsed_results_dir"], f"results{run_num}_{test_group_name}.csv"), index=False)
        results_df.to_excel(os.path.join(paths["parsed_results_dir"], f"results{run_num}_{test_group_name}.xlsx"), index=False)

#---------------------------------------------------------------------------------------------------
def average_generator(paths, test_group, test_group_name):
    """ Function for generating the average of the performance results """

    # Get all the algs from the first results file
    first_test_filename = test_group[0].replace(".json", ".csv")
    temp_df = pd.read_csv(os.path.join(paths["parsed_results_dir"], first_test_filename))
    algs = temp_df["Algorithm"].tolist()
    column_names  = temp_df.columns
    temp_df = None
    
    # Create the main dataframe for the averages
    averages_df = pd.DataFrame(columns=column_names)

    # Loop through the algs and calculate the averages for each operation across all runs
    for current_alg in algs:

        # Set the combined operations dataframe to empty for the current algorithm
        combined_operations = pd.DataFrame(columns=column_names)

        # Loop through the runs and combine the operations for the current algorithm
        for run_num in range(1, len(test_group)+1):

            # Read the current results file and get the operations for the current algorithm
            results_file = os.path.join(paths["parsed_results_dir"], f"results{run_num}_{test_group_name}.csv")
            current_results_df = pd.read_csv(results_file)

            # Add the results to the combined operations dataframe depending on whether it is the first run or not
            if run_num == 1:
                combined_operations = current_results_df.loc[current_results_df["Algorithm"] == current_alg]
            else:
                current_results_df = current_results_df.loc[current_results_df["Algorithm"] == current_alg]
                combined_operations = pd.concat([combined_operations, current_results_df], ignore_index=True, sort=False)

        # Set the new row that will be added to the averages dataframe
        new_row = [current_alg]

        # Loop through the operations and calculate the average for each operation
        for operation in column_names[1:]:

            operation_average = combined_operations[operation].mean()
            new_row.append(operation_average)

        # Add the new row to the averages dataframe for the current current_alg
        averages_df.loc[len(averages_df)] = new_row

    # Save the averages dataframe to a CSV file and xlsx file
    averages_df.to_csv(os.path.join(paths["parsed_results_dir"], f"averages_{test_group_name}.csv"), index=False)
    averages_df.to_excel(os.path.join(paths["parsed_results_dir"], f"averages_{test_group_name}.xlsx"), index=False)
    
#---------------------------------------------------------------------------------------------------
def main():
    """ Main function for controlling the parser """

    # Print the parser start message to the terminal
    print("Starting the results parser...")

    # Setup the base environment for the parser
    paths, test_batches = setup_base_env()

    # Loop through and process the various test batches present
    for test_group_name in test_batches.keys():

        # Convert the JSON results to formatted CSV files and generate the averages
        convert_json_to_formatted_csv(paths, test_batches[test_group_name], test_group_name)
        average_generator(paths, test_batches[test_group_name], test_group_name)

    # Print the parser end message to the terminal
    print("Results parser completed successfully")

#------------------------------------------------------------------------------
"""Main boiler plate"""
if __name__ == "__main__":
    main()