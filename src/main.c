#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include "hash.h"

int main(int argc, char** argv) {
	hashtable* ht = (hashtable*) malloc(sizeof(hashtable));
	hash_build(ht, 32);

	for (int i = 0; i < 32; i++) {
		uint8_t value[PAYLOAD_SIZE];
		value[0] = i;
		value[2] = 3;

		hash_put(ht, i + 1, value);
	}
}
