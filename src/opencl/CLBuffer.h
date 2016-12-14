/*
 * CLBuffer.h
 *
 *  Created on: Nov 25, 2016
 *      Author: Cathy
 */

#ifndef SRC_OPENCL_CLBUFFER_H_
#define SRC_OPENCL_CLBUFFER_H_

#include <CL/opencl.h>
#include "CLEnv.h"
#include "../Logger.h"

class CLProgram;

class CLBuffer {
private:
	Logger* logger = Logger::getLogger("CLBuffer");
private:
	CLEnv* env;
	CLProgram* program = NULL;
	unsigned int _offset;
	unsigned int _size;

	void* rwData = NULL;
	bool createRw = false;
	void* mapData = NULL;

	cl_mem mem_obj = NULL;
	cl_mem_flags flag;
	cl_event event;
private:
	bool checkEnv(const char* name);
	void profiling(const char* name, cl_event event);
public:
	CLBuffer(CLEnv*, void*, unsigned int, cl_mem_flags);
	virtual ~CLBuffer();

	bool read(bool blocking = true, bool waitProgram = true);
	bool write(bool blocking = true);
	void* map(cl_map_flags flags, bool blocking = true,
			bool waitProgram = true);
	bool unmap();

	bool attach(CLProgram*, unsigned int);

	void* getRwData();
	void* getMapData();

	unsigned int size();
};

#endif /* SRC_OPENCL_CLBUFFER_H_ */
