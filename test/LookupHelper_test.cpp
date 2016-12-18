/*
 * util_test.c
 *
 *  Created on: Nov 20, 2016
 *      Author: Cathy
 */

#include <gtest/gtest.h>
#include "../src/lookup/LookupHelper.h"

TEST( Util, Popcnt) {
	uint64_t a = 0x00000000ffffffff;
	ASSERT_EQ(32, popcount(a));
	uint64_t b = 0xffff00000000000f;
	ASSERT_EQ(4, popcount(b));
}

TEST(Util, Popcnt64) {
	uint64_t a = 0x00000000ffffffff;
	ASSERT_EQ(32, popcount64(a));
	uint64_t b = 0xffff00000000000f;
	ASSERT_EQ(20, popcount64(b));
}
