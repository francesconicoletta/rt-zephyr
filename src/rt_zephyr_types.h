#pragma once

typedef struct _log_data_t {
	unsigned long perf;
	unsigned long duration;
	unsigned long c_duration;
	/* TODO: add needed types */
} log_data_t;

/*
 * TODO: check which resource types are needed
 * these are event types
 */
typedef enum resource_t
{
	ev_run,
	ev_sleep
} resource_t;

typedef struct _event_data_t {
	/* TODO order this */
	char name[48];
	resource_t type;
	unsigned long duration;
} event_data_t;

typedef struct _phase_data_t {
	int loop;
	event_data_t *events;
	int nbevents;
} phase_data_t;

typedef struct _thread_data_t {
	char *name;
	int duration;
	int loop;
	int nphases;
	int forked;
	int num_instances;
	phase_data_t *phases;
} thread_data_t;
