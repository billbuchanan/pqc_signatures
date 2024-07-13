cmake_minimum_required(VERSION 3.7)

project(CROSS C)
set(CMAKE_C_STANDARD 11)

# set(CC gcc)
set(CC clang)

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wall -pedantic -Wuninitialized -march=native -O3 -g0")

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${SANITIZE}")
message("Compilation flags:" ${CMAKE_C_FLAGS})

# default compilation picks reference codebase
if(NOT DEFINED REFERENCE)
   set(REFERENCE 0)
endif()

set(CSPRNG_ALGO SHAKE_CSPRNG)
set(HASH_ALGO SHA3_HASH)

find_library(KECCAK_LIB keccak)
if(NOT KECCAK_LIB)
 set(STANDALONE_KECCAK 1)
endif()


# selection of specialized compilation units differing between ref and opt
# implementations.
set(REFERENCE_CODE_DIR ../../Reference_Implementation)
set(OPTIMIZED_CODE_DIR ../../Optimized_Implementation)

if(REFERENCE)
message("Compiling portable reference code")
set(SPEC_HEADERS  )
set(SPEC_SOURCES
        ${REFERENCE_CODE_DIR}/lib/aes256.c
)
else()
message("Compiling optimized code")
set(SPEC_HEADERS )
set(SPEC_SOURCES
        ${OPTIMIZED_CODE_DIR}/lib/aes256.c
)
endif()

set(BASE_DIR ${REFERENCE_CODE_DIR})
set(HEADERS
    ${SPEC_HEADERS}
    ${BASE_DIR}/include/api.h
    ${BASE_DIR}/include/aes256.h
    ${BASE_DIR}/include/aes256_ctr_drbg.h
    ${BASE_DIR}/include/CROSS.h
    ${BASE_DIR}/include/csprng_hash.h
    ${BASE_DIR}/include/pack_unpack.h
    ${BASE_DIR}/include/fips202.h
    ${BASE_DIR}/include/fq_arith.h
    ${BASE_DIR}/include/keccakf1600.h
    ${BASE_DIR}/include/parameters.h
    ${BASE_DIR}/include/seedtree.h
    ${BASE_DIR}/include/sha2.h
    ${BASE_DIR}/include/sha3.h
    ${BASE_DIR}/include/merkle_tree.h
    ${BASE_DIR}/include/merkle.h
)

if(STANDALONE_KECCAK)
  message("Employing standalone SHA-3")
  set(KECCAK_EXTERNAL_LIB "")
  set(KECCAK_EXTERNAL_ENABLE "")
  list(APPEND FALLBACK_SOURCES ${BASE_DIR}/lib/keccakf1600.c)
  list(APPEND FALLBACK_SOURCES ${BASE_DIR}/lib/fips202.c)
else()
  message("Employing libkeccak")
  set(KECCAK_EXTERNAL_LIB keccak)
  set(KECCAK_EXTERNAL_ENABLE "-DSHA_3_LIBKECCAK")
endif()


set(SOURCES
    ${SPEC_SOURCES}
    ${FALLBACK_SOURCES}
    ${BASE_DIR}/lib/aes256_ctr_drbg.c
    ${BASE_DIR}/lib/CROSS.c
    ${BASE_DIR}/lib/csprng_hash.c
    ${BASE_DIR}/lib/pack_unpack.c
    ${BASE_DIR}/lib/keccakf1600.c
    ${BASE_DIR}/lib/fips202.c
    ${BASE_DIR}/lib/seedtree.c
    ${BASE_DIR}/lib/merkle.c
    ${BASE_DIR}/lib/sha2.c
    ${BASE_DIR}/lib/sign.c
)


foreach(category RANGE 1 5 2)
    set(RSDP_VARIANTS RSDP RSDPG)
    foreach(RSDP_VARIANT ${RSDP_VARIANTS})
        set(PARAM_TARGETS SIG_SIZE SPEED)
        foreach(optimiz_target ${PARAM_TARGETS})
             # settings for benchmarking binary
             set(TARGET_BINARY_NAME CROSS_KATgen_cat_${category}_${RSDP_VARIANT}_${optimiz_target})
             add_executable(${TARGET_BINARY_NAME} ${HEADERS} ${SOURCES}
                                ./lib/KAT_NIST_rng.c ./lib/PQCgenKAT_sign.c)
             target_include_directories(${TARGET_BINARY_NAME} PRIVATE
                                        ${BASE_DIR}/include
                                        ./include)
             target_link_libraries(${TARGET_BINARY_NAME} m ssl crypto ${SANITIZE} ${KECCAK_EXTERNAL_LIB})
             set_target_properties(${TARGET_BINARY_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ./bin)
             set_property(TARGET ${TARGET_BINARY_NAME} APPEND PROPERTY
                 COMPILE_FLAGS "-DCATEGORY_${category}=1 -D${optimiz_target}=1 -D${CSPRNG_ALGO}=1 -D${HASH_ALGO}=1 -D${RSDP_VARIANT}=1 ${KECCAK_EXTERNAL_ENABLE} ")
        endforeach(optimiz_target)
    endforeach(RSDP_VARIANT)
endforeach(category)
