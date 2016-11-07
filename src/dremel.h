/*
 * dremel.h
 *
 *  Created on: Nov 5, 2016
 *      Author: Hao Jiang
 */

#ifndef SRC_DREMEL_H_
#define SRC_DREMEL_H_

/**
 * a field in structured data
 */
typedef struct _field {
	int 			id;
	int 			maxRepLevel;
	struct _field* 	parent;
} field, *pfield;

typedef struct _datastr {
	int numFields;
	field* fields;
} datastr, *pdatastr;

#endif /* SRC_DREMEL_H_ */
