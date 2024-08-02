package main

import (
	"bufio"
	"fmt"
	"io/fs"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"sync"
	"time"
)

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

const ResultsFileName = "sig_speed_results_run"

const HUFUWarningMessage = "WARNGING! - HUFU benchmarking takes a considerable amount of time to complete, even on a high performance machine"
const HUFUSuggestMessage = "Would you like to include HUFU in the benchmarking? (y/n): "

const SNOVAWarningMessage = "WARNGING! - SNOVA benchmarking takes a considerable amount of time to complete, even on a high performance machine"
const SNOVASuggestMessage = "Would you like to include SNOVA in the benchmarking? (y/n): "

const NumRunsMessage = "Enter the number of runs to be performed: "

const sep = string(os.PathSeparator)

func NewPathVariables() (*PathVariables, error) {
	// Set absolute path to root folder of the project (run only inside of scripts/)
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

func NewAlgVariationArrays(wd string) (map[string][]string, int) {
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
		res[info.Name()[:len(info.Name())-15]] = lines
		totalAmount += len(lines)
		return nil
	})
	if err != nil {
		fmt.Printf("Working dir error %s \n", err)
	}
	return res, totalAmount
}

func Config() *ConfigParams {
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

func deleteExistingFolder(wd string, name string) {
	// Delete folder if its exists
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

// handleOutputChan handles the output channel and closes when producer stops
func handleOutputChan(f *os.File, output chan []byte, stopchan chan bool) {
	defer f.Close()
	for {
		select {
		case line, ok := <-output:
			if !ok {
				return
			}
			_, err := f.WriteString(string(line))
			if err != nil {
				fmt.Printf("File error %s \n", err)
				continue
			}
		case <-stopchan:
			return
		}
	}
}

func IterateAndRunAlgVers(BinDir string, k string, v []string, output chan<- ([]byte)) {
	for _, element := range v {
		out, err := exec.Command(filepath.Join(BinDir, k, "pqcsign_"+element)).Output()
		if err != nil {
			fmt.Printf("Command execution error: %s \nOutput:  %s \n", err, out)
			continue
		}
		output <- out
	}
}

// PerformBenchmarking performs the benchmarking
func PerformBenchmarking(res map[string][]string, cfg *ConfigParams, pathvars *PathVariables, step int, output chan ([]byte), wg *sync.WaitGroup) {
	fmt.Printf("Performing step %s \n", strconv.Itoa(step+1))
	for k, v := range res {
		wg.Add(1)
		fmt.Printf("Algorithm %s execution started \n", k)
		go func(k string, v []string) {
			defer wg.Done()
			IterateAndRunAlgVers(pathvars.BinDir, k, v, output)
		}(k, v)

	}
}

func main() {
	wg := sync.WaitGroup{}
	start := time.Now()
	pathvars, err := NewPathVariables()
	if err != nil {
		panic(err)
	}
	res, amount := NewAlgVariationArrays(pathvars.AlgVariationsDir)
	cfg := Config()
	output := make(chan ([]byte), amount)
	stopchan := make(chan bool)
	fmt.Println("Started")
	for run := 0; run < int(cfg.NumRuns); run++ {
		file, err := os.Create(pathvars.ResultsDir + sep + "results" + strconv.Itoa(run+1) + "_" + time.Now().Local().Format("20060102") + ".txt")
		if err != nil {
			fmt.Printf("File error %s \n", err)
			return
		}
		go handleOutputChan(file, output, stopchan)
		PerformBenchmarking(res, cfg, pathvars, run, output, &wg)
	}
	wg.Wait()
	stopchan <- true
	fmt.Printf("Finished in %s\n", time.Since(start))
}
