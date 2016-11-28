# Makefile for generating programs for performance test

CC=g++
INCLUDE= -I/cygdrive/e/amd_app_sdk/3.0/include
CC_FLAG= -O3 -Wall -std=gnu++11 -c -fmessage-length=0 -pthread $(INCLUDE)

RM=rm
ifeq ($(OS),Windows_NT)
	EXE_SUFFIX=.exe
else
	EXE_SUFFIX=	
endif

BIN = ../bin
LIBS = -pthread -L/cygdrive/e/amd_app_sdk/3.0/lib/x86_64 -lOpenCL

C_SRCS = \
CPP_SRCS += \
../src/CHT.cpp \
../src/Hash.cpp \
../src/Logger.cpp \
../src/Lookup.cpp \
../src/Timer.cpp \
../src/util.cpp \
../src/opencl/CLBuffer.cpp \
../src/opencl/CLEnv.cpp \
../src/opencl/CLProgram.cpp \
../tool/gentestdata.cpp \
../tool/mergedata.cpp \
../tool/main_single.cpp \
../tool/main_mt.cpp \
../tool/space_analysis.cpp \

COMMON_OBJS = \
./src/CHT.o \
./src/Hash.o \
./src/Logger.o \
./src/Lookup.o \
./src/Timer.o \
./src/util.o \
./src/opencl/CLBuffer.o \
./src/opencl/CLEnv.o \
./src/opencl/CLProgram.o 

EXE_OBJS = \
./tool/main_opencl.o \
./tool/gendata.o \
./tool/mergedata.o \
./tool/main_single.o \
./tool/main_mt.o \
./tool/space_analysis.o \

EXES = \
main_opencl \
gendata \
mergedata \
main_single \
main_mt \
space_analysis \

C_DEPS = \
./src/CHT.d \
./src/Hash.d \
./src/Logger.d \
./src/Lookup.d \
./src/Timer.d \
./src/util.d \
./src/opencl/CLBuffer.d \
./src/opencl/CLEnv.d \
./src/opencl/CLProgram.d \
./tool/gentestdata.d \
./tool/mergedata.d \
./tool/main_single.d \
./tool/main_mt.d \
./tool/space_analysis.d \ 

default: $(EXES)

$(EXES): %: ./tool/%.o $(COMMON_OBJS) $(BIN) copycl
	@echo 'Generating $@'
	$(CC) -o $(BIN)/$@$(EXE_SUFFIX) $(COMMON_OBJS) ./tool/$@.o $(LIBS)
	@echo '$@ Generated'
	@echo ' '

copycl: $(BIN)
	cp ../tool/*.cl $(BIN)

src/%.o: ../src/%.cpp src opencl
	@echo 'Building file: $<'
	$(CC) $(CC_FLAG) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
	
tool/%.o: ../tool/%.cpp tool
	@echo 'Building file: $<'
	$(CC) $(CC_FLAG) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

opencl: src
	mkdir src/$@

src tool $(BIN):
	mkdir $@

clean:
	-$(RM) -f $(EXES)$(COMMON_OBJS)$(EXE_OBJS)$(C_DEPS)
	-$(RM) -rf src $(BIN) tool
	-@echo ' '