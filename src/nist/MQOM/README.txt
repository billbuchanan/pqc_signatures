MQOM
====

This submission package is composed of the following folders:

KAT/
    mqom_<category>_<field>_<variant>/
        KAT vectors for mqom_<category>_<field>_<variant>.

Optimized_Implementation/
    mqom_<category>_<field>_<variant>/
        Optimized implementation for mqom_<category>_<field>_<variant>.

Reference_Implementation/
    mqom_<category>_<field>_<variant>/
        Reference implementation for mqom_<category>_<field>_<variant>.

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
resulting binary `kat_gen` (created in the same directory), when
executed, produced the .req and .rsp files, which should match the ones
provided in KAT/.
