# Makefile for generating programs for performance test

CXX=g++
RM=rm

INCLUDE_OPENCL= -I $(AMDAPPSDKROOT)/include
INCLUDE=$(INCLUDE_OPENCL)

LIB_OPENCL= -L $(AMDAPPSDKROOT)/lib/x86_64 -lOpenCL
LIBS = -pthread $(LIB_OPENCL)

CXX_FLAG= -O0 -g3 -Wall -std=gnu++11 -mavx2 -c -fmessage-length=0 -pthread $(INCLUDE)

BIN = ../bin

INCLUDE_GTEST= -I $(GTEST)/include
LIB_GTEST= $(LIBS) -L $(GTEST)/make -l:gtest_main.a
CXX_TESTFLAG = $(CXX_FLAG) $(INCLUDE_GTEST)



ifeq ($(OS),Windows_NT)
	EXE_SUFFIX=.exe
else
	EXE_SUFFIX=	
endif

COMMON_OBJS = \
./src/lookup/Lookup.o \
./src/lookup/CHT.o \
./src/lookup/Hash.o \
./src/lookup/LookupHelper.o \
./src/util/Logger.o \
./src/util/Timer.o \
./src/util/Thread.o \
./src/opencl/CLBuffer.o \
./src/opencl/CLEnv.o \
./src/opencl/CLProgram.o \
./src/join/CounterThread.o \
./src/join/cstep/CStep.o \
./src/join/cstep/CStepOcl.o \
./src/join/cstep/CStepSimd.o \
./src/join/cstep/GatherThread.o \
./src/join/ocl/ocl_cht.o \
./src/join/ocl/ocl_hash.o \

TEST_OBJS = \
./test/bitmap_test.o \
./test/CHT_test.o \
./test/Hash_test.o \
./test/LookupHelper_test.o \
./test/simd_test.o \
./test/Timer_test.o \

MAIN = \
ocljoin \
stjoin \
mtjoin \
cstepjoin \


TOOL = \
gendata \
mergedata \
space_analysis \


#C_DEPS = \
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
	

test/%.o: ../test/%.cpp mktestfolder
	@echo 'Building file: $<'
	$(CXX) $(CXX_FLAG) $(CXX_TESTFLAG) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

mkfolder:
	mkdir -p $(BIN)
	mkdir -p src
	mkdir -p src/lookup
	mkdir -p src/tool
	mkdir -p src/opencl
	mkdir -p src/util
	mkdir -p src/join/cstep
	mkdir -p src/join/ocl

mktestfolder: mkfolder
	mkdir -p test

clean:
	-$(RM) -f $(MAIN) $(TOOL) $(COMMON_OBJS) $(EXE_OBJS) $(C_DEPS)
	-$(RM) -rf $(BIN) src
	-@echo ' '
	
unittest: $(COMMON_OBJS) $(TEST_OBJS)
	@echo 'Generating Unit Test'
	$(CXX) -o $(BIN)/unittest $(COMMON_OBJS) $(TEST_OBJS) $(LIB_GTEST)
	@echo 'Unit Test generated'
