# Makefile for generating programs for performance test

CXX=g++
RM=rm

INCLUDE_OPENCL= -I $(AMDAPPSDKROOT)/include
INCLUDE=$(INCLUDE_OPENCL)

LIB_OPENCL= -L $(AMDAPPSDKROOT)/lib/x86_64 -lOpenCL
LIBS = -pthread $(LIB_OPENCL)

CXX_FLAG= -O3 -Wall -std=gnu++11 -mavx2 -c -fmessage-length=0 -pthread $(INCLUDE)

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
./src/lookup/BloomFilter.o \
./src/util/Logger.o \
./src/util/Timer.o \
./src/util/Thread.o \
./src/filter/Filter.o \
./src/filter/Pred.o \
./src/filter/ScalarFilter.o \
./src/filter/SimdFilter.o \
./src/simd/SimdHelper.o \
./src/simd/SimdBuffer.o \
./src/simd/SimdBuffer64.o \
./src/opencl/CLBuffer.o \
./src/opencl/CLEnv.o \
./src/opencl/CLProgram.o \
./src/join/Join.o \
./src/join/CounterThread.o \
./src/join/CollectThread.o \
./src/join/Predicate.o \
./src/join/cstep/GatherThread.o \
./src/join/cstep/StepCHTJoin.o \
./src/join/cstep/OclStepCHTJoin.o \
./src/join/cstep/SimdStepCHTJoin.o \
./src/join/ocl/OclCHTJoin.o \
./src/join/ocl/OclHashJoin.o \
./src/join/simd/SimdCHTJoin.o \
./src/join/simple/CHTJoin.o \
./src/join/simple/HashJoin.o \
./src/join/simple/LookupJoin.o \
./src/join/simple/LookupThread.o \
./src/vecbuffer/SimdVecBuffer.o \
./src/vecbuffer/SimpleVecBuffer.o \
./src/vecbuffer/Simd64VecBuffer.o \
./src/vecbuffer/Simple64VecBuffer.o \
./src/vecbuffer/VecBuffer.o \


TEST_OBJS = \
./test/bitmap_test.o \
./test/SimdCHTJoin_test.o \
./test/CHT_test.o \
./test/Hash_test.o \
./test/LookupHelper_test.o \
./test/SimdStepCHTJoin_test.o \
./test/SimdHelper_test.o \
./test/SimdBuffer_test.o \
./test/SimdBuffer64_test.o \
./test/Timer_test.o \
./test/CounterThread_test.o \
./test/CollectThread_test.o \
./test/BloomFilter_test.o \
./test/SimdVecBuffer_test.o \
./test/Simd64VecBuffer_test.o \
./test/SimpleVecBuffer_test.o \

MAIN = join


TOOL = \
gendata \
mergedata \
space_analysis \
buffer_perf \
pred_perf \


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
	mkdir -p src/simd
	mkdir -p src/join/simd
	mkdir -p src/join/cstep
	mkdir -p src/join/ocl
	mkdir -p src/join/simple
	mkdir -p src/vecbuffer
	mkdir -p src/filter

mktestfolder: mkfolder
	mkdir -p test

clean:
	-$(RM) -f $(MAIN) $(TOOL) $(COMMON_OBJS) $(EXE_OBJS) $(C_DEPS)
	-$(RM) -rf $(BIN) src test
	-@echo ' '
	
unittest: $(COMMON_OBJS) $(TEST_OBJS)
	@echo 'Generating Unit Test'
	$(CXX) -o $(BIN)/unittest $(COMMON_OBJS) $(TEST_OBJS) $(LIB_GTEST)
	@echo 'Unit Test generated'
