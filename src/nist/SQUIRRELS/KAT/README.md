Squirrels KATs are shipped in a separate archive.
It can be found at https://drive.google.com/file/d/1AetrZ-PvrNgik0ussOy8VR2D5-R-XvYx/view?usp=sharing.

They are generated from the reference implementation by running build/katint.
Instead of downloading the archive above, it is also possible to compare against
the sha256sum provided in the KAT_Check folder. The script `check_kats.sh` runs
the reference implementation for each parameter set and compares against these 
sha256 sums.

Note that only the reference implementation provides valid KATs; the optimized
implementation reorders floating point operations in an architecture and 
compiler-dependent way, affecting in particular low-order bits of the secret key
generated from a given seed. We stress however that this does not affect the
correctness of the algorithms: keys and signatures generated from both reference
and optimized implementations are functionnaly equivalent, and (in)valid signatures
in one implementation remain so in the other with overwhelming probability.