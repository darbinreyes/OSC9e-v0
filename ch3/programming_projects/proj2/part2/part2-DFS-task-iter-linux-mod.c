#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/types.h> // list_head struct.
#include <linux/list.h> // list defs.
#include <linux/sched.h> // task_struct

struct task_struct *task;

/* This function is called when the module is loaded. */
int simple_init(void)
{
  printk(KERN_INFO "fuk u Loading Module\n");

  for_each_process(task) {
    printk(KERN_INFO "fuk u process for each Name = %s. State = %ld. PID = %d.!\n", task->comm, task->state, task->pid);
  }

  return 0;
}

/* This function is called when the module is removed. */
void simple_exit(void) {
  printk(KERN_INFO "fuk u Removing Module\n");
}

/* Macros for registering module entry and exit points. */
module_init( simple_init );
module_exit( simple_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("fuk u Simple Module");
MODULE_AUTHOR("SGG");

