/*
 * CLEnv.h
 *
 *  Created on: Nov 25, 2016
 *      Author: Cathy
 */

#ifndef SRC_OPENCL_CLENV_H_
#define SRC_OPENCL_CLENV_H_

#include <CL/opencl.h>
#include "../Logger.h"

class CLProgram;
class CLBuffer;

class CLEnv {
	friend class CLProgram;
	friend class CLBuffer;

public:
	Logger logger = Logger("CLEnv");
private:
	cl_platform_id platform = NULL;
	cl_device_id device = NULL;
	cl_context context = NULL;
	cl_command_queue commandQueue = NULL;

	char vendor[50];
protected:
public:

	CLEnv();
	virtual ~CLEnv();

public:
	void displayDeviceInfo();
	char* vendorName();
	bool isAMD();
};

#endif /* SRC_OPENCL_CLENV_H_ */
