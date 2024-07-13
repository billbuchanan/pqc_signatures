# SHAKE/Keccak implementation from XKCP #

See https://github.com/XKCP/XKCP

## Implementation provided: ##

- opt64: plain C implementation, generically optimized for 64-bit platforms;
- avx2: implementations selected for processors that support the AVX2 instruction set (e.g., Haswell and Skylake microarchitectures);

See https://github.com/XKCP/XKCP#how-can-i-build-the-xkcp

The files has been generated from [XKCP](https://github.com/XKCP/XKCP) at commit `7fa59c0ec4` adding the following target descriptions to [HOWTO-customize.build](https://github.com/XKCP/XKCP/blob/master/doc/HOWTO-customize.build):

```

    <!-- Type "make SIG_PERK-opt64.pack" to get a tarball with the sources needed to compile the FIPS 202 functions generically optimized for 64-bit platforms. -->
    <target name="SIG_PERK-opt64" inherits="KeccakSponge FIPS202 K1600-plain-64bits-ua K1600x4-on1"/>

    <!-- Type "make SIG_PERK-avx2.pack" to get a tarball with the sources needed to compile the FIPS 202 functions generically optimized for 64-bit platforms. -->
    <target name="SIG_PERK-avx2" inherits="KeccakSponge FIPS202 K1600-AVX2 K1600x4-AVX2-ua"/>

```

then running:

```
make clean
make SIG_PERK-avx2.pack SIG_PERK-opt64.pack
```
The [**xsltproc**](https://gnome.pages.gitlab.gnome.org/libxslt/xsltproc.html) is needed by the Makefile

The following files has been borrowed from [Picnic project](https://github.com/IAIK/Picnic) at commit `56ae099`:

- KeccakHashtimes4.c
- KeccakHashtimes4.h
- KeccakSpongetimes4.c
- KeccakSpongetimes4.h
- KeccakSpongetimes4.inc

-------------------------------------
Apply this patch to fix some warnings  and if building with **"msan"** and **asan**:

```patch
diff --git a/lib/XKCP/KeccakSponge.inc b/lib/XKCP/KeccakSponge.inc
index 923a80d..360309c 100644
--- a/lib/XKCP/KeccakSponge.inc
+++ b/lib/XKCP/KeccakSponge.inc
@@ -14,6 +14,17 @@ and related or neighboring rights to the source code in this file.
 http://creativecommons.org/publicdomain/zero/1.0/
 */
 
+#ifndef MSAN_FIX
+#define MSAN_FIX(data, len)
+#if defined(__has_feature)
+#  if __has_feature(memory_sanitizer)
+// code that builds only under MemorySanitizer
+    #undef MSAN_FIX
+    #define MSAN_FIX(data, len) memset(data, 0, len);
+#  endif
+#endif
+#endif
+
 #define JOIN0(a, b)                     a ## b
 #define JOIN(a, b)                      JOIN0(a, b)
 
@@ -34,6 +45,9 @@ http://creativecommons.org/publicdomain/zero/1.0/
 
 int Sponge(unsigned int rate, unsigned int capacity, const unsigned char *input, size_t inputByteLen, unsigned char suffix, unsigned char *output, size_t outputByteLen)
 {
+
+    MSAN_FIX (output, outputByteLen)
+
     ALIGN(SnP_stateAlignment) unsigned char state[SnP_stateSizeInBytes];
     unsigned int partialBlock;
     const unsigned char *curInput = input;
@@ -259,6 +273,8 @@ int SpongeSqueeze(SpongeInstance *instance, unsigned char *data, size_t dataByte
     unsigned int rateInBytes = instance->rate/8;
     unsigned char *curData;
 
+    MSAN_FIX (data, dataByteLen)
+
     if (!instance->squeezing)
         SpongeAbsorbLastFewBits(instance, 0x01);
 
diff --git a/lib/XKCP/avx2/KeccakP-1600-times4-SIMD256.c b/lib/XKCP/avx2/KeccakP-1600-times4-SIMD256.c
index d66f3ee..aae3b84 100644
--- a/lib/XKCP/avx2/KeccakP-1600-times4-SIMD256.c
+++ b/lib/XKCP/avx2/KeccakP-1600-times4-SIMD256.c
@@ -92,6 +92,9 @@ void KeccakP1600times4_InitializeAll(void *states)
     memset(states, 0, KeccakP1600times4_statesSizeInBytes);
 }
 
+#ifdef NO_MISALIGNED_ACCESSES
+__attribute__((no_sanitize("alignment")))
+#endif
 void KeccakP1600times4_AddBytes(void *states, unsigned int instanceIndex, const unsigned char *data, unsigned int offset, unsigned int length)
 {
     unsigned int sizeLeft = length;
@@ -265,6 +268,9 @@ void KeccakP1600times4_OverwriteWithZeroes(void *states, unsigned int instanceIn
     }
 }
 
+#ifdef NO_MISALIGNED_ACCESSES
+__attribute__((no_sanitize("alignment")))
+#endif
 void KeccakP1600times4_ExtractBytes(const void *states, unsigned int instanceIndex, unsigned char *data, unsigned int offset, unsigned int length)
 {
     unsigned int sizeLeft = length;
@@ -900,7 +906,6 @@ size_t KeccakF1600times4_FastLoop_Absorb(void *states, unsigned int laneCount, u
         }
         return (const unsigned char *)curData0 - dataStart;
 #else
-        unsigned int i;
         const unsigned char *dataStart = data;
         const uint64_t *curData0 = (const uint64_t *)data;
         const uint64_t *curData1 = (const uint64_t *)(data+laneOffsetParallel*1*SnP_laneLengthInBytes);
@@ -947,7 +952,6 @@ size_t KeccakF1600times4_FastLoop_Absorb(void *states, unsigned int laneCount, u
 #endif
     }
     else {
-        unsigned int i;
         const unsigned char *dataStart = data;
 
         while(dataByteLen >= (laneOffsetParallel*3 + laneCount)*8) {
@@ -1002,7 +1006,6 @@ size_t KeccakP1600times4_12rounds_FastLoop_Absorb(void *states, unsigned int lan
         }
         return (const unsigned char *)curData0 - dataStart;
 #else
-        unsigned int i;
         const unsigned char *dataStart = data;
         const uint64_t *curData0 = (const uint64_t *)data;
         const uint64_t *curData1 = (const uint64_t *)(data+laneOffsetParallel*1*SnP_laneLengthInBytes);
@@ -1049,7 +1052,6 @@ size_t KeccakP1600times4_12rounds_FastLoop_Absorb(void *states, unsigned int lan
 #endif
     }
     else {
-        unsigned int i;
         const unsigned char *dataStart = data;
 
         while(dataByteLen >= (laneOffsetParallel*3 + laneCount)*8) {
diff --git a/lib/XKCP/opt64/KeccakP-1600-opt64.c b/lib/XKCP/opt64/KeccakP-1600-opt64.c
index 617069e..634b483 100644
--- a/lib/XKCP/opt64/KeccakP-1600-opt64.c
+++ b/lib/XKCP/opt64/KeccakP-1600-opt64.c
@@ -520,6 +520,9 @@ void KeccakP1600_ExtractAndAddBytes(const void *state, const unsigned char *inpu
 
 /* ---------------------------------------------------------------- */
 
+#ifdef NO_MISALIGNED_ACCESSES
+__attribute__((no_sanitize("alignment")))
+#endif
 size_t KeccakF1600_FastLoop_Absorb(void *state, unsigned int laneCount, const unsigned char *data, size_t dataByteLen)
 {
     size_t originalDataByteLen = dataByteLen;
```