#pragma once

#define XSTR(s) STR(s)
#define STR(s) #s

#define STACKSIZE	1024
#define MAX_PHASES	5
#define MAX_EVENTS	5

#define NEWTHREADSTACK(t_name)							\
	K_THREAD_STACK_DEFINE(thread_stack_##t_name, STACKSIZE);		\
	struct k_thread thread_data_##t_name;

#define CRTHREAD(t_name)							\
	thread_tid_##t_name = k_thread_create(&thread_data_##t_name,		\
			thread_stack_##t_name,					\
			K_THREAD_STACK_SIZEOF(thread_stack_##t_name),		\
			run_thread, &thread_##t_name, NULL, NULL,		\
			thread_##t_name.priority, 0, thread_##t_name.delay);	\
	k_thread_name_set(&thread_data_##t_name, thread_##t_name.name);

#define DEFTHREAD(t_name, evn, del, lo, pri)					\
	static k_tid_t thread_tid_##t_name;					\
	static thread_data_t thread_##t_name;					\
	static event_data_t events_##t_name[MAX_EVENTS];			\
	thread_##t_name.events = events_##t_name;				\
	thread_##t_name.nbevents = evn;						\
	thread_##t_name.delay = delay(del);					\
	thread_##t_name.loop = lo;						\
	thread_##t_name.priority = pri;						\
	thread_##t_name.name = XSTR(t_name);					\
	CRTHREAD(t_name)

#define STARTTHREAD(t_name)	k_thread_start(&thread_data_##t_name);

#define NEWEVENT(t_name, ev_name, num, ev_type, usecs)				\
	events_##t_name[num].name = XSTR(ev_name);				\
	events_##t_name[num].type = ev_type;					\
	events_##t_name[num].duration = usecs;
