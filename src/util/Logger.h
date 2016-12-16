/*
 * Logger.h
 *
 * Warning : Not thread safe implmentation.
 *
 *  Created on: Nov 22, 2016
 *      Author: Cathy
 */

#ifndef SRC_LOGGER_H_
#define SRC_LOGGER_H_

enum Level {
	ALL, DEBUG, INFO, WARN, ERROR, NEVER
};

class Logger {
private:
	const char* name;
	Level level = INFO;
	char log_buffer[1000];

	Logger();
	Logger(const char* name);
public:
	static Logger* getLogger(const char* name);

	Level getLevel();
	void setLevel(Level);

	void info(const char* format, ...);
	void warn(const char* format, ...);
	void error(const char* format, ...);
	void debug(const char* format, ...);

	bool isDebugEnabled();
};

#endif /* SRC_LOGGER_H_ */
