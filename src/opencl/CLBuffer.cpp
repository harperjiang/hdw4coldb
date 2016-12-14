/*
 * CLBuffer.cpp
 *
 *  Created on: Nov 25, 2016
 *      Author: Cathy
 */

#include <cstdlib>
#include "CLBuffer.h"
#include "CLProgram.h"

CLBuffer::CLBuffer(CLEnv* env, void* data, unsigned int size,
		cl_mem_flags flag) {
	this->env = env;
	this->_size = size;
	this->rwData = data;
	this->flag = flag;
	this->mapData = NULL;

	cl_int status;
	this->mem_obj = clCreateBuffer(env->context, flag, _size, rwData, &status);
	if (status != CL_SUCCESS) {
		logger->error("Failed to create buffer of size %u: %d\n", size, status);
		mem_obj = NULL;
	}
}

CLBuffer::~CLBuffer() {
	if (this->mem_obj != NULL)
		clReleaseMemObject(mem_obj);
	if (createRw)
		::free(rwData);
}

bool CLBuffer::checkEnv(const char* name) {
	if (NULL == env) {
		logger->error("%x %s CLEnv not set\n", mem_obj, name);
		return false;
	}
	if (NULL == mem_obj) {
		logger->error("%x %s memObj not present\n", mem_obj, name);
		return false;
	}
	return true;
}

bool CLBuffer::attach(CLProgram* program, unsigned int index) {
	if (!checkEnv("attach"))
		return false;
	this->program = program;
	cl_int status;

	status = clSetKernelArg(program->kernel, index, sizeof(cl_mem),
			(void *) &mem_obj);
	if (status != CL_SUCCESS) {
		logger->error("%x failed to attach buffer to kernel %s at %d: %d\n",
				mem_obj, program->name, index, status);
		return false;
	}
	logger->debug("%x buffer attached to kernel %s at %d", mem_obj,
			program->name, index);
	return true;
}

bool CLBuffer::read(bool blocking, bool waitProgram) {
	if (!checkEnv("read"))
		return false;
	cl_uint numEvent = 0;
	cl_event* events = NULL;

	if (waitProgram && this->program != NULL) {
		numEvent = 1;
		events = &(this->program->event);
	}
	// Create Buffer
	if (this->rwData == NULL) {
		this->rwData = ::malloc(_size);
		createRw = true;
	}

	cl_int status = clEnqueueReadBuffer(env->commandQueue, mem_obj,
			blocking ? CL_TRUE : CL_FALSE, 0, _size, rwData, numEvent, events,
			&event);
	if (status != CL_SUCCESS) {
		logger->error("%x failed to read buffer: %d\n", mem_obj, status);
		return false;
	}
	profiling("read", event);
	return true;
}

bool CLBuffer::write(bool blocking) {
	if (!checkEnv("write"))
		return false;
	cl_int status = clEnqueueWriteBuffer(env->commandQueue, mem_obj,
			blocking ? CL_TRUE : CL_FALSE, 0, _size, rwData, 0, NULL, &event);
	if (status != CL_SUCCESS) {
		logger->error("%x failed to write buffer: %d\n", mem_obj, status);
		return false;
	}
	profiling("write", event);
	return true;
}

void* CLBuffer::map(cl_map_flags flags, bool blocking, bool waitProgram) {
	if (!checkEnv("map"))
		return NULL;
	cl_uint numEvent = 0;
	cl_event* events = NULL;

	if (waitProgram && this->program != NULL) {
		numEvent = 1;
		events = &(this->program->event);
	}
	cl_int status;
	mapData = clEnqueueMapBuffer(env->commandQueue, mem_obj,
			blocking ? CL_TRUE : CL_FALSE, flags, 0, _size, numEvent, events,
			&event, &status);
	if (status != CL_SUCCESS) {
		logger->error("%x failed to map buffer: %d\n", status);
		return NULL;
	}
	profiling("map", event);
	return mapData;
}

bool CLBuffer::unmap() {
	if (!checkEnv("unmap"))
		return false;
	if (this->mapData == NULL) {
		logger->error("%x mapped memory not found. Cannot unmap.\n", mem_obj);
		return false;
	}
	cl_int status = clEnqueueUnmapMemObject(env->commandQueue, mem_obj,
			this->mapData, 0, NULL, &event);
	if (status != CL_SUCCESS) {
		logger->error("%x failed to unmap buffer: %d\n", mem_obj, status);
		return false;
	}
	profiling("unmap", event);
	return true;
}

void* CLBuffer::getRwData() {
	return rwData;
}

void* CLBuffer::getMapData() {
	return mapData;
}

unsigned int CLBuffer::size() {
	return _size;
}

void CLBuffer::profiling(const char* name, cl_event event) {
	if (env->enableProfiling) {
		cl_ulong start = 0, end = 0;
		clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
				sizeof(cl_ulong), &start, NULL);
		clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
				sizeof(cl_ulong), &end, NULL);
		logger->debug("%x buffer size %u, %s execution time %u us", mem_obj,
				_size, name, (end - start) / 1000);
	}
}
