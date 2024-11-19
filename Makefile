CC = g++
OPT = -O3
DBG = -g -std=c++11
WARN = -Wall
ERR = -Werror

CFLAGS = $(OPT) $(WARN) $(ERR) $(INC) $(LIB)
DFLAGS = $(DBG) $(WARN) $(ERR) $(INC) $(LIB)

SIM_SRC = main.cc cache.cc
SIM_OBJ = main.o cache.o

all: smp_cache
	@echo "Compilation Done ---> nothing else to make :) "

smp_cache: $(SIM_OBJ)
	$(CC) -o smp_cache $(CFLAGS) $(SIM_OBJ) -lm
	@echo "----------------------------------------------------------"
	@echo "-----------FALL24-506-406 SMP SIMULATOR (SMP_CACHE)-----------"
	@echo "----------------------------------------------------------"

debug: $(SIM_OBJ)
	$(CC) -o smp_cache $(DFLAGS) $(SIM_OBJ) -lm
	@echo "----------------------------------------------------------"
	@echo "-----------Debug Build of SMP_CACHE---------------------------"
	@echo "----------------------------------------------------------"

test_mesi: smp_cache
	@echo "Running test with MESI protocol, cache size = infinite, associativity = infinite, block size = 64, processors = 4, trace = canneal.04t.longTrace"
	./smp_cache --cache-size -1 --assoc 8 --block-size 64 --num-proc 4 --protocol 0 --trace ../trace/canneal.04t.longTrace

test_moesi: smp_cache
	@echo "Running test with MOESI protocol, cache size = infinite, associativity = 8, block size = 64, processors = 4, trace = canneal.04t.longTrace"
	./smp_cache --cache-size -1 --assoc 8 --block-size 64 --num-proc 4 --protocol 1 --trace ../trace/canneal.04t.longTrace

.cc.o:
	$(CC) $(CFLAGS) -c $*.cc

clean:
	rm -f *.o smp_cache

clobber: clean
