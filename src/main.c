#include <zephyr.h>
#include <sys/printk.h>
#include <logging/log.h>
#include <usb/usb_device.h>

#define STACKSIZE 1024

#define PRIORITY_A 4
#define PRIORITY_B 3
#define PRIORITY_C 2

K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;

K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);
static struct k_thread threadB_data;

K_THREAD_STACK_DEFINE(threadC_stack_area, STACKSIZE);
static struct k_thread threadC_data;

/*
 * TODO: threads runtime statistics
 * https://docs.zephyrproject.org/latest/reference/kernel/threads/index.html#runtime-statistics
 */

void threadA(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	int i = 0;

	printk("threadA: thread started \n");
	k_thread_start(&threadB_data);

	while (1)
	{
		i++;
		printk("threadA: thread loop %d\n", i);
		if (i == 3)
		{
			printk("threadA: thread abort\n");
			k_thread_abort(&threadA_data);
		}
	}
}

void threadB(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	int i = 0;

	printk("threadB: thread started \n");
	k_thread_start(&threadC_data);

	while (1)
	{
		i++;
		printk("threadB: thread loop %d\n", i);
		if (i == 3)
		{
			printk("threadB: thread abort\n");
			k_thread_abort(&threadB_data);
		}
	}

}

void threadC(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	int i = 0;

	printk("threadC: thread started \n");

	while (1)
	{
		i++;
		printk("threadC: thread loop %d\n", i);
		if (i == 3)
		{
			printk("threadC: thread abort\n");
			k_thread_abort(&threadC_data);
		}
	}

}

void main(void)
{
	k_thread_create(&threadA_data, threadA_stack_area,
			K_THREAD_STACK_SIZEOF(threadA_stack_area),
			threadA, NULL, NULL, NULL,
			PRIORITY_A, 0, K_FOREVER);
	k_thread_name_set(&threadA_data, "threadA");

	k_thread_create(&threadB_data, threadB_stack_area,
			K_THREAD_STACK_SIZEOF(threadB_stack_area),
			threadB, NULL, NULL, NULL,
			PRIORITY_B, 0, K_FOREVER);
	k_thread_name_set(&threadB_data, "threadB");

	k_thread_create(&threadC_data, threadC_stack_area,
			K_THREAD_STACK_SIZEOF(threadC_stack_area),
			threadC, NULL, NULL, NULL,
			PRIORITY_C, 0, K_FOREVER);
	k_thread_name_set(&threadC_data, "threadC");

	k_thread_start(&threadA_data);
}
