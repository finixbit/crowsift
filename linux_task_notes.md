# Intro
Linux employs a hierarchical scheme in which each process depends on a parent process.
`init` is therefore the root from which all processes originate.


# Process
How this tree structure spreads is closely connected with how new processes are generated. 
For this purpose, Unix uses two mechanisms called fork and exec.
1. `fork` — Generates an exact copy of the current process that differs from the parent process only in its PID (process identification). The memory contents of the initial process are duplicated Linux uses a well-known technique known as copy on write that allows it to make the operation much more efficient by deferring the copy operations until either parent or child writes to a page — read-only
accessed can be satisfied from the same page for both
2. `exec` — Loads a new program into an existing content and then executes it. 
The memory pages reserved by the old program are flushed, and their contents are replaced with new data. The new program then starts executing.


# Threads
sometimes referred to as light-weight processes
a process may consist of several threads that all share the same data and resources 
but take different paths through the program code.


# Namespaces
Every namespace can contain a specific set of PIDs, or can provide different views
of the filesystem, where mounts in one namespace do not propagate into different namespaces


# page tables
Data structures known as page tables are used to map virtual address space to physical address space.
In most cases, a single virtual address space is bigger than the physical RAM available to the system
The preferred method is to use page tables to allocate virtual addresses to physical addresses.


# virtual address (vm of process)
The virtual address spaces of processes are divided into portions of equal size by the kernel. 
These portions are known as pages.Physical memory is also divided into pages of the
same size.


# Physical pages
Physical pages are often called page frames. In contrast, the term page is reserved for pages in virtual
address space.


# System Calls
- Process Management — Creating new tasks, querying information, debugging
- Signals — Sending signals, timers, handling mechanisms
- Files — Creating, opening, and closing files, reading from and writing to files, querying infor-
mation and status
- Directories and Filesystem — Creating, deleting, and renaming directories, querying informa-
tion, links, changing directories
- Protection Mechanisms — Reading and changing UIDs/GIDs, and namespace handling
- Timer Functions — Timer functions and statistical information

```c
<list.h>
struct list_head {
  struct list_head *next, *prev;
};
```

This element could be placed in a data structure as follows:
```c
struct task_struct {
...
struct list_head run_list;
...
};
```

# list_head 
There are several standard functions for handling and processing lists.

## list_add(new, head)
- list_add(new, head) inserts new right after the existing head element.

## list_add_tail(new, head)
- list_add_tail(new, head) inserts new right before the element specified by head . If the list head
is specified for head , this causes the new element to be inserted at the end of the list because of
the cyclic nature of the list (this gives the function its name).

## list_del(entry)
- list_del(entry) deletes an entry from a list.

## list_empty(head)
- list_empty(head) checks if a list is empty, that is, if it does not contain any elements.

## list_splice(list, head)
- list_splice(list, head) combines two lists by inserting the list in list after the head element
of an existing list.

# list_entry
- `list_entry` must be used to find a list element; at first glance, its call syntax appears to be quite
complicated: `list_entry(ptr, type, member)` .   
`ptr` is a pointer to the list_head instance of the data structure, 
`type` is its type, and 
`member` is the element name used for the list element.
The following sample call would be needed to find a task_struct instance of a list:
```c
struct task_struct = list_entry(ptr, struct task_struct, run_list)
```

# list_for_each
Explicit type specification is required because list implementation is not type-safe. 
The list element must be specified to find the correct element if there are data structures that are included
in several lists. 
- `list_for_each(pos, head)` must be used to iterate through all elements of a list. `pos` indicates
the current position in the list, while `head` specifies the list head.
```c
struct list_head *p;
list_for_each(p, &list)
  if (condition)
    return list_entry(p, struct task_struct, run_list);
return NULL;
```

# Generic Kernel Objects
The kobject abstraction indeed allows for using object-oriented techniques in the kernel
The generic kernel object mechanism can be used to perform the following operations on objects:
- Reference counting
- Management of lists (sets) of objects
- Locking of sets
- Exporting object properties into userspace (via the sysfs filesystem)

# struct kobject
```c
<kobject.h>
  struct kobject {
  const char
  struct kref
  struct list_head
  struct kobject
  struct kset
  struct kobj_type
  struct sysfs_dirent
};

struct sample {
...
struct kobject kobj;
...
};
```

