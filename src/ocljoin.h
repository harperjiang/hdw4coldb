/*
 * ocljoin.h
 *
 *  Created on: Dec 16, 2016
 *      Author: harper
 */

#ifndef SRC_OCLJOIN_H_
#define SRC_OCLJOIN_H_

#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <getopt.h>

#include "util/Timer.h"
#include "lookup/CHT.h"
#include "util/Logger.h"
#include "opencl/CLEnv.h"
#include "opencl/CLProgram.h"
#include "lookup/LookupHelper.h"

#define THRESHOLD 5
#define BITMAP_SIZE		32
#define BITMAP_EXT 		32
#define BITMAP_UNIT 	32
#define BITMAP_EXTMASK 	0xffffffff00000000
#define BITMAP_MASK		0xffffffff

#define OVERFLOW_INIT   10000
#define MIN_SIZE 	1000

#endif /* SRC_OCLJOIN_H_ */
