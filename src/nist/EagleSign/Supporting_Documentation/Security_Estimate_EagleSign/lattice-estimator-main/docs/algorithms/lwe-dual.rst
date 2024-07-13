.. _LWE Dual Attacks:

LWE Dual Attacks
==================

We construct an (easy) example LWE instance::

    from estimator import *
    params = LWE.Parameters(n=200, q=7981, Xs=ND.SparseTernary(384, 16), Xe=ND.CenteredBinomial(4))
    params

The simples (and quickest to estimate) algorithm is the "plain" dual attack as described in [PQCBook:MicReg09]_::

    LWE.dual(params)

We can improve these results by considering a dual hybrid attack as in [EC:Albrecht17,INDOCRYPT:EspJouKha20]_::

    LWE.dual_hybrid(params)

Further improvements are possible using a meet-in-the-middle approach [EPRINT:CHHS19]_::

   LWE.dual_hybrid(params, mitm_optimization=True)
