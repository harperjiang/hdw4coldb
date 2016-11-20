/*
 * util_test.c
 *
 *  Created on: Nov 20, 2016
 *      Author: Cathy
 */

#include <gtest/gtest.h>
#include "../src/util.h"

TEST( Util, Popcnt) {
	uint64_t a = 0x00000000ffffffff;
	ASSERT_EQ(32, popcount(a));
	uint64_t b = 0xffff00000000000f;
	ASSERT_EQ(4, popcount(b));
}
