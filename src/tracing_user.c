#include <tracing_user.h>

void sys_trace_thread_switched_in_user(struct k_thread *thread)
{
	printk("%s: %p\n", __func__, thread);
}

void sys_trace_thread_switched_out_user(struct k_thread *thread)
{
	printk("%s: %p\n", __func__, thread);
}
