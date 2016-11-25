/*
 * log.h
 *
 *  Created on: Nov 22, 2016
 *      Author: Cathy
 */

#ifndef SRC_LOGGER_H_
#define SRC_LOGGER_H_

class Logger {
private:
	char log_buffer[1000];
public:
	void info(const char* format, ...);
};

#endif /* SRC_LOGGER_H_ */
