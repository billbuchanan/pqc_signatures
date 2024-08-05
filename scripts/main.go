package main

import (
	"bufio"
	"encoding/csv"
	"fmt"
	"io/fs"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"time"
)

// Workaround in project dir
type PathVariables struct {
	RootDir          string
	SrcDir           string
	BinDir           string
	TestDataDir      string
	ResultsDir       string
	AlgsListDir      string
	AlgVariationsDir string
}

type ConfigParams struct {
	HUFU    bool
	SNOVA   bool
	NumRuns uint8
}

type AlgorithmVariationMessage struct {
	Keygen         string
	Sign           string
	Verify         string
	PrivateKeySize uint
	PublicKeySize  uint
	SignatureSize  uint
}

// Used for internal channel communications
// :Algorithm mame - [key -> value] :Algorithm output data
type AlgorithmMessage map[string][]byte

const HUFUWarningMessage = "WARNGING! - HUFU benchmarking takes a considerable amount of time to complete, even on a high performance machine"
const HUFUSuggestMessage = "Would you like to include HUFU in the benchmarking? (y/n): "

const SNOVAWarningMessage = "WARNGING! - SNOVA benchmarking takes a considerable amount of time to complete, even on a high performance machine"
const SNOVASuggestMessage = "Would you like to include SNOVA in the benchmarking? (y/n): "

const NumRunsMessage = "Enter the number of runs to be performed: "

const sep = string(os.PathSeparator)

// Set absolute path to root folder of the project (run only inside of scripts/)
func NewPathVariables() (*PathVariables, error) {
	wd, err := os.Getwd()
	if err != nil {
		fmt.Printf("Working dir error %s \n", err)
		return &PathVariables{}, err
	}
	root := filepath.Dir(wd)
	pvb := &PathVariables{
		RootDir:          root,
		SrcDir:           root + sep + "src",
		BinDir:           root + sep + "bin",
		TestDataDir:      root + sep + "test_data",
		ResultsDir:       root + sep + "test_data" + sep + "results",
		AlgsListDir:      root + sep + "test_data" + sep + "sig_algs_list",
		AlgVariationsDir: root + sep + "test_data" + sep + "alg_variation_lists",
	}
	return pvb, nil
}

// Extraction of algorithms and their versions from .txt files in test_data/alg_variation_lists dir
func NewAlgVariationArrays(wd string) (map[string][]string, int) {
	// :map[string][]string - dictionary of algorithm names with their versions
	// :int - total amount of algorithm versions for a buffer size
	var totalAmount int = 0

	res := make(map[string][]string)
	err := filepath.Walk(wd, func(path string, info fs.FileInfo, err error) error {
		if err != nil {
			fmt.Printf("Working dir error %s \n", err)
			return err
		}

		file, err := os.Open(path)
		if err != nil {
			fmt.Printf("Reading file in dir error %s \n", err)
			return err
		}
		defer file.Close()

		var lines []string
		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			lines = append(lines, scanner.Text())
		}
		// :TODO Ascon_sign insted of Ascon_Sign?
		res[info.Name()[:len(info.Name())-15]] = lines
		totalAmount += len(lines)
		return nil
	})
	if err != nil {
		fmt.Printf("Working dir error %s \n", err)
	}
	return res, totalAmount
}

// Set config of heavy params
func setConfig() *ConfigParams {
	var message string
	var nums uint8
	cfg := ConfigParams{}
	fmt.Println(HUFUWarningMessage)
	fmt.Println(HUFUSuggestMessage)
	_, err := fmt.Scan(&message)
	if err != nil || (message != "y" && message != "n") {
		fmt.Printf("Invalid inpuit  %s \n", err)
		panic(err)
	}
	if message == "y" {
		cfg.HUFU = true
	} else if message == "n" {
		cfg.HUFU = false
	}

	fmt.Println(SNOVAWarningMessage)
	fmt.Println(SNOVASuggestMessage)
	_, err1 := fmt.Scan(&message)
	if err1 != nil || (message != "y" && message != "n") {
		fmt.Printf("Invalid inpuit  %s \n", err1)
		panic(err1)
	}
	if message == "y" {
		cfg.SNOVA = true
	} else if message == "n" {
		cfg.SNOVA = false
	}

	fmt.Println(NumRunsMessage)
	_, err2 := fmt.Scan(&nums)
	if err2 != nil {
		fmt.Printf("Invalid inpuit  %s \n", err2)
		panic(err2)
	}
	cfg.NumRuns = nums

	return &cfg
}

