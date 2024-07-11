#ifdef QRUOV_CONFIG_H_GEN

#include <stdio.h>

#define QRUOV_STR_INDIR(x)  #x
#define QRUOV_STR(x)        QRUOV_STR_INDIR(x)

char * format =
  "#pragma once\n"
  "#define QRUOV_security_strength_category %d\n"
  "#define QRUOV_q                          %d\n"
  "#define QRUOV_v                          %d\n"
  "#define QRUOV_m                          %d\n"
  "#define QRUOV_L                          %d\n"
  "#define QRUOV_fc                         %d\n"
  "#define QRUOV_fe                         %d\n"
  "#define QRUOV_fc0                        %d\n"
  "#define QRUOV_PLATFORM                   %s\n"
;

int main(){
  printf(format,
    QRUOV_security_strength_category,
    QRUOV_q,
    QRUOV_v,
    QRUOV_m,
    QRUOV_L,
    QRUOV_fc,
    QRUOV_fe,
    QRUOV_fc0,
    QRUOV_STR(QRUOV_PLATFORM)
  ) ;
  return 0 ;
}

#endif
