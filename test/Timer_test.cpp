#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "../src/util/Timer.h"

TEST( Timer, Run) {

	Timer* timer = new Timer();
	for (int i = 0; i < 10; i++) {
		timer->start();

		usleep(50000);

		timer->stop();

		ASSERT_TRUE(48 < timer->wallclockms() && timer->wallclockms() < 52);
	}
	delete timer;
}

TEST(Timer, Interval) {
	Timer* timer = new Timer();
	timer->start();
	for (int i = 0; i < 30; i++) {
		usleep(50000);
		timer->interval("t");
	}
	timer->stop();

	ASSERT_EQ(31, timer->numInterval());
	uint sum = 0;
	for(uint i = 0; i < 30; i++) {
		ASSERT_TRUE(48 < timer->interval(i) && timer->interval(i) < 52);
		sum += timer->interval(i);
	}
	sum += timer->interval(30);
	ASSERT_EQ(sum, timer->wallclockms());
	delete timer;
}
