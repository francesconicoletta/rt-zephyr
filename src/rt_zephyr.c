#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <zephyr/timing/timing.h>
#include <zephyr/sys_clock.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include "rt_zephyr.h"
#include "rt_zephyr_types.h"

static int p_load = 0;

k_timeout_t delay(int del)
{
	switch(del) {
	case -1:
		return K_FOREVER;
	case 0:
		return K_NO_WAIT;
	default:
		return K_USEC(del);
	}
}

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
}

int calibrate_cpu_cycles(void)
{
	int max_load_loop = 10000, cal_trial = 1000;
	timing_t start_time, end_time;
	uint64_t diff, nsec_per_loop, avg_per_loop = 0;

	timing_init();
	timing_start();

	while (cal_trial) {
		cal_trial--;

		start_time = timing_counter_get();
		waste_cpu_cycles(max_load_loop);
		end_time = timing_counter_get();

		diff = timing_cycles_to_ns(timing_cycles_get(&start_time,
					&end_time));
		nsec_per_loop = diff / max_load_loop;
		avg_per_loop = (avg_per_loop + nsec_per_loop) >> 1;

		if ((abs(nsec_per_loop - avg_per_loop) * 50) < avg_per_loop)
			return avg_per_loop;

		max_load_loop += 3333;
		max_load_loop %= 1000000;
	}

	timing_stop();
	return 0;
}

static inline unsigned long loadwait(unsigned long exec)
{
	unsigned long load_count, secs, perf;
	int i;

	/*
	 * Performace is the fixed amount of work that is performed by this run
	 * phase. We need to compute it here because both load_count and exec
	 * might be modified below.
	 */
	perf = exec / p_load;

	/*
	 * If exec is still too big, let's run it in bursts
	 * so that we don't overflow load_count.
	 */
	secs = exec / 1000000;

	for (i = 0; i < secs; i++) {
		load_count = 1000000000/p_load;
		waste_cpu_cycles(load_count);
		exec -= 1000000;
	}

	/* Run for the remainig exec (if any). */
	load_count = (exec * 1000) / p_load;
	waste_cpu_cycles(load_count);
	return perf;
}

uint32_t run_event(event_data_t *event, unsigned long *perf, log_data_t *ldata)
{
	/* TODO: log data */
	uint32_t start_time = 0, stop_time = 0;
	switch(event->type) {
	case ev_run:
		{
			start_time = k_cycle_get_32();
			*perf += loadwait(event->duration);
			stop_time = k_cycle_get_32();
		}
		break;
	case ev_sleep:
		{
			start_time = k_cycle_get_32();
			k_sleep(K_USEC(event->duration));
			stop_time = k_cycle_get_32();
		}
		break;
	}
	return k_cyc_to_ms_floor32(stop_time - start_time);
}

void run_thread(void *thread_data, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	unsigned long perf = 0;
	uint32_t duration;
	log_data_t ldata = {0};
	thread_data_t *tdata = (thread_data_t *) thread_data;
	event_data_t *events = tdata->events;

	for (int j = 0; j < tdata->loop; j++) {
		for (int i = 0; i < tdata->nbevents; i++) {
			duration = run_event(&events[i], &perf, &ldata);
			printk("event[%d] - %s - duration %lu\n",
					i, events[i].name,
					(unsigned long) duration);
		}
	}
}

uint32_t cycles_per_sec()
{
	uint32_t start_time, stop_time;
	start_time = k_cycle_get_32();
	loadwait(1000000);
	stop_time = k_cycle_get_32();
	printk("%d cycles for 1 sec\n", stop_time - start_time);
	return stop_time - start_time;
}

NEWTHREADSTACK(th1)
NEWTHREADSTACK(th1lo1)
NEWTHREADSTACK(th1lo2)

NEWTHREADSTACK(th2)
NEWTHREADSTACK(th2lo1)

void main(void)
{
	/* nanoseconds per loop */
	p_load = calibrate_cpu_cycles();
	printk("p_load: %d \n", p_load);

#ifdef CONFIG_SCHED_DEADLINE
	uint32_t csec = cycles_per_sec();
#endif

	DEFTHREAD(th1, 2, 0, 4, 1)
	NEWEVENT(th1, "th1 ev0 run", 0, ev_run, 2000000)
	NEWEVENT(th1, "th1 ev1 sleep", 1, ev_sleep, 2000000)

	DEFTHREAD(th2, 2, 0, 6, 1)
	NEWEVENT(th2, "th2 ev0 run", 0, ev_run, 3000000)
	NEWEVENT(th2, "th2 ev1 sleep", 1, ev_sleep, 3000000)

	DEFTHREAD(th1lo1, 2, 4000000, 4, 1)
	NEWEVENT(th1lo1, "th1lo1 ev0 run", 0, ev_run, 2000000)
	NEWEVENT(th1lo1, "th1lo1 ev1 sleep", 1, ev_sleep, 2000000)

	DEFTHREAD(th2lo1, 2, 6000000, 6, 1)
	NEWEVENT(th2lo1, "th2lo1 ev0 run", 0, ev_run, 3000000)
	NEWEVENT(th2lo1, "th2lo1 ev1 sleep", 1, ev_sleep, 3000000)

	DEFTHREAD(th1lo2, 2, 8000000, 4, 1)
	NEWEVENT(th1lo2, "th1lo2 ev0 run", 0, ev_run, 2000000)
	NEWEVENT(th1lo2, "th1lo2 ev1 sleep", 1, ev_sleep, 2000000)
}
