# Makefile for generating programs for performance test

CC=g++
CC_FLAG= -O0 -g3 -Wall -c -fmessage-length=0
RM=rm
ifeq ($(OS),Windows_NT)
	EXE_SUFFIX=.exe
else
	EXE_SUFFIX=	
endif

BIN = ../bin
LIBS = 

C_SRCS = \
../src/chash.c \
../src/cht.c \
../src/gentestdata.c \
../src/hash.c \
../src/main_xeon_single.c \
../src/perf.c \
../src/util.c 

COMMON_OBJS = \
./src/chash.o \
./src/cht.o \
./src/hash.o \
./src/perf.o \
./src/util.o 

EXE_OBJS = \
./src/gendata.o \
./src/main_xeon_single.o \

EXES = \
gendata \
main_xeon_single \

C_DEPS = \
./src/chash.d \
./src/cht.d \
./src/gentestdata.d \
./src/hash.d \
./src/main_xeon_single.d \
./src/perf.d \
./src/util.d 

default: $(EXES)

$(EXES): %: ./src/%.o $(COMMON_OBJS) $(BIN)
	@echo 'Generating $@'
	$(CC) -o $(BIN)/$@$(EXE_SUFFIX) $(COMMON_OBJS) ./src/$@.o $(LIBS)
	@echo '$@ Generated'
	@echo ' '

$(BIN):
	mkdir $@

src/%.o: ../src/%.c src
	@echo 'Building file: $<'
	$(CC) $(CC_FLAG) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src:	
	mkdir $@
    
clean:
	-$(RM) -f $(EXES)$(COMMON_OBJS)$(EXE_OBJS)$(C_DEPS)
	-$(RM) -rf src $(BIN)
	-@echo ' '