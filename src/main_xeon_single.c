/**
 * Performance Comparison
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "cht.h"
#include "perf.h"
//
int main(int argc, char** argv) {

	srand(time(NULL));

	cht* table = (cht*) malloc(sizeof(cht));

	perf_loadcht(table, "unique_500000.data");

	// Warm up
	perf_accesscht(table, 200000);
}
