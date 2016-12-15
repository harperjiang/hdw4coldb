#define THRESHOLD 5
#define BITMAP_SIZE 32

__kernel void scan_cht_full(__global uint* meta, __global ulong* bitmap,__global uint* chtPayload, __global uint* hashpayload,
		__global uint* inner, __global uint* result, __global uint* debugger) {
	int index = get_global_id(0);
	uint key = inner[index];
	uint bitmapSize = meta[0] * 32;
	uint payloadSize = meta[2];
	uint hash = (key * ((uint)2654435761)) % bitmapSize;

	uint bitmapIndex = hash / BITMAP_SIZE;
	uint bitmapOffset = hash % BITMAP_SIZE;

	if(bitmap[bitmapIndex] & (1 << bitmapOffset) & 0xffffffff) {
		ulong bitmapMask = ~(0xffffffffffffffff << bitmapOffset);
		uint offset = (uint)(bitmap[bitmapIndex] >> 32) + popcount((uint)(bitmap[bitmapIndex] & bitmapMask));

		uint i = 0;
		while(i< THRESHOLD && offset+i < payloadSize && chtPayload[offset+i] != key) {
			i++;
		}
		if(offset+i < payloadSize && chtPayload[offset+i] == key) {
			result[index] = offset + i;
			return;
		} else {
			// Search in Hash

			uint bucket_size = meta[1];
			if(0 == bucket_size) {
				result[index] = 0xffffffff;
				return;
			}
			uint counter = (key * ((uint)2654435761)) % bucket_size;

			if(key == (uint)199954612) {
				debugger[0] = (key * (uint)2654435761);
				debugger[1] = counter;
				debugger[4] = bucket_size;
			}

			while(hashpayload[counter]!= key && hashpayload[counter] != 0) {
				counter = (counter + 1) % bucket_size;
			}
			if(hashpayload[counter] == key) {
				result[index] = counter;
			} else {
				result[index] = 0xffffffff;
			}
			if(key == (uint)199954612) {
				debugger[2] = counter;
				debugger[3] = hashpayload[counter];
			}
		}
	} else {
		result[index] = 0xffffffff;
	}
}
