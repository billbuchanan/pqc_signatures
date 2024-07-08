cmake_minimum_required(VERSION 3.22)
project(mirith)
enable_language(ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE ON)

# needed for nvim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
            ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG -O0 -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -Wall -flto -march=native -ftree-vectorize -funroll-loops -fomit-frame-pointer -fno-stack-protector")

set(COMMON_SOURCES
        ${PROJECT_SOURCE_DIR}/matrix.c
        ${PROJECT_SOURCE_DIR}/hash.c
        ${PROJECT_SOURCE_DIR}/packing.c
        ${PROJECT_SOURCE_DIR}/prng.c
        ${PROJECT_SOURCE_DIR}/random.c
        ${PROJECT_SOURCE_DIR}/seed_tree.c
        ${PROJECT_SOURCE_DIR}/sign.c
        ${PROJECT_SOURCE_DIR}/challenges.c
        ${PROJECT_SOURCE_DIR}/fips202.c
        )

add_executable(test test/test_mirith.c ${COMMON_SOURCES})
target_link_libraries(test crypto)
