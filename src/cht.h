/*
 * Concise Hash Table
 *
 *  Created on: Nov 15, 2016
 *      Author: harper
 */

#ifndef CHT_H_
#define CHT_H_

#include<stdint.h>

#define PAYLOAD_SIZE 32

typedef struct _data {
	uint32_t key;
	uint8_t payload[PAYLOAD_SIZE];
} data, *pdata;

typedef struct _cht {
	uint32_t size;
	uint64_t* bitmap;
	data* payloads;
} cht, *pcht;

#endif /* CHT_H_ */
