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
		ResultsDir:       root + sep + "test_data" + sep + "results" + time.Now().Local().Format("20060102"),
		AlgsListDir:      root + sep + "test_data" + sep + "sig_algs_list",
		AlgVariationsDir: root + sep + "test_data" + sep + "alg_variation_lists",
	}
	return pvb, nil
}

func NewAlgVariationArrays(wd string) map[string][]string {
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
		return nil
	})
	if err != nil {
		fmt.Printf("Working dir error %s \n", err)
	}
	return res
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

func PerformBenchmarking(res map[string][]string, cfg *ConfigParams, pathvars *PathVariables, step int, wg *sync.WaitGroup) {
	defer wg.Done() // Correctly defer wg.Done()
	for k, v := range res {
		if (k == "HuFu" && cfg.HUFU == false) || (k == "SNOVA" && cfg.SNOVA == false) {
			continue
		}
		for _, element := range v {
			cmd := exec.Command(pathvars.BinDir+sep+k+sep+"pqcsign_"+element, ">>", pathvars.ResultsDir+sep+ResultsFileName+strconv.Itoa(step+1)+".txt")
			if err := cmd.Run(); err != nil {
				fmt.Println(err)
			}
		}
	}
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
func main() {
	var wg sync.WaitGroup
	pathvars, err := NewPathVariables()
	if err != nil {
		panic(err)
	}
	res := NewAlgVariationArrays(pathvars.AlgVariationsDir)
	cfg := Config()

	for run := 0; run < int(cfg.NumRuns); run++ {
		wg.Add(1)
		go PerformBenchmarking(res, cfg, pathvars, run, &wg)
	}
	wg.Wait()
	fmt.Println("Started")
}
