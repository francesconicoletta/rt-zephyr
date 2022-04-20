#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys_clock.h>
#include <timing/timing.h>
#include <time.h>
#include <zephyr.h>
#include "kernel.h"
#include "rt_zephyr.h"
#include "rt_zephyr_types.h"

#define STACKSIZE 1024
#define INIT_PRIORITY -1

static int p_load = 0;

K_THREAD_STACK_DEFINE(init_thread_stack_area, STACKSIZE);
static struct k_thread init_thread_data;

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

int run_event(event_data_t *event, unsigned long *perf, log_data_t *ldata)
{
	/* TODO: log data duration */
	unsigned long lock = 1;
	switch(event->type) {
	case ev_run:
		{
			uint32_t start_time, stop_time, duration;
			start_time = k_cycle_get_32();
			*perf += loadwait(event->duration);
			stop_time = k_cycle_get_32();
			duration = k_cyc_to_ms_floor32(stop_time - start_time);
			printk("ran for %lu ms\n", (unsigned long) duration);
		}
		break;
	case ev_sleep:
		{
			uint32_t start_time, stop_time, duration;
			start_time = k_cycle_get_32();
			k_sleep(K_USEC(event->duration));
			stop_time = k_cycle_get_32();
			duration = k_cyc_to_ms_floor32(stop_time - start_time);
			printk("slept for %lu ms\n", (unsigned long) duration);
		}
		break;
	}
	return lock;
}

int run(phase_data_t *pdata, log_data_t *ldata)
{
	unsigned long perf = 0;
	int i;
	
	event_data_t *events = pdata->events;
	int nbevents = pdata->nbevents;

	for (i = 0; i < nbevents; i++) {
		/*
		 * TODO:
		 * - fix this
		 *
		 * if (!continue_running && !lock)
		 * 	return perf;
		 * log_debug("[%d] runs events %d type %d ", ind, i, events[i].type);
		 * lock += run_event(&events[i], !continue_running, &perf,
		 *	  tdata, t_first, ldata);
		 */
		printk("Event[%d]\n", i);
		run_event(&events[i], &perf, ldata);
	}

	return perf;
}

void init_thread(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	printk("init_thread started \n");
	/* nanoseconds per loop */
	p_load = calibrate_cpu_cycles();
	printk("p_load: %d \n", p_load);

	/*
	 * from rt-app
	 * Get the 1st phase's data
	 * pdata = &data->phases[0];
	 *
	 * TODO: pdata should be populated by a config file
	 */
	phase_data_t pdata;

	event_data_t event[3];
	memset(event, 0, sizeof(event));

	strcpy(event[0].name, "prova");
	event[0].type = ev_run;
	event[0].duration = 2000000;

	strcpy(event[1].name, "prova");
	event[1].type = ev_sleep;
	event[1].duration = 3000000;

	pdata.events = event;
	pdata.nbevents = 2;
	log_data_t ldata = {0};

	run(&pdata, &ldata);
	printk("init_thread abort\n");
	k_thread_abort(&init_thread_data);
}

void main(void)
{
	k_tid_t init_id;
	k_thread_runtime_stats_t rt_stats_thread;

	/*
	 * TODO:
	 * 1. read thread duration, priority, delay etc from config file
	 * 2. one between main and init_thread should only spawn other threads
	 */
	init_id = k_thread_create(&init_thread_data, init_thread_stack_area,
			K_THREAD_STACK_SIZEOF(init_thread_stack_area),
			init_thread, NULL, NULL, NULL,
			INIT_PRIORITY, 0, K_FOREVER);
	k_thread_name_set(&init_thread_data, "init_thread");

	k_thread_start(&init_thread_data);

	k_thread_runtime_stats_get(init_id, &rt_stats_thread);

	/*
	 * with CONFIG_SCHED_THREAD_USAGE_ANALYSIS we can use current_cycles,
	 * peak_cycles, average_cycles
	 * printk("Cycles: %llu\n", rt_stats_thread.execution_cycles);
	 */
}
