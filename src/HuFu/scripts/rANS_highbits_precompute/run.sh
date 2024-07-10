echo compute sec1
python highbits_freq_precompute.py "1055.6" > sec1_highbits_freq.h
cp sec1_highbits_freq.h prec_encoding_param.h
gcc -Wall prec_encoding.c
./a.out > highbits_compress_params_sec1.h

echo compute sec3
python highbits_freq_precompute.py "1253.8" > sec3_highbits_freq.h
cp sec3_highbits_freq.h prec_encoding_param.h
gcc -Wall prec_encoding.c
./a.out > highbits_compress_params_sec3.h

echo compute sec5
python highbits_freq_precompute.py "1415.3" > sec5_highbits_freq.h
cp sec5_highbits_freq.h prec_encoding_param.h
gcc -Wall prec_encoding.c
./a.out > highbits_compress_params_sec5.h