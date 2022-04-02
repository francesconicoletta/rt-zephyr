#include <logging/log.h>
#include <sys/printk.h>
#include <usb/usb_device.h>
#include <zephyr.h>
#include "rt_zephyr.h"

#define STACKSIZE 1024

#define INIT_PRIORITY -1

K_THREAD_STACK_DEFINE(init_thread_stack_area, STACKSIZE);
static struct k_thread init_thread_data;

void init_thread(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	int p_load = 0;

	printk("init_thread started \n");
	p_load = calibrate_cpu_cycles();
	printk("%d", p_load);

	printk("init_thread abort\n");
	k_thread_abort(&init_thread_data);
}

void main(void)
{
	k_thread_create(&init_thread_data, init_thread_stack_area,
			K_THREAD_STACK_SIZEOF(init_thread_stack_area),
			init_thread, NULL, NULL, NULL,
			INIT_PRIORITY, 0, K_FOREVER);
	k_thread_name_set(&init_thread_data, "init_thread");

	k_thread_start(&init_thread_data);
}
