#pragma once

typedef struct _log_data_t {
	unsigned long perf;
	unsigned long duration;
	unsigned long c_duration;
	/* TODO: add needed types */
} log_data_t;

typedef enum resource_t
{
	ev_run,
	ev_sleep
} resource_t;

typedef struct _event_data_t {
	char name[20];
	unsigned long duration;
	resource_t type;
} event_data_t;

typedef struct _thread_data_t {
	char *name;
	event_data_t *events;
	int loop;
	int nbevents;
	int priority;
	k_timeout_t delay;
} thread_data_t;
