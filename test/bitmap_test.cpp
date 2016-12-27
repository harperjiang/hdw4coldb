#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../src/lookup/CHT.h"

extern bool bitmap_testset(uint64_t* bitmap, uint32_t offset);
extern void bitmap_setpopcnt(uint64_t* bitmap, uint32_t offset, uint32_t value);
extern uint32_t bitmap_popcnt(uint64_t* bitmap, uint32_t val);
extern bool bitmap_test(uint64_t* bitmap, uint32_t offset);
extern uint32_t bitmap_roundup(uint32_t);

TEST( Bitmap, TestSet) {
	uint64_t* bitmap = (uint64_t*) calloc(1024, sizeof(uint64_t));

	for (uint64_t i = 0; i < 1024; i++) {
		ASSERT_EQ(0, bitmap[i]);
		// Set a value to higher bit
		bitmap[i] |= (i << 32);
	}

	for (uint32_t i = 0; i < 1024 * 32; i++) {
		ASSERT_TRUE(bitmap_testset(bitmap, i));
	}

	for (uint64_t i = 0; i < 1024; i++) {
		ASSERT_EQ(0xffffffff00000000 & bitmap[i], (i << 32));
	}

	for (uint32_t i = 0; i < 1024 * 32; i++) {
		ASSERT_FALSE(bitmap_testset(bitmap, i));
	}

	free(bitmap);
}

TEST(Bitmap, Test) {
	uint64_t* bitmap = (uint64_t*) calloc(1024, sizeof(uint64_t));

	for (uint64_t i = 0; i < 1024; i++) {
		ASSERT_EQ(0, bitmap[i]);
		bitmap[i] |= i;
	}

	for (uint32_t i = 0; i < 1024 * 32; i++) {
		uint32_t num = i / 32;
		uint32_t remainder = i % 32;
		if (num & (1 << remainder)) {
			ASSERT_TRUE(bitmap_test(bitmap, i));
		} else {
			ASSERT_FALSE(bitmap_test(bitmap, i));
		}
	}
}

TEST( Bitmap, SetPopcnt) {
	uint64_t* bitmap = (uint64_t*) calloc(1024, sizeof(uint64_t));

	for (int i = 0; i < 1024; i++) {
		bitmap[i] |= i * 2;
	}

	for (int i = 0; i < 1024; i++) {
		bitmap_setpopcnt(bitmap, i, i);
	}

	for (uint64_t i = 0; i < 1024; i++) {
		ASSERT_EQ(bitmap[i], (i * 2) | (i << 32));
	}

	free(bitmap);
}

TEST( Bitmap, Popcnt) {
	uint64_t* bitmap = (uint64_t*) calloc(1024, sizeof(uint64_t));

	bitmap[5] = ((uint64_t) 3424) << 32 | 7;

	ASSERT_EQ(3427, bitmap_popcnt(bitmap, 164));
	ASSERT_EQ(3427, bitmap_popcnt(bitmap, 163));
	ASSERT_EQ(3426, bitmap_popcnt(bitmap, 162));
	free(bitmap);
}

TEST(Bitmap, Roundup) {
	ASSERT_EQ(0x10, bitmap_roundup(0b1000));
	ASSERT_EQ(0x10, bitmap_roundup(0b1100));
	ASSERT_EQ(0x10, bitmap_roundup(0b1010));
	ASSERT_EQ(0x10, bitmap_roundup(0b1101));

	ASSERT_EQ(0x100, bitmap_roundup(0xfa));
	ASSERT_EQ(0x100, bitmap_roundup(0b10000000));
	ASSERT_EQ(0x100, bitmap_roundup(0b10101101));
	ASSERT_EQ(0x100, bitmap_roundup(0b10000100));

	ASSERT_EQ(0xffffffff, bitmap_roundup(0xf1234567));
	ASSERT_EQ(0xffffffff, bitmap_roundup(0xf0000000));
	ASSERT_EQ(0xffffffff, bitmap_roundup(0x80000000));
	ASSERT_EQ(0xffffffff, bitmap_roundup(0x90000000));
	ASSERT_EQ(0xffffffff, bitmap_roundup(0xa1232133));
}
