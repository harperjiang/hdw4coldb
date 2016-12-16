#define THRESHOLD 5
#define BITMAP_SIZE 32

__kernel void scan_hash(__global uint* meta, __global uint* payload,
		__global uint* innerKey, __global uint* result) {
	int index = get_global_id(0);

	uint payload_size = meta[1];
	uint key = innerKey[index];
	uint hash = (key * ((uint)2654435761)) % payload_size;
	
	int counter = hash;
	while(payload[counter]!= key && payload[counter] != 0) {
		counter++;
	}
	if(payload[counter] == key) {
		result[index] = counter;
	} else {
		result[index] = 0xffffffff;
	}
}