# djbsort at version 20190516

## install

Download and unzip djbsort package following the instructions at https://sorting.cr.yp.to/install.html 
Then, from the djbsort-20190516 directory, type the following commands: 

```bash
PERK_DIR=</full/path/to/perk/project/directory>
mkdir -p $PERK_DIR/lib/djbsort/avx2
cp int32/avx2/sort.c $PERK_DIR/lib/djbsort/avx2/sort.c
cp h-internal/int32_minmax_x86.c  $PERK_DIR/lib/djbsort/avx2/int32_minmax_x86.inc
mkdir -p $PERK_DIR/lib/djbsort/opt64
cp int32/portable4/sort.c $PERK_DIR/lib/djbsort/opt64/sort.c
cp h-internal/int32_minmax.c $PERK_DIR/lib/djbsort/opt64/int32_minmax.inc
cp uint32/useint32/sort.c  $PERK_DIR/lib/djbsort/djbsort.c
```

Change directory to the perk project and apply the following patch:

```patch
diff --git a/lib/djbsort/avx2/int32_minmax_x86.inc b/lib/djbsort/avx2/int32_minmax_x86.inc
index c5f3006..ffc177f 100644
--- a/lib/djbsort/avx2/int32_minmax_x86.inc
+++ b/lib/djbsort/avx2/int32_minmax_x86.inc
@@ -1,7 +1,7 @@
 #define int32_MINMAX(a,b) \
 do { \
   int32 temp1; \
-  asm( \
+  __asm__( \
     "cmpl %1,%0\n\t" \
     "mov %0,%2\n\t" \
     "cmovg %1,%0\n\t" \
diff --git a/lib/djbsort/avx2/sort.c b/lib/djbsort/avx2/sort.c
index ca81bf6..c50c2d2 100644
--- a/lib/djbsort/avx2/sort.c
+++ b/lib/djbsort/avx2/sort.c
@@ -1,8 +1,8 @@
-#include "int32_sort.h"
+#include "djbsort.h"
 #define int32 int32_t
 
 #include <immintrin.h>
-#include "int32_minmax_x86.c"
+#include "int32_minmax_x86.inc"
 
 typedef __m256i int32x8;
 #define int32x8_load(z) _mm256_loadu_si256((__m256i *) (z))
diff --git a/lib/djbsort/djbsort.c b/lib/djbsort/djbsort.c
index b11ed55..c293b0e 100644
--- a/lib/djbsort/djbsort.c
+++ b/lib/djbsort/djbsort.c
@@ -1,5 +1,4 @@
-#include "int32_sort.h"
-#include "uint32_sort.h"
+#include "djbsort.h"
 
 /* can save time by vectorizing xor loops */
 /* can save time by integrating xor loops with int32_sort */
diff --git a/lib/djbsort/djbsort.h b/lib/djbsort/djbsort.h
new file mode 100644
index 0000000..3e3fb5f
--- /dev/null
+++ b/lib/djbsort/djbsort.h
@@ -0,0 +1,15 @@
+
+/**
+ * @file djbsort.h
+ * @brief Header file for sorting functions
+ */
+
+#ifndef DJB_SORT_H
+#define DJB_SORT_H
+
+#include <stdint.h>
+
+extern void uint32_sort(uint32_t *, long long) __attribute__((visibility("default")));
+extern void int32_sort(int32_t *, long long) __attribute__((visibility("default")));
+
+#endif
diff --git a/lib/djbsort/opt64/sort.c b/lib/djbsort/opt64/sort.c
index 511e98a..69d0ee6 100644
--- a/lib/djbsort/opt64/sort.c
+++ b/lib/djbsort/opt64/sort.c
@@ -1,7 +1,7 @@
-#include "int32_sort.h"
+#include "djbsort.h"
 #define int32 int32_t
 
-#include "int32_minmax.c"
+#include "int32_minmax.inc"
 
 void int32_sort(int32 *x,long long n)
 {
```


