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
		logger.error("Failed to release kernel\n");
	}
	if (NULL != program) {
		status = clReleaseProgram(program);
	}
	if (status != CL_SUCCESS) {
		logger.error("Failed to release program\n");
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
		logger.error("CLEnv not set\n");
		return;
	}
	if (program != NULL) {
		logger.warn("Program has already been initialized.\n");
		return;
	}
	std::ifstream f(sourceFile);
	std::stringstream st;
	st << f.rdbuf();
	std::string ss = st.str();
	const char* source = ss.c_str();
	this->fromString(source, numParams);
}

void CLProgram::fromString(const char* source, unsigned int numParams,
		const char* option) {
	if (NULL == env) {
		logger.error("CLEnv not set\n");
		return;
	}
	if (program != NULL) {
		logger.warn("Program has already been initialized.\n");
		return;
	}
	const size_t length = strlen(source);
	cl_int status;
	program = clCreateProgramWithSource(env->context, 1, &source, &length,
			&status);
	if (status != CL_SUCCESS) {
		logger.error("Program loading failed: %d\n", status);
		program = NULL;
		return;
	}
	status = clBuildProgram(program, 1, &env->device, option, NULL, NULL);
	if (status != CL_SUCCESS) {
		logger.error("Program build failed: %d\n", status);

		char log[0x10000];
		clGetProgramBuildInfo(program, env->device, CL_PROGRAM_BUILD_LOG,
				0x10000, log, NULL);
		logger.error("Build Log:\n%s\n", log);

		program = NULL;
		return;
	}

	numBuffer = numParams;
	buffers = new CLBuffer*[numBuffer]();
}

void CLProgram::setBuffer(unsigned int index, CLBuffer* buffer) {
	if (NULL == env) {
		logger.error("CLEnv not set\n");
		return;
	}
	if (index < numBuffer && buffers[index] != buffer) {
		// Purge old buffer
		if (buffers[index] != NULL) {
			delete buffers[index];
		}
		buffers[index] = buffer;
	} else {
		logger.error("Set buffer index out of Bounds: %d\n", index);
	}
}

void CLProgram::setInput(unsigned int index, void * data, unsigned int size) {
	setBuffer(index,
			new CLBuffer(env, data, size,
					CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY));
}

void CLProgram::setOutput(unsigned int index, unsigned int size) {
	setBuffer(index, new CLBuffer(env, NULL, size, CL_MEM_WRITE_ONLY));
}

bool CLProgram::execute(unsigned int workSize) {
	if (NULL == env) {
		logger.error("CLEnv not set\n");
		return false;
	}
	cl_int status;
	if (NULL == program) {
		logger.error("Program not init\n");
		return false;
	}

	if (NULL == kernel) {
		kernel = clCreateKernel(program, this->name, &status);
		if (CL_SUCCESS != status) {
			logger.error("Cannot init kernel : %d\n", status);
			return false;
		}
	}
	// Set kernel arguments
	for (unsigned int i = 0; i < numBuffer; i++) {
		if (NULL == buffers[i]) {
			logger.error("Parameter %d not initialized\n", i);
			return false;
		}
		if (!buffers[i]->attach(this, i)) {
			logger.error("Buffer %d creation failed. Stop execution.\n", i);
			return false;
		}
	}
	// Enqueue the program with 1D size
	size_t global_work_size[1] = { workSize };
	status = clEnqueueNDRangeKernel(env->commandQueue, kernel, 1, NULL,
			global_work_size,
			NULL, 0, NULL, &event);
	if (status != CL_SUCCESS) {
		logger.error("Failed to execute kernel: %d\n", status);
		return false;
	}
	return true;
}

CLBuffer* CLProgram::getBuffer(unsigned int index) {
	if (index >= numBuffer) {
		logger.warn("getBuffer: Index out of Bound: %d\n", index);
		return NULL;
	}
	return buffers[index];
}

