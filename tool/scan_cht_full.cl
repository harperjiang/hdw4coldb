#define THRESHOLD 5
#define BITMAP_SIZE 32

__kernel void scan_cht_full(__global uint* meta, __global ulong* bitmap,__global uint* chtPayload, __global uint* hashpayload,
		__global uint* inner, __global uint* result) {
	int index = get_global_id(0);
	uint key = inner[index];
	uint bitmapSize = meta[0] * 32;

	uint hash = (key * ((uint)2654435761)) % bitmapSize;

	uint bitmapIndex = hash / BITMAP_SIZE;
	uint bitmapOffset = hash % BITMAP_SIZE;

	if(bitmap[bitmapIndex] & (1 << bitmapOffset) & 0xffffffff) {
		ulong bitmapMask = ~(0xffffffffffffffff << bitmapOffset);
		uint offset = (uint)(bitmap[bitmapIndex] >> 32) + popcount((uint)(bitmap[bitmapIndex] & bitmapMask));

		uint i = 0;
		for(i = 0; i < THRESHOLD; i++) {
			if(chtPayload[offset+i] == key) {
				// Found
				result[index] = offset+i;
				return;
			}
		}
		if(i== THRESHOLD) {
			// Search in Hash
			uint bucket_size = meta[1];

			uint counter = (key * ((uint)2654435761)) % bucket_size;
			while(hashpayload[counter]!= key && hashpayload[counter] != 0) {
				counter = (counter + 1) % bucket_size;
			}
			if(hashpayload[counter] == key) {
				result[index] = counter;
			} else {
				result[index] = 0xffffffff;
			}
		}
	} else {
		result[index] = 0xffffffff;
	}
}
