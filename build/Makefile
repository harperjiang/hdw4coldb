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
../src/hash.c \
../src/perf.c \
../src/util.c \
../src/cat.c \
../tool/gentestdata.c \
../tool/mergedata.c \
../tool/main_xeon_single.c \

COMMON_OBJS = \
./src/chash.o \
./src/cht.o \
./src/cat.o \
./src/hash.o \
./src/perf.o \
./src/util.o 

EXE_OBJS = \
./tool/gendata.o \
./tool/mergedata.o \
./tool/main_xeon_single.o \

EXES = \
gendata \
mergedata \
main_xeon_single \

C_DEPS = \
./src/chash.d \
./src/cht.d \
./src/cat.d \
./src/hash.d \
./src/perf.d \
./src/util.d \
./tool/gentestdata.d \
./tool/mergedata.d \
./tool/main_xeon_single.d 

default: $(EXES)

$(EXES): %: ./tool/%.o $(COMMON_OBJS) $(BIN)
	@echo 'Generating $@'
	$(CC) -o $(BIN)/$@$(EXE_SUFFIX) $(COMMON_OBJS) ./tool/$@.o $(LIBS)
	@echo '$@ Generated'
	@echo ' '

src/%.o: ../src/%.c src
	@echo 'Building file: $<'
	$(CC) $(CC_FLAG) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
	
tool/%.o: ../tool/%.c tool
	@echo 'Building file: $<'
	$(CC) $(CC_FLAG) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src tool $(BIN):	
	mkdir $@
  
clean:
	-$(RM) -f $(EXES)$(COMMON_OBJS)$(EXE_OBJS)$(C_DEPS)
	-$(RM) -rf src $(BIN) tool
	-@echo ' '