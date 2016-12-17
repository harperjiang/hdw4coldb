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

		usleep(500000);

		timer->stop();

		ASSERT_TRUE(480 < timer->wallclockms() && timer->wallclockms() < 520);
	}
}

TEST(Timer, Interval) {
	Timer* timer = new Timer();
	timer->start();
	for (int i = 0; i < 30; i++) {
		usleep(500000);
		timer->pause();
		timer->resume();
	}
	timer->stop();

	ASSERT_EQ(31, timer->numInterval());
	uint sum = 0;
	for(uint i = 0; i < 30; i++) {
		ASSERT_TRUE(480 < timer->interval(i) && timer->interval(i) < 520);
		sum += timer->interval(i);
	}
	sum += timer->interval(30);
	ASSERT_EQ(sum, timer->wallclockms());
}
