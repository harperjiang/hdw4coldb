#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

typedef struct _a {
	uint8_t used;
	double x[3];
	char y[5];
} a;

//int main(int argc, char** argv) {
//	a* array = (a*) calloc(10, sizeof(a));
//	printf("0x%08x\n", array[2].used);
//}
