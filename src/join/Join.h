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
	Matched* _matched;
	bool enableProfiling;

	Lookup* _lookup;
	uint* _probe;
	uint _probeSize;
protected:
	virtual Lookup* createLookup() = 0;
	virtual void buildLookup(kvlist* outer);
	virtual void buildProbe(kvlist* inner);

	/**
	 * Output Execution Summary
	 */
	virtual void printSummary();
public:
	Join(bool enableProfiling = false);
	virtual ~Join();

	virtual void join(kvlist* outer, kvlist* inner) = 0;

	Matched* getMatched();
	void setMatched(Matched*);
};

/**
 * Context for matched records
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
	virtual void setCounter(uint c) {
		this->counter = counter;
	}
	virtual void merge(Matched* another) {
		counter += another->counter;
	}
};

#endif /* SRC_JOIN_JOIN_H_ */
