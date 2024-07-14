# core options for timing
OPT= -lcrypto -O3  -Wall -pedantic
# optimizations
OPT2=
# get rid of some annoying warning
OPT3= -Wno-unused-function

AES_SRC= ./aes/aes256.c
KECCAK_SRC= ./keccak/Keccak-readable-and-compact.c
SRC= atf.c compress.c expand.c field.c matrix.c sign.c

SRC_SPD= ./test/test_speed_alteq.c ./test/cpucycles.c ./test/speed_print.c
SRC_CPU= ./test/get_CPU_time.c

# no need to force old standards for speed tests, this will save a lot of warnings
# STD = -ansi
STD =
# only use RDPMC if you can be sure you can use it. Otherwise default to RDTSC
# RDPMC = -D USE_RDPMC
RDPMC =

test_speed.o: $(SRC) $(AES_SRC) $(KECCAK_SRC) $(SPEED_SRC)
	gcc -o test_speed.o *.c $(AES_SRC) $(KECCAK_SRC)  $(OPT) $(OPT2) $(STD)

CC_SPD_TEST = gcc -o test_speed.o  $(SRC) $(SRC_SPD) $(AES_SRC) $(KECCAK_SRC) $(OPT) $(OPT2) $(OPT3) $(STD) $(RDPMC)

test_speed_all:
	cp ./api/api.h.1.fe api.h
	$(CC_SPD_TEST)
	./test_speed.o 1> out_api.h.1.fe.txt  2> err_api.h.1.fe.txt

	cp ./api/api.h.3.fe api.h
	$(CC_SPD_TEST)
	./test_speed.o 1> out_api.h.3.fe.txt  2> err_api.h.3.fe.txt

	cp ./api/api.h.5.fe api.h
	$(CC_SPD_TEST)
	./test_speed.o 1> out_api.h.5.fe.txt  2> err_api.h.5.fe.txt

	cp ./api/api.h.1.lp api.h
	$(CC_SPD_TEST)
	./test_speed.o 1> out_api.h.1.lp.txt  2> err_api.h.1.lp.txt

	cp ./api/api.h.3.lp api.h
	$(CC_SPD_TEST)
	./test_speed.o 1> out_api.h.3.lp.txt  2> err_api.h.3.lp.txt

	cp ./api/api.h.5.lp api.h
	$(CC_SPD_TEST)
	./test_speed.o 1> out_api.h.5.lp.txt  2> err_api.h.5.lp.txt

	# it is wise to say which cpu was done for the tests...
	lscpu > cpu_model_and_stuff.txt
	gcc -v 2> gcc_version.txt
	cat /etc/os-release > os_info.txt
	cat /proc/meminfo > memory_info.txt

CC_CPU_TEST = gcc -o test_cpu.o  $(SRC) $(SRC_CPU) $(AES_SRC) $(KECCAK_SRC) $(OPT) $(OPT2) $(OPT3) $(STD) $(RDPMC)
test_CPU_time:
	printf  "NUM,LAMBDA,N,LOG_Q,ROUND,K,C,num_success/NUM,CRYPTO_SECRETKEYBYTES, CRYPTO_PUBLICKEYBYTES, CRYPTO_BYTES,CRYPTO_PUBLICKEYBYTES+CRYPTO_BYTES,	(set_time+NUM-1)/NUM,(sig_time+NUM-1)/NUM,(ver_time+NUM-1)/NUM,(sig_time+ver_time+NUM-1)/NUM,	 1000000*set_time/CLOCKS_PER_SEC/NUM,1000000*sig_time/CLOCKS_PER_SEC/NUM,1000000*ver_time/CLOCKS_PER_SEC/NUM, 1000000*(sig_time+ver_time)/CLOCKS_PER_SEC/NUM,   (CRYPTO_PUBLICKEYBYTES+CRYPTO_BYTES)*(sig_time+ver_time)/CLOCKS_PER_SEC/NUM,   (CRYPTO_BYTES)*(ver_time)/CLOCKS_PER_SEC/NUM , CLOCKS_PER_SEC\n" > cpu_time.txt

	cp ./api/api.h.1.fe api.h
	$(CC_CPU_TEST);
	./test_cpu.o 1>> cpu_time.txt  2> err_api.h.1.fe.txt

	cp ./api/api.h.3.fe api.h
	$(CC_CPU_TEST);
	./test_cpu.o 1>> cpu_time.txt  2> err_api.h.3.fe.txt

	cp ./api/api.h.5.fe api.h
	$(CC_CPU_TEST);
	./test_cpu.o 1>> cpu_time.txt  2> err_api.h.5.fe.txt

	cp ./api/api.h.1.lp api.h
	$(CC_CPU_TEST);
	./test_cpu.o 1>> cpu_time.txt  2> err_api.h.1.lp.txt

	cp ./api/api.h.3.lp api.h
	$(CC_CPU_TEST);
	./test_cpu.o 1>> cpu_time.txt  2> err_api.h.3.lp.txt

	cp ./api/api.h.5.lp api.h
	$(CC_CPU_TEST);
	./test_cpu.o 1>> cpu_time.txt  2> err_api.h.5.lp.txt

	# it is wise to say which cpu was done for the tests...
	lscpu > cpu_model_and_stuff.txt
	gcc -v 2> gcc_version.txt
	cat /etc/os-release > os_info.txt
	cat /proc/meminfo > memory_info.txt

clean:
	rm -f *.exe *.o api.h err*.txt

distclean:
	make clean
	rm -f *.txt