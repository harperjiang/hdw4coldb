/*
 * CLEnv.cpp
 *
 *  Created on: Nov 25, 2016
 *      Author: Cathy
 */

#include <cstring>
#include "CLEnv.h"

CLEnv::CLEnv() {
	/* Getting platforms and choose an available one.*/
	cl_uint numPlatforms;	//the NO. of platforms
	platform = NULL;	//the chosen platform
	cl_int status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS) {
		logger.error("Failed to get platforms.\n");
		return;
	}

	/* Choose the first available platform. */
	if (numPlatforms > 0) {
		cl_platform_id* platforms = (cl_platform_id*) malloc(
				numPlatforms * sizeof(cl_platform_id));
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		platform = platforms[0];
		free(platforms);
	} else {
		logger.error("No platform available.\n");
		return;
	}

	/* Query the platform and choose the first GPU device if has one.Otherwise use the CPU as device.*/
	cl_uint numDevices = 0;
	cl_device_id *devices;
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
	logger.debug("Number of GPU discovered : %d\n", numDevices);
	if (numDevices == 0) {
		//no GPU available.
		logger.warn("No GPU device available.\n");
		logger.warn("Choose CPU as default device.\n");
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL,
				&numDevices);
		devices = (cl_device_id*) malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices,
				devices, NULL);
	} else {
		devices = (cl_device_id*) malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices,
				devices, NULL);
	}
	if (status != CL_SUCCESS) {
		logger.error("Failed to query devices: %d\n", status);
		return;
	}
	device = devices[0];
	free(devices);

	/* Display Device Information */

	if (logger.isDebugEnabled()) {
		this->displayDeviceInfo();
	}
	/* Create context.*/
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);
	if (status != CL_SUCCESS) {
		logger.error("Failed to create context: %d\n", status);
		return;
	}
	/* Creating command queue associate with the context.*/
	commandQueue = clCreateCommandQueueWithProperties(context, device, NULL,
			&status);
	if (status != CL_SUCCESS) {
		logger.error("Failed to create command queue: %d\n", status);
		return;
	}
	/* Query vendor infos */
	vendor[0] = '\0';
	vendorName();
}

CLEnv::~CLEnv() {
	cl_int status = clReleaseCommandQueue(commandQueue);
	if (status != CL_SUCCESS) {
		logger.error("Failed to release command queue: %d\n", status);
	}
	status = clReleaseContext(context);
	if (status != CL_SUCCESS) {
		logger.error("Failed to release context: %d\n", status);
	}
}

void CLEnv::displayDeviceInfo() {
	unsigned long memSize;
	char devName[30];
	cl_int status;
	status = clGetDeviceInfo(device, CL_DEVICE_NAME, 30, devName, NULL);
	if (status == CL_SUCCESS) {
		logger.debug("Device Name: %s\n", devName);
	}
	status = clGetDeviceInfo(device, CL_DEVICE_VENDOR, 30, devName, NULL);
	if (status == CL_SUCCESS) {
		logger.debug("Device Vendor: %s\n", devName);
	}
	status = clGetDeviceInfo(device, CL_DEVICE_VERSION, 30, devName, NULL);
	if (status == CL_SUCCESS) {
		logger.debug("OpenCL Version: %s\n", devName);
	}
	status = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE,
			sizeof(unsigned long), &memSize, NULL);
	if (status == CL_SUCCESS) {
		logger.debug("Device global mem size: %uMB\n", memSize / (1 << 20));
	}
	status = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,
			sizeof(unsigned long), &memSize, NULL);
	if (status == CL_SUCCESS) {
		logger.debug("Device global mem cache size: %uKB\n",
				memSize / (1 << 10));
	}
	status = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE,
			sizeof(unsigned long), &memSize, NULL);
	if (status == CL_SUCCESS) {
		logger.debug("Device local mem size: %uKB\n", memSize / (1 << 10));
	}
	status = clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY,
			sizeof(unsigned long), &memSize, NULL);
	if (status == CL_SUCCESS) {
		logger.debug("Device frequency: %u MHz\n", memSize);
	}
	status = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS,
			sizeof(unsigned long), &memSize, NULL);
	if (status == CL_SUCCESS) {
		logger.debug("Device max compute unit: %u \n", memSize);
	}
}

char* CLEnv::vendorName() {
	if (0 != ::strlen(vendor))
		return vendor;
	cl_int status = clGetDeviceInfo(device, CL_DEVICE_VENDOR, 50, vendor, NULL);
	if (status != CL_SUCCESS) {
		logger.error("Failed to query vendor name\n");
	}
	return vendor;
}

bool CLEnv::isAMD() {
	return 0 == ::strcmp("Advanced Micro Devices, Inc.", vendorName());
}
