// Scan Bitmap and store the result word by word

#define INPUT_UNIT 64
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics: enable

__kernel void scan_bitmap(__global uint* meta, __global ulong* bitmap,
		__global uint* inner, __global ulong* result) {

	uint index = get_global_id(0);

	uint bitIndex = index % INPUT_UNIT;
	uint wordIndex = index / INPUT_UNIT;

	uint bitmapSize = meta[0];

	uint hval = (inner[index] * ((uint)2654435761)) % bitmapSize;

	uint bitmapIndex = hval / INPUT_UNIT;
	uint bitmapOffset = hval % INPUT_UNIT;

	uint bittest = bitmap[bitmapIndex] & (1 << bitmapOffset) & 0xffffffff;

	atomic_or(result+wordIndex, ((ulong)isnotequal((float)bittest,(float)0)) << bitIndex);
}
