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
		logger.error("Failed to create buffer : %d\n", status);
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
		logger.error("%s: CLEnv not set\n", name);
		return false;
	}
	if (NULL == mem_obj) {
		logger.error("%s: MemObj not present\n", name);
		return false;
	}
	return true;
}

bool CLBuffer::attach(CLProgram* program, unsigned int index) {
	if (!checkEnv("Attach"))
		return false;
	this->program = program;
	cl_int status;

	status = clSetKernelArg(program->kernel, index, sizeof(cl_mem),
			(void *) &mem_obj);
	if (status != CL_SUCCESS) {
		logger.error("Failed to attach buffer to kernel: %d\n", status);
		return false;
	}
	return true;
}

bool CLBuffer::read(bool blocking, bool waitProgram) {
	if (!checkEnv("Read"))
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
		logger.error("Failed to read result: %d\n", status);
		return false;
	}
	return true;
}

bool CLBuffer::write(bool blocking) {
	if (!checkEnv("Write"))
		return false;
	cl_int status = clEnqueueWriteBuffer(env->commandQueue, mem_obj,
			blocking ? CL_TRUE : CL_FALSE, 0, _size, rwData, 0, NULL, &event);
	if (status != CL_SUCCESS) {
		logger.error("Failed to write result: %d\n", status);
		return false;
	}
	return true;
}

void* CLBuffer::map(cl_map_flags flags, bool blocking, bool waitProgram) {
	if (!checkEnv("Map"))
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
		logger.error("Failed to map buffer: %d\n", status);
	}
	return mapData;
}

bool CLBuffer::unmap() {
	if (!checkEnv("Unmap"))
		return false;
	if (this->mapData == NULL) {
		logger.error("Mapped memory not found. Cannot unmap.\n");
		return false;
	}
	cl_int status = clEnqueueUnmapMemObject(env->commandQueue, mem_obj,
			this->mapData, 0, NULL, &event);
	if (status != CL_SUCCESS) {
		logger.error("Failed to unmap buffer: %d\n", status);
		return false;
	}
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

