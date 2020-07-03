# crowdsift
List, Kill and Dump linux processes directly from the task_list.

## Requirements
```
sudo apt-get install linux-headers-$(uname -r)
```

## Features
1. Dump the kernel task_list containing all the process task struct
```sh
$ cat /sys/kernel/crowdsift/tasklist
```

2. Dump the memory of a task
```sh
echo $PID > /sys/kernel/crowdsift/taskdump
cat /sys/kernel/crowdsift/taskdump
```

3. Kill a process task within the task_list
```sh
echo $PID > /sys/kernel/crowdsift/taskkill
cat /sys/kernel/crowdsift/taskkill
```
