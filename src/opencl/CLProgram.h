/*
 * CLProgram.h
 *
 *  Created on: Nov 25, 2016
 *      Author: Cathy
 */

#ifndef SRC_OPENCL_CLPROGRAM_H_
#define SRC_OPENCL_CLPROGRAM_H_

#include <CL/opencl.h>
#include "CLEnv.h"
#include "CLBuffer.h"
#include "../Logger.h"

class CLProgram {
	friend class CLBuffer;
private:
	Logger* logger = Logger::getLogger("CLProgram");
	const char* name;
private:
	CLEnv* env;

	unsigned int numBuffer = 0;
	CLBuffer** buffers = NULL;

	cl_program program;
	cl_kernel kernel;
	cl_event event;
public:
	CLProgram(CLEnv* env, const char* name);
	virtual ~CLProgram();

	void fromFile(const char* source, unsigned int numParams,
			const char* option = NULL);
	void fromString(const char* strsource, unsigned int numParams,
			const char* option = NULL);

	void setInput(unsigned int index, void* data, unsigned int size);
	void setOutput(unsigned int index, unsigned int size);
	void setBuffer(unsigned int index, CLBuffer* buffer);

	bool execute(unsigned int workSize);

	CLBuffer* getBuffer(unsigned int index);
};

#endif /* SRC_OPENCL_CLPROGRAM_H_ */
