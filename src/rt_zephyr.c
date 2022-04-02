#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <zephyr.h>
#include <stdio.h>
#include "rt_zephyr.h"

void waste_cpu_cycles(int load_loops)
{
	double param, result;
	double n;
	unsigned long long i;

	param = 0.95;
	n = 4;
	for (i = 0; i < load_loops; i++) {
		result = ldexp(param, (ldexp(param, ldexp(param, n))));
		result = ldexp(param, (ldexp(param, ldexp(param, n))));
		result = ldexp(param, (ldexp(param, ldexp(param, n))));
		result = ldexp(param, (ldexp(param, ldexp(param, n))));
	}
	return;
}

int calibrate_cpu_cycles(void)
{
	int nsec_per_loop, avg_per_loop = 0;
	int max_load_loop = 10000;
	int cal_trial = 1000;
	int64_t uptime, uptime_d;

	while (cal_trial) {
		cal_trial--;

		uptime = k_uptime_get();
		waste_cpu_cycles(max_load_loop);
		uptime_d = k_uptime_delta(&uptime);

		printf("uptime   : %lld\n", uptime);
		printf("uptime_d : %lld\n", uptime_d);

		nsec_per_loop = uptime_d / max_load_loop;
		avg_per_loop = (avg_per_loop + nsec_per_loop) >> 1;

		printf("nsec: %d\n", nsec_per_loop);
		printf("avg : %d\n", avg_per_loop);

		/* collect a critical mass of samples.*/
		if ((abs(nsec_per_loop - avg_per_loop) * 50) < avg_per_loop)
			return avg_per_loop;

		/*
		* use several loop duration in order to be sure to not
		* fall into a specific platform loop duration
		* (like the cpufreq period)
		*/
		/* randomize the number of loops and recheck 1000 times */
		max_load_loop += 33333;
		max_load_loop %= 1000000;
	}
	return 0;
}
