#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "../src/Timer.h"

TEST( Timer, Run) {

	Timer* timer = new Timer();
	for (int i = 0; i < 10; i++) {
		timer->start();

		usleep(500000);

		timer->stop();

		ASSERT_TRUE(480 < timer->wallclockms() && timer->wallclockms() < 520);
	}
}
