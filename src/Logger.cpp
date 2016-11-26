/*
 * log.c
 *
 *  Created on: Nov 22, 2016
 *      Author: Cathy
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "Logger.h"

Logger gLogger("");

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
