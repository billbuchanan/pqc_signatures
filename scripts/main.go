package main

import (
	"bufio"
	"encoding/csv"
	"encoding/json"
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

type AlgorithmDetails struct {
	Keygen         string
	Sign           string
	Verify         string
	PrivateKeySize uint
	PublicKeySize  uint
	SignatureSize  uint
}

type AlgorithmVersion map[string]AlgorithmDetails
type JSONChannelMessage map[string][]AlgorithmVersion

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
	var algVersAmount int = 0

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
		algVersAmount += len(lines)
		return nil
	})
	if err != nil {
		fmt.Printf("Working dir error %s \n", err)
	}
	return res, algVersAmount
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
func prepareJSONFormat(data []byte) AlgorithmVersion {
	lines := strings.Split(string(data), "\n")
	var algName string
	algStruct := AlgorithmDetails{}
	result := make(AlgorithmVersion)

	for _, line := range lines {
		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}

		parts := strings.SplitN(line, ":", 2)
		if len(parts) < 2 {
			continue
		}

		key := strings.TrimSpace(parts[0])
		value := strings.TrimSpace(parts[1])

		// remove cycles word
		value = strings.Replace(value, " cycles", "", 1)
		switch key {
		case "Algorithm":
			algName = value
		case "Private key size":
			if size, err := strconv.Atoi(value); err == nil {
				algStruct.PrivateKeySize = uint(size)
			}
		case "Public key size":
			if size, err := strconv.Atoi(value); err == nil {
				algStruct.PublicKeySize = uint(size)
			}
		case "Signature size":
			if size, err := strconv.Atoi(value); err == nil {
				algStruct.SignatureSize = uint(size)
			}
		case "Keygen":
			algStruct.Keygen = value
		case "Sign":
			algStruct.Sign = value
		case "Verify":
			algStruct.Verify = value
		}
	}

	if algName != "" {
		result[algName] = algStruct
	}
	return result

}
func handleJSONChan(f *os.File, jsonchan <-chan JSONChannelMessage, stopchan <-chan struct{}, jsonChannelCap int) {
	defer f.Close()
	enc := json.NewEncoder(f)
	enc.SetIndent("", " ")
	f.WriteString("{\n")
	for {
		select {
		case message, ok := <-jsonchan:
			if !ok {
				return
			}
			v, err := json.MarshalIndent(&message, "", " ")
			if err != nil {
				fmt.Printf("JSON file error %s \n", err)
			}
			f.Write(v[1 : len(v)-1])
			jsonChannelCap -= 1
			if jsonChannelCap > 2 {
				f.Write([]byte(","))
			}

		case <-stopchan:
			f.Write([]byte("\n}"))
			return
		}
	}
}

// handleOutputChan-writing from a buff chan into txt and csv files
func handleOutputChan(f *os.File, f1 *os.File, output <-chan []byte, stopchan <-chan struct{}) {
	defer f1.Close()
	defer f.Close()
	wCSV := csv.NewWriter(f1)
	for {
		select {
		case line, ok := <-output:
			if !ok {
				return
			}
			res := extractRelevantContent(string(line), "Verify")
			_, err := f.WriteString(res + "\n")
			if err != nil {
				fmt.Printf("TXT file error %s \n", err)
				continue
			}
			err1 := wCSV.Write(strings.Split(res, "\n"))
			if err1 != nil {
				fmt.Printf("CSV file error %s \n", err1)
				continue
			}
		case <-stopchan:
			wCSV.Flush()
			return
		}
	}
}

// Iterating over the algorithm versions and send command output to the channel
func IterateOverAlgsAndRunAlgVers(BinDir string, k string, v []string, output chan<- []byte, jsonchan chan<- JSONChannelMessage) {
	var algVersionsArray []AlgorithmVersion
	for _, element := range v {
		out, err := exec.Command(filepath.Join(BinDir, k, "pqcsign_"+element)).Output()
		if err != nil {
			fmt.Printf("Command execution error: %s \nOutput:  %s \nAlgorithm: %s \n", err, out, element)
			continue
		}
		output <- out

		algVersionsArray = append(algVersionsArray, prepareJSONFormat(out))
	}
	jsonchan <- JSONChannelMessage{k: algVersionsArray}
	algVersionsArray = nil
}

