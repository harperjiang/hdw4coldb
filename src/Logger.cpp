/*
 * log.c
 *
 *  Created on: Nov 22, 2016
 *      Author: Cathy
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "Logger.h"

uint size = 0;
uint limit = 10;

Logger** loggers = (Logger**) ::malloc(sizeof(Logger*) * limit);

Logger* Logger::getLogger(const char* name) {
	for (uint i = 0; i < size; i++) {
		if (0 == ::strcmp(loggers[i]->name, name))
			return loggers[i];
	}
	// Add new logger
	if (size == limit) {
		uint newLimit = limit * 2;
		Logger** tmp = (Logger**) ::malloc(sizeof(Logger*) * newLimit);
		::memcpy(tmp, loggers, sizeof(Logger*) * limit);
		::free(loggers);
		limit = newLimit;
		loggers = tmp;
	}
	loggers[size++] = new Logger(name);
	return loggers[size - 1];
}

Logger::Logger() {
	this->name = "";
}

Logger::Logger(const char* name) {
	this->name = name;
}

void Logger::info(const char* format, ...) {
	if (level > INFO) {
		return;
	}
	sprintf(log_buffer, "%s%s - %s", "[INFO ]", name, format);

	va_list arg;
	va_start(arg, log_buffer);
	vfprintf(stdout, log_buffer, arg);
	va_end(arg);
}

void Logger::warn(const char* format, ...) {
	if (level > WARN) {
		return;
	}
	sprintf(log_buffer, "%s%s - %s", "[WARN ]", name, format);

	va_list arg;
	va_start(arg, log_buffer);
	vfprintf(stdout, log_buffer, arg);
	va_end(arg);
}

void Logger::error(const char* format, ...) {
	if (level > ERROR) {
		return;
	}
	sprintf(log_buffer, "%s%s - %s", "[ERROR ]", name, format);

	va_list arg;
	va_start(arg, log_buffer);
	vfprintf(stderr, log_buffer, arg);
	va_end(arg);
}

void Logger::debug(const char* format, ...) {
	if (level > DEBUG) {
		return;
	}
	sprintf(log_buffer, "%s%s - %s", "[DEBUG ]", name, format);

	va_list arg;
	va_start(arg, log_buffer);
	vfprintf(stdout, log_buffer, arg);
	va_end(arg);
}

Level Logger::getLevel() {
	return level;
}

void Logger::setLevel(Level l) {
	this->level = l;
}

bool Logger::isDebugEnabled() {
	return level <= DEBUG;
}
