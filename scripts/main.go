package main

import (
	"bufio"
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
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

type AlgVariationArrays struct {
	BISCUIT         chan map[string][]string
	CROSS           chan map[string][]string
	FAEST           chan map[string][]string
	FULEECA         chan map[string][]string
	PQSIGRM         chan map[string][]string
	SPHINCS_ALPHA   chan map[string][]string
	SQI             chan map[string][]string
	UOV             chan map[string][]string
	MED             chan map[string][]string
	HAWK            chan map[string][]string
	EHTV3V4         chan map[string][]string
	HUFU            chan map[string][]string
	THREE_WISE      chan map[string][]string
	MIRA            chan map[string][]string
	PERK            chan map[string][]string
	RYDE            chan map[string][]string
	SDITH_HYPERCUBE chan map[string][]string
	ASCON_SIGN      chan map[string][]string
	MAYO            chan map[string][]string
	EMLE_SIG_2_0    chan map[string][]string
	DME_SIGN        chan map[string][]string
	XIFRAT1_SIGN    chan map[string][]string
	VOX             chan map[string][]string
	TUOV            chan map[string][]string
	PROV            chan map[string][]string
	QR_UOV          chan map[string][]string
	SNOVA           chan map[string][]string
	HPPC            chan map[string][]string
	ALTEQ           chan map[string][]string
}

var algs map[int][]string

func NewAlgVariationArrays(wd string) map[string][]string {
	res := make(map[string][]string)
	err := filepath.Walk(wd, func(path string, info fs.FileInfo, err error) error {
		if err != nil {
			fmt.Printf("Working dir error %s", err)
			return err
		}

		file, err := os.Open(path)
		if err != nil {
			fmt.Printf("Reading file in dir error %s", err)
			return err
		}
		defer file.Close()

		var lines []string
		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			lines = append(lines, scanner.Text())
		}

		// Print lines to verify
		fmt.Printf("Lines in file %s:\n", info.Name())
		for _, line := range lines {
			fmt.Println(line)
		}
		res[info.Name()[:len(info.Name())-4]] = lines
		return nil
	})
	if err != nil {
		fmt.Printf("Working dir error %s", err)
	}
	return res
}

func NewPathVariables() (*PathVariables, error) {
	// Set absolute path to root folder of the project (run only inside of scripts/)
	wd, err := os.Getwd()
	if err != nil {
		fmt.Printf("Working dir error %s \n", err)
		return &PathVariables{}, err
	}
	sep := string(os.PathSeparator)
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

func deleteExistingFolder(wd string, name string) {
	// Delete folder if its exists
	err := filepath.Walk(wd, func(path string, info fs.FileInfo, err error) error {
		if err != nil {
			fmt.Printf("Working dir error %s", err)
			return err
		}
		if info.Name() == name {
			os.RemoveAll(info.Name())
			fmt.Printf("Previous %s folder was deleted", name)
		}
		return nil
	})
	if err != nil {
		fmt.Printf("Working dir error %s", err)
	}

}
func main() {
	pathvars, err := NewPathVariables()
	if err != nil {
		panic(err)
	}
	res := NewAlgVariationArrays(pathvars.AlgVariationsDir)
	for k, v := range res {
		fmt.Println(k, "value is", v)
	}
}
