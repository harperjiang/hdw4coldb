__kernel void scan_bitmap(__global uint* meta, __global ulong* bitmap,
		__global uint* inner, __global char* result) {
	int index = get_global_id(0);

	uint bitmapSize = meta[0] * 32;

	uint hval = (inner[index] * ((uint)2654435761)) % bitmapSize;

	uint bitmapIndex = hval /32;
	uint bitmapOffset = hval % 32;

	result[index] = (bitmap[bitmapIndex] & (1 << bitmapOffset) & 0xffffffff)?1:0;
}