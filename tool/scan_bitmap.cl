#define INPUT_UNIT 32

__kernel void scan_bitmap(__global uint* meta, __global ulong* bitmap,
		__global uint* inner, __global uint* result) {

	uint bitindex = get_group_id(0);
	uint wordindex = get_local_id(0);

	if(get_global_id(0) >= get_global_size(0)) {
		return;
	}

	uint bitmapSize = meta[0] * 32;

	uint hval = (inner[index] * ((uint)2654435761)) % bitmapSize;

	uint bitmapIndex = hval / INPUT_UNIT;
	uint bitmapOffset = hval % INPUT_UNIT;

	uint bittest = bitmap[bitmapIndex] & (1 << bitmapOffset) & 0xffffffff;

	atomic_or(result+wordindex, isnotequal((float)bittest,(float)0) << bitindex);
}