
enable_language(C ASM)

SET(ASM_OPTIONS "-x assembler-with-cpp -Wa,-defsym,old_gas_syntax=1 -Wa,-defsym,no_plt=1")
SET(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}" )


add_library(xkcp OBJECT 
  KeccakHash.c 
  KeccakHashtimes4.c 
  KeccakSponge.c 
  KeccakSpongetimes4.c 
  SimpleFIPS202.c
)

if(CORE_LAYER_IMPLEM STREQUAL "avx")
  target_sources(xkcp PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/avx2/KeccakP-1600-AVX2.s
    ${CMAKE_CURRENT_SOURCE_DIR}/avx2/KeccakP-1600-times4-SIMD256.c 
  )
else()
  target_sources(xkcp PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/opt64/KeccakP-1600-opt64.c
    ${CMAKE_CURRENT_SOURCE_DIR}/opt64/KeccakP-1600-times4-on1.c
  )
endif()



target_include_directories(xkcp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(CORE_LAYER_IMPLEM STREQUAL "avx")
  target_include_directories(xkcp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/avx2)
else()
  target_include_directories(xkcp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/opt64)
endif()

