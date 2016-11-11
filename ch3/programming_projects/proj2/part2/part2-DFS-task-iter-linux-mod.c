#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/types.h> // list_head struct.
#include <linux/list.h> // list defs.
#include <linux/sched.h> // task_struct

struct task_struct *task;
struct list_head   *list;

int dfs_task_iter(struct task_struct *parent_task)
{
  struct list_head   *children_list;
  struct task_struct *child_task;

  printk(KERN_INFO "fuk u part 2 dfs\n");

  if(parent_task == NULL) {
    return 1;
  }

  printk(KERN_INFO "fuk u  part 2 dfs Name = %s. State = %ld. PID = %d.\n", parent_task->comm, parent_task->state, parent_task->pid);

  list_for_each(children_list, &parent_task->children) { // for each child.
    child_task = list_entry(children_list, struct task_struct, sibling); // galvins uses sibling here, children would make more sense but it doesn't really affect the code's behaviour, the result is identical , i.e. container_of(sibling) == constainer_of(children)
    //printk(KERN_INFO "fuk u  part 2 dfs Name = %s. State = %ld. PID = %d.\n", child_task->comm, child_task->state, child_task->pid);
    dfs_task_iter(child_task);
    // assuming there are not cycles in this tree, by def. this should be true
  }

  return 0;
}
/* This function is called when the module is loaded. */
int simple_init(void)
{
  printk(KERN_INFO "fuk u part 2 Loading Module\n");
  dfs_task_iter(&init_task);
  return 0;
}

/* This function is called when the module is removed. */
void simple_exit(void) {
  printk(KERN_INFO "fuk u  part 2 Removing Module\n");
}

/* Macros for registering module entry and exit points. */
module_init( simple_init );
module_exit( simple_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("fuk u  part 2 Simple Module");
MODULE_AUTHOR("SGG");

