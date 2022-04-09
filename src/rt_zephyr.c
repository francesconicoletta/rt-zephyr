#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <zephyr.h>
#include <stdio.h>
#include "rt_zephyr.h"
#include "rt_zephyr_types.h"

void waste_cpu_cycles(int load_loops)
{
	double param;
	double n;
	double result;
	unsigned long long i;

	param = 0.95;
	n = 4;
	for (i = 0; i < load_loops * 4; i++) {
		result = ldexp(param, (ldexp(param, ldexp(param, n))));
		result = ldexp(param, (ldexp(param, ldexp(param, n))));
		result = ldexp(param, (ldexp(param, ldexp(param, n))));
		result = ldexp(param, (ldexp(param, ldexp(param, n))));
	}
	return;
}

int calibrate_cpu_cycles(void)
{
	int max_load_loop = 1000, cal_trial = 1000;
	uint32_t start_time, stop_time, nsec_spent, nsec_per_loop, avg_per_loop = 0;

	while (cal_trial) {
		cal_trial--;
		start_time = k_cycle_get_32();
		waste_cpu_cycles(max_load_loop);
		stop_time = k_cycle_get_32();

		/* TODO: check overflow */
		nsec_spent = k_cyc_to_ns_floor32(stop_time - start_time);
		nsec_per_loop = nsec_spent / max_load_loop;
		avg_per_loop = (avg_per_loop + nsec_per_loop) >> 1;

		if ((abs(nsec_per_loop - avg_per_loop) * 50) < avg_per_loop)
			return avg_per_loop;

		max_load_loop += 3333;
		max_load_loop %= 100000;
	}

	return 0;
}
