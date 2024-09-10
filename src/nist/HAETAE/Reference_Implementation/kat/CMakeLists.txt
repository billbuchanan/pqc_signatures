set(kat_srcs
    kat_sign.c
)

find_package(OpenSSL REQUIRED)

# Rank-2 SIGN
set(KAT_SIGN_LIB_NAME2 ${PROJECT_NAME}2-kat-sign)
add_library(${KAT_SIGN_LIB_NAME2} SHARED ${kat_srcs})
target_include_directories(${KAT_SIGN_LIB_NAME2} PUBLIC ../include)
target_compile_definitions(${KAT_SIGN_LIB_NAME2} PUBLIC HAETAE_MODE=2)

add_executable(${KAT_SIGN_LIB_NAME2}-main PQCgenKAT_sign.c)
target_link_libraries(${KAT_SIGN_LIB_NAME2}-main ${KAT_SIGN_LIB_NAME2} ${LIB_NAME2} OpenSSL::Crypto)

# Rank-3 SIGN
set(KAT_SIGN_LIB_NAME3 ${PROJECT_NAME}3-kat-sign)
add_library(${KAT_SIGN_LIB_NAME3} SHARED ${kat_srcs})
target_include_directories(${KAT_SIGN_LIB_NAME3} PUBLIC ../include)
target_compile_definitions(${KAT_SIGN_LIB_NAME3} PUBLIC HAETAE_MODE=3)

add_executable(${KAT_SIGN_LIB_NAME3}-main PQCgenKAT_sign.c)
target_link_libraries(${KAT_SIGN_LIB_NAME3}-main ${KAT_SIGN_LIB_NAME3} ${LIB_NAME3} OpenSSL::Crypto)

# Rank-5 SIGN
set(KAT_SIGN_LIB_NAME5 ${PROJECT_NAME}5-kat-sign)
add_library(${KAT_SIGN_LIB_NAME5} SHARED ${kat_srcs})
target_include_directories(${KAT_SIGN_LIB_NAME5} PUBLIC ../include)
target_compile_definitions(${KAT_SIGN_LIB_NAME5} PUBLIC HAETAE_MODE=5)

add_executable(${KAT_SIGN_LIB_NAME5}-main PQCgenKAT_sign.c)
target_link_libraries(${KAT_SIGN_LIB_NAME5}-main ${KAT_SIGN_LIB_NAME5} ${LIB_NAME5} OpenSSL::Crypto)