# elements of struct kobject
The meanings of the individual elements of struct kobject are as follows:
- `k_name` is a text name exported to userspace using sysfs . Sysfs is a virtual filesystem that allows
for exporting various properties of the system into userspace. 
- `kref` holds the general type struct kref designed to simplify reference management. I discuss
this below.
- `entry` is a standard list element used to group several kobject s in a list (known as a set in this
case).
- `kset` is required when an object is grouped with other objects in a set.
- `parent` is a pointer to the parent element and enables a hierarchical structure to be established
between kobjects.
- `ktype` provides more detailed information on the data structure in which a kobject is
embedded. Of greatest importance is the destructor function that returns the resources of the
embedding data structure.

# kref structure
The layout of the kref structure used to manage references is as follows:
```c
<kref.h>
  struct kref {
  atomic_t refcount;
};
```
refcount is an atomic data type to specify the number of positions in the kernel at which an object is
currently being used. When the counter reaches 0, the object is no longer needed and can therefore be
removed from memory.

# Standard Methods for Processing kobjects
`kobject_get` ,`kobject_put` - Increments or decrements the reference counter of a kobject

# kobject_(un)register        
Registers or removes obj from a hierarchy (the object is added to the existing set (if any) of the parent element; a corresponding entry is created in the sysfs filesystem).

# kobject_init 
Initializes a kobject; that is, it sets the reference counter to its initial value and initializes the list elements of the object.

# kobect_add 
Initializes a kernel object and makes it visible in sysfs

# kobject_cleanup 
Releases the allocated resources when a kobject (and therefore the embedding object) is no longer needed

# Process Descriptors
The kernel maintains info about each process in a process descriptor, of type `task_struct`
See `include/linux/sched.h`
Each process descriptor contains info such as run-state of process, address space, list of open files, process priority etc …

# struct task_struct
```c
struct task_struct {
  volatile long state; /* -1 unrunnable, 0 runnable, >0 stopped */
  unsigned long flags; /* per process flags */
  mm_segment_t addr_limit; /* thread address space:
  0-0xBFFFFFFF for user-thead
  0-0xFFFFFFFF for kernel-thread */
  struct exec_domain *exec_domain;
  long need_resched;
  long counter;
  long priority;
  /* SMP and runqueue state */
  struct task_struct *next_task, *prev_task;
  struct task_struct *next_run, *prev_run;
  ...
  /* task state */
  /* limits */
  /* file system info */
  /* ipc stuff */
  /* tss for this task */
  /* filesystem information */
  /* open file information */
  /* memory management info */
  /* signal handlers */
  ...
};
```

# state values:
n `TASK_RUNNING` (executing on CPU or runnable).
n `TASK_INTERRUPTIBLE` (waiting on a condition: interrupts,
  signals and releasing resources may “wake” process).
n `TASK_UNINTERRUPTIBLE` (Sleeping process cannot be
  woken by a signal).
n `TASK_STOPPED` (stopped process e.g., by a debugger).
n `TASK_ZOMBIE` (terminated before waiting for parent).

# tasks
Each process, or independently scheduled execution context, has its own process descriptor.
Kernel maintains a task array
Processes are dynamic, so descriptors are kept in dynamic memory.

# free_task_struct & alloc_task_struct
free_task_struct() and alloc_task_struct() are used to release & allocate 8KB memory areas to/from the cache

# The Process List
The process list (of all processes in system) is a doubly-linked list. 
`prev_task` & `next_task` fields of process descriptor are used to build list.
`init_task` (i.e., swapper) descriptor is at head of list.
`prev_task` field of init_task points to process descriptor inserted last in the list.
`for_each_task()` macro scans whole list.

# Chained Hashing of PIDs
PIDs are converted to matching process descriptors using a hash function. 
A pidhash table maps PID to descriptor. 
Collisions are resolved by chaining.
`find_task_by_pid()` searches hash table and returns a pointer to a matching process descriptor or NULL.

# Managing the task Array
The task array is updated every time a process is created or destroyed.
A separate list (headed by `tarray_freelist`) keeps track of free elements in the task array.
When a process is destroyed its entry in the task array is added to the head of the freelist.

# Intercepting process creation using syscall
The `init_module` function replaces the appropriate location in `sys_call_table` and keeps the original pointer in a variable. 
The `cleanup_module` function uses that variable to restore everything back to normal