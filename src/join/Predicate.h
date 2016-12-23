/*
 * Predicate.h
 *
 *  Created on: Dec 23, 2016
 *      Author: Cathy
 */

#ifndef SRC_JOIN_PREDICATE_H_
#define SRC_JOIN_PREDICATE_H_

#include <sys/types.h>

class Predicate {
public:
	Predicate();
	virtual ~Predicate();
	virtual bool test(uint) = 0;
};

class Equal: public Predicate {
private:
	uint value = 0;
public:
	Equal(uint);
	virtual ~Equal();
	bool test(uint);
};

class NotEqual: public Predicate {
private:
	uint value = 0;
public:
	NotEqual(uint);
	virtual ~NotEqual();
	bool test(uint);
};

#endif /* SRC_JOIN_PREDICATE_H_ */
