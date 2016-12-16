# Makefile for generating programs for performance test

CXX=g++

INCLUDE_OPENCL= -I $(AMDAPPSDKROOT)/include

CXX_FLAG= -O3 -Wall -std=gnu++11 -c -fmessage-length=0 -pthread $(INCLUDE_OPENCL)

BIN = ../bin

LIB_OPENCL= -L $(AMDAPPSDKROOT)/lib/x86_64 -lOpenCL

LIBS = -pthread $(LIB_OPENCL)

RM=rm

ifeq ($(OS),Windows_NT)
	EXE_SUFFIX=.exe
else
	EXE_SUFFIX=	
endif

CPP_SRCS += \
../src/lookup/CHT.cpp \
../src/lookup/Hash.cpp \
../src/lookup/Lookup.cpp \
../src/lookup/LookupHelper.cpp \
../src/util/Logger.cpp \
../src/util/Timer.cpp \
../src/opencl/CLBuffer.cpp \
../src/opencl/CLEnv.cpp \
../src/opencl/CLProgram.cpp \
../src/tool/gentestdata.cpp \
../src/tool/mergedata.cpp \
../src/tool/space_analysis.cpp \
../src/singlethread.cpp \
../src/mthread.cpp \
../src/opencl.cpp \

COMMON_OBJS = \
./src/lookup/Lookup.o \
./src/lookup/CHT.o \
./src/lookup/Hash.o \
./src/lookup/LookupHelper.o \
./src/util/Logger.o \
./src/util/Timer.o \
./src/opencl/CLBuffer.o \
./src/opencl/CLEnv.o \
./src/opencl/CLProgram.o 

MAIN = \
opencl \
singlethread \
mthread

MAIN_OBJS = \
./src/opencl.o \
./src/singlethread.o \
./src/mthread.o \

TOOL = \
gendata \
mergedata \
space_analysis \

TOOL_OBJS = \
./tool/space_analysis.o \
./tool/gendata.o \
./tool/mergedata.o \

C_DEPS = \
./src/lookup/CHT.d \
./src/lookup/Hash.d \
./src/util/Logger.d \
./src/lookup/Lookup.d \
./src/util/Timer.d \
./src/lookup/LookupHelper.d \
./src/opencl/CLBuffer.d \
./src/opencl/CLEnv.d \
./src/opencl/CLProgram.d \
./src/tool/gentestdata.d \
./src/tool/mergedata.d \
./src/tool/space_analysis.d \
./src/single.d \
./src/mthread.d \
./src/opencl.d \

default: $(MAIN) $(TOOL)

$(MAIN) : %: src/%.o $(COMMON_OBJS) $(BIN) copycl
	@echo 'Generating $@'
	$(CXX) -o $(BIN)/$@$(EXE_SUFFIX) $(COMMON_OBJS) ./src/$@.o $(LIBS)
	@echo '$@ Generated'
	@echo ' '
	
$(TOOL): %: src/tool/%.o $(COMMON_OBJS) $(BIN)
	@echo 'Generating $@'
	$(CXX) -o $(BIN)/$@$(EXE_SUFFIX) $(COMMON_OBJS) ./src/tool/$@.o $(LIBS)
	@echo '$@ Generated'
	@echo ' '

copycl: $(BIN)
	@echo 'Copying CL Files'
	cp ../src/cl/*.cl $(BIN)
	@echo 'CL Files Generated'
	@echo ' '

src/%.o: ../src/%.cpp mkfolder
	@echo 'Building file: $<'
	$(CXX) $(CXX_FLAG) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

mkfolder:
	mkdir $(BIN)
	mkdir src
	mkdir src/lookup
	mkdir src/tool
	mkdir src/opencl
	mkdir src/util

clean:
	-$(RM) -f $(MAIN) $(TOOL) $(COMMON_OBJS) $(EXE_OBJS) $(C_DEPS)
	-$(RM) -rf $(BIN) src
	-@echo ' '
