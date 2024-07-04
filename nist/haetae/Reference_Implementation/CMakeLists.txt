cmake_minimum_required(VERSION 3.18)
project(haetae LANGUAGES C CXX) # CXX for the google test

enable_testing() # Enables running `ctest`

set(CMAKE_C_STANDARD 11)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(HAETAE_SRCS
  src/poly.c
  src/ntt.c
  src/fft.c
  src/reduce.c
  src/polyvec.c
  src/polymat.c
  src/polyfix.c
  src/decompose.c
  src/sampler.c
  src/packing.c
  src/sign.c
  src/fixpoint.c
  src/encoding.c
)

set(HAETAE_FIPS202_SRCS
  ${HAETAE_SRCS}
  src/symmetric-shake.c
)
set(FIPS202_SRCS src/fips202.c)

if(MSVC)
  set(C_FLAGS /nologo /O2 /W4 /wd4146 /wd4244)
else()
  set(C_FLAGS -O3 -fomit-frame-pointer -Wall -Wextra -Wpedantic)
endif()

find_package(OpenSSL REQUIRED)

include_directories(include)
include_directories(api)
link_directories(libs)

add_library(fips202 SHARED ${FIPS202_SRCS})
add_library(RNG SHARED ${PROJECT_SOURCE_DIR}/src/randombytes.c)
target_compile_options(RNG PRIVATE -O3 -fomit-frame-pointer -fPIC)
target_link_libraries(RNG PUBLIC OpenSSL::Crypto)


# HAETAE 2 SHAKE ONLY
set(LIB_NAME2 ${PROJECT_NAME}2)
add_library(${LIB_NAME2} SHARED ${HAETAE_FIPS202_SRCS})
target_compile_definitions(${LIB_NAME2} PUBLIC HAETAE_MODE=2)
target_compile_options(${LIB_NAME2} PRIVATE ${C_FLAGS})
target_link_libraries(${LIB_NAME2} INTERFACE fips202 m)
target_link_libraries(${LIB_NAME2} PUBLIC RNG)

# HAETAE 3 SHAKE ONLY
set(LIB_NAME3 ${PROJECT_NAME}3)
add_library(${LIB_NAME3} SHARED ${HAETAE_FIPS202_SRCS})
target_compile_definitions(${LIB_NAME3} PUBLIC HAETAE_MODE=3)
target_compile_options(${LIB_NAME3} PRIVATE ${C_FLAGS})
target_link_libraries(${LIB_NAME3} INTERFACE fips202 m)
target_link_libraries(${LIB_NAME3} PUBLIC RNG)

# HAETAE 5 SHAKE ONLY
set(LIB_NAME5 ${PROJECT_NAME}5)
add_library(${LIB_NAME5} SHARED ${HAETAE_FIPS202_SRCS})
target_compile_definitions(${LIB_NAME5} PUBLIC HAETAE_MODE=5)
target_compile_options(${LIB_NAME5} PRIVATE ${C_FLAGS})
target_link_libraries(${LIB_NAME5} INTERFACE fips202 m)
target_link_libraries(${LIB_NAME5} PUBLIC RNG)

add_subdirectory(test)
add_subdirectory(kat)
add_subdirectory(benchmark)