// O(n^2)
// PerformBenchmarking iterates over the algorithms and start goroutines for evaluation of each algorithm
func PerformBenchmarking(res map[string][]string, cfg *ConfigParams, pathvars *PathVariables, step int, output chan []byte, jsonchan chan JSONChannelMessage, wg *sync.WaitGroup) {
	fmt.Printf("Performing step %s \n", strconv.Itoa(step+1))
	for k, v := range res {
		// Heavy algorithms=SIGNIFICANT AMOUNT OF TIME! Half an hour? Halndilng of heavy algorithms
		if (k == "HuFu" && !cfg.HUFU) || (k == "SNOVA" && !cfg.SNOVA) {
			continue
		}
		wg.Add(1)
		fmt.Printf("Algorithm %s execution started \n", k)
		go func(k string, v []string) {
			defer wg.Done()
			IterateOverAlgsAndRunAlgVers(pathvars.BinDir, k, v, output, jsonchan)
		}(k, v)

	}
}

func main() {

	// Start the test timer to measure the time of the whole benchmarking
	start := time.Now()

	// Set up the config
	pathvars, err := NewPathVariables()
	if err != nil {
		panic(err)
	}
	res, amount := NewAlgVariationArrays(pathvars.AlgVariationsDir)
	cfg := setConfig()

	// NOTE - The channels had to be changed to start and stop for each run, due to the script starting the second run before the first one is finished
	// this caused the script to exceed the systems resources and crash. Will need to review this further.

	fmt.Println("Started")
	for run := 0; run < int(cfg.NumRuns); run++ {

		// Creating the output files
		file, err := os.Create(pathvars.ResultsDir + sep + "results" + strconv.Itoa(run+1) + "_" + time.Now().Local().Format("20060102") + ".txt")
		file1, err1 := os.Create(pathvars.ResultsDir + sep + "results" + strconv.Itoa(run+1) + "_" + time.Now().Local().Format("20060102") + ".csv")
		file2, err2 := os.Create(pathvars.ResultsDir + sep + "results" + strconv.Itoa(run+1) + "_" + time.Now().Local().Format("20060102") + ".json")
		if err != nil || err1 != nil || err2 != nil {
			fmt.Printf("File error %s \n", err)
			return
		}

		// Setting up of the WaitGroup to track all the goroutines for the current run
		wg := sync.WaitGroup{}
		handlerWG := sync.WaitGroup{}

		// Starting the channels for the current run

		// Setting up bufferend channel (length is amount of all the algorithms with their versions) for communication and stop-signal channel
		output := make(chan []byte, amount)
		stopchan := make(chan struct{})
		// Setting up bufferend channel (length is amount of all the algorithms excluding their versions) for json writing channel
		jsonchan := make(chan JSONChannelMessage, len(res))

		// Starting the goroutines for the text and csv file handling
		handlerWG.Add(1)
		go func(file *os.File, file1 *os.File, output <-chan []byte, stopchan <-chan struct{}) {
			defer handlerWG.Done()
			handleOutputChan(file, file1, output, stopchan)
		}(file, file1, output, stopchan)

		// Starting the goroutine for the json file handling
		handlerWG.Add(1)
		go func(file2 *os.File, jsonchan <-chan JSONChannelMessage, stopchan <-chan struct{}) {
			defer handlerWG.Done()
			handleJSONChan(file2, jsonchan, stopchan, len(res))
		}(file2, jsonchan, stopchan)

		// Perform the benchmarking for the current run
		PerformBenchmarking(res, cfg, pathvars, run, output, jsonchan, &wg)

		wg.Wait()
		// Send close signals to both channels
		close(stopchan)
		handlerWG.Wait()
		close(output)
		close(jsonchan)

	}

	fmt.Printf("Finished in %s\n", time.Since(start))
}
