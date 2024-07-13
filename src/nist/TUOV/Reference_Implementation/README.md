  
# TUOV signature

This project contains the implementations of the triangular unbalanced oil and vinegar (TUOV) signature system.

## Authors

## Licence

Public domain.  

## Supporting Parameters

| Parameter    | signature size | pk size  | sk size | cpk size | compressed-sk size |  
|:-----------: |:--------------:|--------- |---------|------------|--------------------|
|GF(16),160,64 | 80             |412,160   |350,272  |65,552      | 48                 |
|GF(256),112,44| 112            |278,432   |239,391  |42,608      | 48                 |
|GF(256),184,72| 184            |1,225,440 |1,048,279|186,640     | 48                 |
|GF(256),244,96| 244            |2,869,440 |2,443,711|442,384     | 48                 |


## Instructions for testing, generating KATs and benchmarking

Type **make**    
```
make
```
It will generate an executable **api_test** testing API functions (crypto_keygen(), crypto_sign(), and crypto_verify()).  

### **Options for Parameters:**

The source code of all supporting parameters is stored in subdirectories of the parameter names.
The default setting is compiling **Ip**.  
We use **PROJ** parameters for compiling other parameters.  
For example:  
```
make PROJ=Is_pkc
```
```
make PROJ=III_pkc_skc
```

### **Generating KAT:**
Use the following script:  
```
source ./generate_KAT.sh
```

### **Benchmarking programs**

For benchmarking the implementations, we provide speed test programs for x86 CPUs that use the Time Step Counter (TSC) or the actual cycle counter provided by the Performance Measurement Counters (PMC) to measure performance. To compile the programs run
```sh
make test_speed
```

This produces the executables
```sh
test_speed
```

The default setting is compiling **Ip**.  
We use **PROJ** parameters for compiling other parameters.  
For example:  
```
make test_speed PROJ=Is_pkc
```
```
make test_speed PROJ=III_pkc_skc
```