#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "../src/timer.h"

TEST( Timer, Run) {

	timer_token token;
	for (int i = 0; i < 10; i++) {
		timer_start(&token);

		usleep(500000);

		timer_stop(&token);

		ASSERT_TRUE(480 < token.wallclockms && token.wallclockms < 520);
	}
}
