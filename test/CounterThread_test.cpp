/*
 * CounterThread_test.cpp
 *
 *  Created on: Dec 23, 2016
 *      Author: Cathy
 */

#include <gtest/gtest.h>
#include "../src/join/CounterThread.h"

TEST(CounterThread, Counter) {
	uint input[3227];

	for (int i = 0; i < 3227; i++) {
		if (i % 10)
			input[i] = 2403 + i;
		else
			input[i] = 0;
	}

	NotEqual nz(0);

	ASSERT_EQ(322, CounterThread::count(input, 3227, &nz));
}