// Delete folder if its exists
func deleteExistingFolder(wd string, name string) {
	err := filepath.Walk(wd, func(path string, info fs.FileInfo, err error) error {
		if err != nil {
			fmt.Printf("Working dir error %s \n", err)
			return err
		}
		if info.Name() == name {
			os.RemoveAll(info.Name())
			fmt.Printf("Previous %s folder was deleted \n", name)
		}
		return nil
	})
	if err != nil {
		fmt.Printf("Working dir error %s \n", err)
	}

}

// Convertion of output
func extractRelevantContent(input string, excludeKey string) string {
	// :input - working string
	// :excludeKey - string above exclusion
	lines := strings.Split(input, "\n")

	// Iterate through the lines to find key to exclude
	for i, line := range lines {
		if strings.HasPrefix(line, excludeKey) {
			return strings.Join(lines[:i+1], "\n")
		}
	}
	return input
}

// handleOutputChan-writing from a buff chan into txt and csv files
func handleOutputChan(f *os.File, f1 *os.File, output <-chan AlgorithmMessage, stopchan <-chan struct{}) {
	defer f1.Close()
	defer f.Close()
	wCSV := csv.NewWriter(f1)
	for {
		select {
		case message, ok := <-output:
			if !ok {
				return
			}
			for _, data := range message {
				res := extractRelevantContent(string(data), "Verify")
				_, err := f.WriteString(res + "\n")
				if err != nil {
					fmt.Printf("TXT file error %s \n", err)
					continue
				}

				err1 := wCSV.Write(strings.Split(res, "\n"))
				if err1 != nil {
					fmt.Printf("CSV file error %s \n", err1)
				}
			}
		case <-stopchan:
			wCSV.Flush()
			return
		}
	}
}

// Iterating over the algorithm versions and send command output to the channel
func IterateAndRunAlgVers(BinDir string, k string, v []string, output chan<- AlgorithmMessage) {
	for _, element := range v {
		out, err := exec.Command(filepath.Join(BinDir, k, "pqcsign_"+element)).Output()
		if err != nil {
			fmt.Printf("Command execution error: %s \nOutput:  %s \nAlgorithm: %s \n", err, out, element)
			continue
		}
		output <- AlgorithmMessage{k: out}
	}
}

// PerformBenchmarking iterates over the algorithms and start goroutines for evaluation of each algorithm
func PerformBenchmarking(res map[string][]string, cfg *ConfigParams, pathvars *PathVariables, step int, output chan AlgorithmMessage, wg *sync.WaitGroup) {
	fmt.Printf("Performing step %s \n", strconv.Itoa(step+1))
	for k, v := range res {
		// Heavy algorithms
		if (k == "HuFu" && !cfg.HUFU) || (k == "SNOVA" && !cfg.SNOVA) {
			continue
		}
		wg.Add(1)
		fmt.Printf("Algorithm %s execution started \n", k)
		go func(k string, v []string) {
			defer wg.Done()
			IterateAndRunAlgVers(pathvars.BinDir, k, v, output)
		}(k, v)

	}
}

func main() {
	// Setting up of the WaitGroup to track all the goroutines
	wg := sync.WaitGroup{}
	start := time.Now()
	// Set up the config
	pathvars, err := NewPathVariables()
	if err != nil {
		panic(err)
	}
	res, amount := NewAlgVariationArrays(pathvars.AlgVariationsDir)
	cfg := setConfig()
	// Setting up bufferend channel for communication and stop-signal channel
	output := make(chan (AlgorithmMessage), amount)
	stopchan := make(chan struct{})

	fmt.Println("Started")
	for run := 0; run < int(cfg.NumRuns); run++ {
		file, err := os.Create(pathvars.ResultsDir + sep + "results" + strconv.Itoa(run+1) + "_" + time.Now().Local().Format("20060102") + ".txt")
		file1, err1 := os.Create(pathvars.ResultsDir + sep + "results" + strconv.Itoa(run+1) + "_" + time.Now().Local().Format("20060102") + ".csv")
		if err != nil || err1 != nil {
			fmt.Printf("File error %s \n", err)
			return
		}
		go handleOutputChan(file, file1, output, stopchan)
		PerformBenchmarking(res, cfg, pathvars, run, output, &wg)
	}
	wg.Wait()
	// Send close signals to both channels
	close(output)
	close(stopchan)
	fmt.Printf("Finished in %s\n", time.Since(start))
}
