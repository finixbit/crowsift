#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>

#define MAX_TASK_BUFFER 100

static struct kobject *crowdsift_kobj;

static int taskdump_pid = 0;
static int taskkill_pid = 0;


/**
 * cs_get_task_struct - create a task string 
 * @pid: holds pid to get task
 *
 * The function get data process task using pid
 * into @task_struct. 
 */
static struct task_struct *cs_get_task_struct(unsigned int pid) {
    struct pid *proc_pid = find_vpid(pid);
    struct task_struct *task;

    if(!proc_pid)
        return 0;
    
    task = pid_task(proc_pid, PIDTYPE_PID);
    return task;
}

/**
 * tasklist_show - Get task list as string and place it in a buffer
 * @kobj: kobject struct
 * @attr: kobj_attribute struct
 * @buf: The buffer to place the result into
 *
 * The function loops through the all process task struct and writes
 * into @buf. 
 */
static ssize_t tasklist_show(struct kobject *kobj,
                               struct kobj_attribute *attr,
                               char *buf)
{
    struct task_struct *iter; 
    int task_length = 0;

    for_each_process(iter) {
        if (iter->state > 0) {
            if(task_length > INT_MAX) 
                break;
            else {
                task_length += snprintf(buf+task_length, MAX_TASK_BUFFER,
                                        "%s, %d, %d, %ld\n", 
                                        iter->group_leader->comm, 
                                        iter->pid, 
                                        iter->parent->pid, iter->state);

                // add trailing '\0' length
                task_length += 1;

                pr_info("crowdsift: tasklist ps=%s pid=%d ppid=%d\n", 
                            iter->group_leader->comm, 
                            iter->pid, iter->parent->pid);
            }
        }
    }

    return task_length;
}

/**
 * tasklist_store - Get input to tasklist attribute
 * @kobj: kobject struct
 * @attr: kobj_attribute struct
 * @buf: The buffer to hold input
 * @count: The size of buffer
 *
 * The function get data input into tasklist
 * into @buf. 
 */
static ssize_t tasklist_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                char *buf, size_t count)
{
    int tasklist_pid = 0;
    sscanf(buf, "%du", &tasklist_pid);
    return -EIO;
}

/**
 * tasklist_show - Get kills task and places status in a buffer
 * @kobj: kobject struct
 * @attr: kobj_attribute struct
 * @buf: The buffer to place the result into
 *
 * Get kills task and places status into @buf. 
 */
static ssize_t taskkill_show(struct kobject *kobj,
                               struct kobj_attribute *attr,
                               char *buf)
{
    struct task_struct *task;

    if(taskkill_pid == 0)
        return sprintf(buf, "OK\n");

    task = cs_get_task_struct(taskkill_pid);
    if(!task)
        return sprintf(buf, "ERROR\n");

    kill_pid(task_pid(task), SIGKILL, 1);

    taskkill_pid = 0;
    return sprintf(buf, "OK\n");
}

/**
 * taskkill_store - Get input to taskkill attribute
 * @kobj: kobject struct
 * @attr: kobj_attribute struct
 * @buf: The buffer to hold input
 * @count: The size of buffer
 *
 * The function get data input into taskkill
 * into @buf. 
 */
static ssize_t taskkill_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                char *buf, size_t count)
{
    struct pid *proc_pid;

    taskkill_pid = 0;
    sscanf(buf, "%du", &taskkill_pid);

    if(taskkill_pid > INT_MAX) {
        taskkill_pid = 0;
        return count;
    }

    proc_pid = find_vpid(taskkill_pid);
    if(!proc_pid) {
        taskkill_pid = 0;
        return count;
    }

    return count;
}

/**
 * taskdump_show - Get mem dump of task and places in a buffer
 * @kobj: kobject struct
 * @attr: kobj_attribute struct
 * @buf: The buffer to place the result into
 *
 * Get dump of task and places data into @buf. 
 */
static ssize_t taskdump_show(struct kobject *kobj,
                               struct kobj_attribute *attr,
                               char *buf)
{
    struct task_struct *task;
    struct mm_struct *mm;
    struct vm_area_struct *vma;
    int count = 0;

    if(taskdump_pid == 0)
        return sprintf(buf, "OK\n");

    task = cs_get_task_struct(taskdump_pid);
    if(!task)
        return sprintf(buf, "ERROR\n");

    // get process dump here
    mm = task->mm;

    printk("\nThis mm_struct has %d vmas.\n", mm->map_count);
        for (vma = mm->mmap ; vma ; vma = vma->vm_next) {
                printk ("\nVma number %d: \n", ++count);
                printk("  Starts at 0x%lx, Ends at 0x%lx\n",
                          vma->vm_start, vma->vm_end);
        }

    printk("\nCode  Segment start = 0x%lx, end = 0x%lx \n"
                 "Data  Segment start = 0x%lx, end = 0x%lx\n"
                 "Stack Segment start = 0x%lx\n",
                 mm->start_code, mm->end_code,
                 mm->start_data, mm->end_data,
                 mm->start_stack);

    taskdump_pid = 0;
    return sprintf(buf, "OK\n");
}

/**
 * taskdump_store - Get input to taskdump attribute
 * @kobj: kobject struct
 * @attr: kobj_attribute struct
 * @buf: The buffer to hold input
 * @count: The size of buffer
 *
 * The function get data input into taskdump
 * into @buf. 
 */
static ssize_t taskdump_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                char *buf, size_t count)
{
    struct pid *proc_pid;

    taskdump_pid = 0;
    sscanf(buf, "%du", &taskdump_pid);

    if(taskdump_pid > INT_MAX) {
        taskdump_pid = 0;
        return count;
    }

    proc_pid = find_vpid(taskdump_pid);
    if(!proc_pid) {
        taskdump_pid = 0;
        return count;
    }

    return count;
 }

static struct kobj_attribute tasklist_attribute =
    __ATTR(tasklist, 0660, tasklist_show,
           (void*)tasklist_store);

static struct kobj_attribute taskkill_attribute =
    __ATTR(taskkill, 0660, taskkill_show,
           (void*)taskkill_store);

static struct kobj_attribute taskdump_attribute =
    __ATTR(taskdump, 0660, taskdump_show,
           (void*)taskdump_store);


static int __init crowdsift_init (void)
{
    int error = 0;
    pr_info("crowdsift: init\n");

    crowdsift_kobj =
        kobject_create_and_add("crowdsift", kernel_kobj);
    if (!crowdsift_kobj)
        return -ENOMEM;

    error = sysfs_create_file(crowdsift_kobj, &tasklist_attribute.attr);
    if (error) {
        pr_info("crowdsift: failed to create the crowdsift file " \
               "in /sys/kernel/crowdsift/tasklist\n");
        return -ENOMEM;
    }

    error = sysfs_create_file(crowdsift_kobj, &taskdump_attribute.attr);
    if (error) {
        pr_info("crowdsift: failed to create the crowdsift file " \
               "in /sys/kernel/crowdsift/taskdump\n");
        return -ENOMEM;
    }

    error = sysfs_create_file(crowdsift_kobj, &taskkill_attribute.attr);
    if (error) {
        pr_info("crowdsift: failed to create the crowdsift file " \
               "in /sys/kernel/crowdsift/taskkill\n");
        return -ENOMEM;
    }

    return error;
}

static void __exit crowdsift_exit (void)
{
    pr_info("crowdsift: exit\n");
    kobject_put(crowdsift_kobj);
}

module_init(crowdsift_init);
module_exit(crowdsift_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Crowdsift");
MODULE_DESCRIPTION("Crowdsift driver");