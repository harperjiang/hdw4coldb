/*
 * log.c
 *
 *  Created on: Nov 22, 2016
 *      Author: Cathy
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "log.h"

char log_buffer[1000];

void log_info(const char* format, ...) {

	sprintf(log_buffer, "%s%s", "[INFO ]", format);

	va_list arg;
	va_start(arg, log_buffer);
	vfprintf(stdout, log_buffer, arg);
	va_end(arg);

}
