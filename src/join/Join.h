/*
 * Join.h
 *
 *  Created on: Dec 22, 2016
 *      Author: harper
 */

#ifndef SRC_JOIN_JOIN_H_
#define SRC_JOIN_JOIN_H_

#include "../util/Timer.h"
#include "../lookup/Lookup.h"
#include "../util/Logger.h"

class Matched;

class Join {
protected:
	Timer _timer;
	Logger* _logger;
public:
	Join();
	virtual ~Join();

	virtual void join(kvlist* outer, kvlist* inner, uint split,
			bool enableProfiling) = 0;
};

/**
 * Context for match
 */
class Matched {
private:
	uint counter;
public:
	Matched() {
		counter = 0;
	}
	virtual ~Matched() {
	}
	virtual void match(uint key, uint8_t* innerpld, uint8_t* outerpld) {
		counter++;
	}
	virtual uint getCounter() {
		return counter;
	}
};

#endif /* SRC_JOIN_JOIN_H_ */
