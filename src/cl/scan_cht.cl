#define THRESHOLD 5
#define BITMAP_SIZE 32

__kernel void scan_cht(__global uint* meta, __global ulong* bitmap, __global uint* chtPayload,
		__global uint* innerKey, __global uint* result) {
	int index = get_global_id(0);

	uint bitmapSize = meta[0];
	uint chtpayloadSize = meta[2];

	uint key = innerKey[index];
	uint hash = (key * ((uint)2654435761)) % (bitmapSize * BITMAP_SIZE);

	uint bitmapIndex = hash / BITMAP_SIZE;
	uint bitmapOffset = hash % BITMAP_SIZE;

	ulong bitmapMask = ~(0xffffffffffffffff << bitmapOffset);
	uint offset = (uint)(bitmap[bitmapIndex] >> 32) + popcount((uint)(bitmap[bitmapIndex] & bitmapMask));

	uint i = 0;
	for(i = 0; i < THRESHOLD && offset + i < chtpayloadSize; i++) {
		if(chtPayload[offset+i] == key) {
			result[index] = offset+i;
			return;
		}
	}
	result[index] = 0xffffffff;
}