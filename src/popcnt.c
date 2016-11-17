#include <stdio.h>
#include "util.h"

int main(int argc, char** argv) {
	uint64_t test = 0x0000ffff;
	printf("%d\n", popcount(test));
}
