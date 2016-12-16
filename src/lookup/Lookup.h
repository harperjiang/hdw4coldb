/*
 * Algorithm.h
 *
 *  Created on: Nov 24, 2016
 *      Author: Cathy
 */

#ifndef SRC_CPP_LOOKUP_H_
#define SRC_CPP_LOOKUP_H_

#include <cstdint>
#include <cstdbool>
#include <cstdlib>

#define PAYLOAD_SIZE 4

struct kv {
public:
	uint32_t key = 0;
	uint8_t payload[PAYLOAD_SIZE];
};

struct kvlist {
	kv* entries = NULL;
	uint32_t size = 0;
};

class ScanContext {
protected:
	uint8_t* inner = NULL;
	void (*func)(uint32_t, uint8_t*, uint8_t*, void*);
	void* params = NULL;
public:
	ScanContext(void (*func)(uint32_t, uint8_t*, uint8_t*, void*),
			void *param) {
		this->func = func;
		this->params = param;
	}
	~ScanContext() {
	}

	void updateInner(uint8_t* inner) {
		this->inner = inner;
	}

	void execute(uint32_t key, uint8_t* outer) {
		this->func(key, outer, inner, params);
	}
};

class Lookup {
private:
	const char* name;
public:
	Lookup(const char* name);
	virtual ~Lookup() {
	}

	const char* getName() {
		return name;
	}

	virtual bool has(uint32_t key) = 0;
	virtual void build(kv* datas, uint32_t size) = 0;
	virtual uint8_t* access(uint32_t key) = 0;
	virtual void scan(uint32_t key, ScanContext* context) = 0;
};

#endif /* SRC_CPP_LOOKUP_H_ */
