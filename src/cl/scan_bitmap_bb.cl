// Scan Bitmap and store the result word by word

#define INPUT_UNIT 32

__kernel void scan_bitmap(__global uint* meta, __global ulong* bitmap,
		__global uint* inner, __global uint* result) {
	// Prefetch the bitmap result used to cache
	prefetch(get_group_id()*get_local_size()/INPUT_UNIT, get_local_size()/INPUT_UNIT);

	uint index = get_global_id(0);

	uint bitIndex = index % INPUT_UNIT;
	uint wordIndex = index / INPUT_UNIT;

	uint bitmapSize = meta[0] * 32;

	uint hval = (inner[index] * ((uint)2654435761)) % bitmapSize;

	uint bitmapIndex = hval / INPUT_UNIT;
	uint bitmapOffset = hval % INPUT_UNIT;

	uint bittest = bitmap[bitmapIndex] & (1 << bitmapOffset) & 0xffffffff;

	atomic_or(result+wordIndex, isnotequal((float)bittest,(float)0) << bitIndex);
}
