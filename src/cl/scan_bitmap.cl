
#define INPUT_UNIT 32

__kernel void scan_bitmap(__global uint* meta, __global ulong* bitmap,
		__global uint* inner, __global uint* result) {

	uint index = get_global_id(0);

/*
	if(index >= meta[4]) {
		return;
	}
*/
	uint bitIndex = index / meta[3];
	uint wordIndex = index % meta[3];

	uint bitmapSize = meta[0] * 32;

	uint hval = (inner[index] * ((uint)2654435761)) % bitmapSize;

	uint bitmapIndex = hval / INPUT_UNIT;
	uint bitmapOffset = hval % INPUT_UNIT;

	uint bittest = bitmap[bitmapIndex] & (1 << bitmapOffset) & 0xffffffff;

	atomic_or(result+wordIndex, isnotequal((float)bittest,(float)0) << bitIndex);
}
