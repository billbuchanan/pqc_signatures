set(BENCH_NAME2 ${PROJECT_NAME}2-benchmark)
add_executable(${BENCH_NAME2} speed.c cpucycles.c speed_print.c ${PROJECT_SOURCE_DIR}/src/randombytes.c)
target_link_libraries(${BENCH_NAME2} ${LIB_NAME2} OpenSSL::Crypto)

set(BENCH_NAME3 ${PROJECT_NAME}3-benchmark)
add_executable(${BENCH_NAME3} speed.c cpucycles.c speed_print.c ${PROJECT_SOURCE_DIR}/src/randombytes.c)
target_link_libraries(${BENCH_NAME3} ${LIB_NAME3} OpenSSL::Crypto)

set(BENCH_NAME5 ${PROJECT_NAME}5-benchmark)
add_executable(${BENCH_NAME5} speed.c cpucycles.c speed_print.c ${PROJECT_SOURCE_DIR}/src/randombytes.c)
target_link_libraries(${BENCH_NAME5} ${LIB_NAME5} OpenSSL::Crypto)