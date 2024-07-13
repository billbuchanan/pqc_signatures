SDitH
======

This submission package is composed of the following folders:

Reference_Implementation/
    Hypercube_Variant/
        sdith_hypercube_cat1_gf256/
            Reference implementation of SDitH-Hypercube with Category 1
            parameters and field GF256.
        
        sdith_hypercube_cat1_p251/
            Reference implementation of SDitH-Hypercube with Category 1
            parameters and field P251.
        
        sdith_hypercube_cat3_gf256/
            Reference implementation of SDitH-Hypercube with Category 3
            parameters and field GF256.
        
        sdith_hypercube_cat3_p251/
            Reference implementation of SDitH-Hypercube with Category 3
            parameters and field P251.
        
        sdith_hypercube_cat5_gf256/
            Reference implementation of SDitH-Hypercube with Category 5
            parameters and field GF256.
        
        sdith_hypercube_cat5_p251/
            Reference implementation of SDitH-Hypercube with Category 5
            parameters and field P251.
    
    Threshold_Variant/
        sdith_threshold_cat1_gf256/
            Reference implementation of SDitH-Threshold with Category 1
            parameters and field GF256.

        sdith_threshold_cat1_p251/
            Reference implementation of SDitH-Threshold with Category 1
            parameters and field P251.

        sdith_threshold_cat3_gf256/
            Reference implementation of SDitH-Threshold with Category 3
            parameters and field GF256.

        sdith_threshold_cat3_p251/
            Reference implementation of SDitH-Threshold with Category 3
            parameters and field P251.

        sdith_threshold_cat5_gf256/
            Reference implementation of SDitH-Threshold with Category 5
            parameters and field GF256.

        sdith_threshold_cat5_p251/
            Reference implementation of SDitH-Threshold with Category 5
            parameters and field P251.

Optimized_Implementation/
    Hypercube_Variant/
        sdith_hypercube_cat1_gf256/
            AVX2 implementation of SDitH-Hypercube with Category 1
            parameters and field GF256.
        
        sdith_hypercube_cat1_p251/
            AVX2 implementation of SDitH-Hypercube with Category 1
            parameters and field P251.
        
        sdith_hypercube_cat3_gf256/
            AVX2 implementation of SDitH-Hypercube with Category 3
            parameters and field GF256.
        
        sdith_hypercube_cat3_p251/
            AVX2 implementation of SDitH-Hypercube with Category 3
            parameters and field P251.
        
        sdith_hypercube_cat5_gf256/
            AVX2 implementation of SDitH-Hypercube with Category 5
            parameters and field GF256.
        
        sdith_hypercube_cat5_p251/
            AVX2 implementation of SDitH-Hypercube with Category 5
            parameters and field P251.
    
    Threshold_Variant/
        sdith_threshold_cat1_gf256/
            AVX2 implementation of SDitH-Threshold with Category 1
            parameters and field GF256.

        sdith_threshold_cat1_p251/
            AVX2 implementation of SDitH-Threshold with Category 1
            parameters and field P251.

        sdith_threshold_cat3_gf256/
            AVX2 implementation of SDitH-Threshold with Category 3
            parameters and field GF256.

        sdith_threshold_cat3_p251/
            AVX2 implementation of SDitH-Threshold with Category 3
            parameters and field P251.

        sdith_threshold_cat5_gf256/
            AVX2 implementation of SDitH-Threshold with Category 5
            parameters and field GF256.

        sdith_threshold_cat5_p251/
            AVX2 implementation of SDitH-Threshold with Category 5
            parameters and field P251.

KAT/
    Hypercube_Variant/
        sdith_hypercube_<category>_<field>/
            KAT vectors for sdith_hypercube_<category>_<field>.

    Threshold_Variant/
        sdith_threshold_<category>_<field>/
            KAT vectors for sdith_threshold_<category>_<field>.

Supporting_Documentation/
    cover_sheet.pdf
    specifications.pdf
    IP_Statements/
        IP statements of submitters

Notes
======

Each implementation under Reference_Implementation and
Optimized_Implementation has its own Makefile; when used, it compiles
the code along with the test vector generator. The
resulting binary `sign` (created in the same directory), when
executed, produced the .req and .rsp files, which should match the ones
provided in KAT/.
