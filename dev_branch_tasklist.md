#### Short-Term Goals
- [x] Perform basic restructure source directories and project files after branch initialisation

- [x] Review signature scheme source code to identify best method of integration within performance evaluation scripts

- [X] Create functionality to ensure default source code files are always present, even if setup script is stopped halfway through before restoring the default files after the modified version has been implemented.

- [X] Create basic setup script and makefiles to compile various algorithm source code that can later be used within performance measuring scripts

- [ ] Resolve issues with the remaining algorithms which have been marked for team debugging

- [ ] Implement automated compiling and benchmarking for the signature schemes selected for standardisation in the third round of the main NIST competition to compare round 1 signatures against

- [ ] Add functionality and modifications to relevant Makefiles that allow for the flint library to only need downloaded and compiled once rather than multiples times for each algorithm that needs it. 

- [ ] Verify the performance metrics being outputted by the automated benchmarking script against specification papers to ensure projects accuracy.

- [ ] Increase functionality of automated benchmarking script to include handling of old results and allow the user to conduct multiple benchmarking runs.

- [ ] Create Python parsing scripts to grab performance metrics from outputted results txt file and store in CSV format.


#### Long-Term Goals
- [ ] Determine if possible to remove redundant copies of the reference-code and instead create the security variations through combined Makefiles

- [ ] Create a unified testing suite that utilises the source code to perform all algorithm testing in a C script rather than bash scripting

- [ ] Optimise the automated bash scripts within the project to reduce code clutter and improve performance across multiple systems
  
- [ ] Utilise optimised versions of the various submitted algorithm source code files and allow for execution on highly constrained devices