/*
 * CLProgram.cpp
 *
 *  Created on: Nov 25, 2016
 *      Author: Cathy
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include "CLProgram.h"

CLProgram::CLProgram(CLEnv* env, const char* name) {
	this->env = env;
	this->name = name;
	this->kernel = NULL;
	this->program = NULL;
	this->event = NULL;
}

CLProgram::~CLProgram() {
	cl_int status = CL_SUCCESS;
	if (NULL != kernel) {
		status = clReleaseKernel(kernel); //Release kernel.
	}
	if (status != CL_SUCCESS) {
		logger->error("%s: Failed to release kernel\n", this->name);
	}
	if (NULL != program) {
		status = clReleaseProgram(program);
	}
	if (status != CL_SUCCESS) {
		logger->error("%s: Failed to release program\n", this->name);
	}
	if (status != CL_SUCCESS) {

	}
	if (NULL != buffers) {
		// We don't release the buffer content as we didn't create them
		delete[] buffers;
	}
}

void CLProgram::fromFile(const char* sourceFile, unsigned int numParams,
		const char* option) {
	if (NULL == env) {
		logger->error("%s: CLEnv not set\n", this->name);
		return;
	}
	if (program != NULL) {
		logger->warn("%s: Program has already been initialized.\n", this->name);
		return;
	}
	std::ifstream f(sourceFile);
	std::stringstream st;
	st << f.rdbuf();
	std::string ss = st.str();
	const char* source = ss.c_str();
	this->fromString(source, numParams, option);
}

void CLProgram::fromString(const char* source, unsigned int numParams,
		const char* option) {
	if (NULL == env) {
		logger->error("%s: CLEnv not set\n", this->name);
		return;
	}
	if (program != NULL) {
		logger->warn("%s: Program has already been initialized.\n", this->name);
		return;
	}
	const size_t length = strlen(source);
	cl_int status;
	program = clCreateProgramWithSource(env->context, 1, &source, &length,
			&status);
	if (status != CL_SUCCESS) {
		logger->error("%s: Program loading failed: %d\n", this->name, status);
		program = NULL;
		return;
	}
	status = clBuildProgram(program, 1, &env->device, option, NULL, NULL);
	if (status != CL_SUCCESS) {
		logger->error("%s: Program build failed: %d\n", this->name, status);

		char log[0x10000];
		clGetProgramBuildInfo(program, env->device, CL_PROGRAM_BUILD_LOG,
				0x10000, log, NULL);
		logger->error("%s: Build Log:\n%s\n", this->name, log);

		program = NULL;
		return;
	}

	kernel = clCreateKernel(program, this->name, &status);
	if (CL_SUCCESS != status) {
		logger->error("%s: Kernel failed to initialize: %d\n", this->name,
				status);
		return;
	}

	numBuffer = numParams;
	buffers = new CLBuffer*[numBuffer]();
}

void CLProgram::setBuffer(unsigned int index, CLBuffer* buffer) {
	if (NULL == env) {
		logger->error("%s: CLEnv not set\n", this->name);
		return;
	}
	if (index < numBuffer) {
		buffers[index] = buffer;
	} else {
		logger->error("%s: Set buffer index out of Bounds: %d\n", this->name,
				index);
	}
}

void CLProgram::setInput(unsigned int index, void * data, unsigned int size) {
	setBuffer(index, new CLBuffer(env, data, size,
	CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY));
}

void CLProgram::setOutput(unsigned int index, unsigned int size) {
	setBuffer(index, new CLBuffer(env, NULL, size, CL_MEM_WRITE_ONLY));
}

bool CLProgram::execute(uint workSize) {
	this->execute(workSize, 0);
}

bool CLProgram::execute(unsigned int workSize, uint groupSize) {
	if (NULL == env) {
		logger->error("%s CLEnv not set\n", this->name);
		return false;
	}
	cl_int status;
	if (NULL == program) {
		logger->error("%s Program not init\n", this->name);
		return false;
	}

	if (NULL == kernel) {
		kernel = clCreateKernel(program, this->name, &status);
		if (CL_SUCCESS != status) {
			logger->error("%s Cannot init kernel : %d\n", this->name, status);
			return false;
		}
	}
	// Set kernel arguments
	for (unsigned int i = 0; i < numBuffer; i++) {
		if (NULL == buffers[i]) {
			logger->error("%s Parameter %d not initialized\n", this->name, i);
			return false;
		}
		if (!buffers[i]->attach(this, i)) {
			logger->error("%s Buffer %d creation failed. Stop execution.\n",
					this->name, i);
			return false;
		}
	}
	// Enqueue the program with 1D size
	size_t global_work_size[1] = { workSize };
	size_t local_work_size[1] = { groupSize };
	status = clEnqueueNDRangeKernel(env->commandQueue, kernel, 1, NULL,
			global_work_size, 0 == groupSize ? NULL : local_work_size, 0, NULL,
			&event);
	if (status != CL_SUCCESS) {
		logger->error("%s failed to execute kernel: %d\n", this->name, status);
		return false;
	}
	// Profiling the event
	if (env->enableProfiling) {
		clWaitForEvents(1, &event);
		cl_ulong start = 0, end = 0;
		clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
				sizeof(cl_ulong), &start, NULL);
		clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
				sizeof(cl_ulong), &end, NULL);
//		logger->debug("%s execution time start %lu\n", this->name, start);
//		logger->debug("%s execution time end %lu\n", this->name, end);
		logger->debug("%s execution time %lu us\n", this->name,
				(end - start) / 1000);
	}
	return true;
}

CLBuffer* CLProgram::getBuffer(unsigned int index) {
	if (index >= numBuffer) {
		logger->warn("getBuffer: Index out of Bound: %d\n", index);
		return NULL;
	}
	return buffers[index];
}

