__kernel void test_bitmap(__global uint* result) {
	int index = get_global_id(0);

	result[0] = (1 << index);
}