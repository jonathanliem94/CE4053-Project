# CE4053-Project
CE4053: Micrium OS Modding

This project utilises the Real-Time Operating System of µC/OS-III, found here at https://www.micrium.com/rtos/ .

We have made modifications to the Operating System(OS), such that the OS now uses Earliest-Deadline First (EDF) Scheduling, instead of the original priority-based scheduler, which may not be suitable for some embedded applications.
The modification does NOT replace the original implementation of the ready list and priority bitmap.
Instead, the EDF Scheduler makes use of an additional data structure, the Adelson-Velsky and Landis (AVL) tree. 
More information about the tree can be found here at https://en.wikipedia.org/wiki/AVL_tree .

Another modification made to the OS is the addition of a Resource Sharing Protocol.
This is to prevent deadlocks when mutexes are taken by differing processes.
The protocol we have added into the OS is the Stack Resource Protocol (SRP).
Additionally, to manage resource usage, we have added stacks to keep track of the order of when the resources are taken, and thus, when they should be released.
A red-black tree is added to the system to store and manage blocked tasks. More information here: https://en.wikipedia.org/wiki/Red%E2%80%93black_tree .

All these additions to the base OS, will of course produce overheads, and we have assessed them in the reports in the Reports folder.

All these modifications serve to prevent deadlock in a system where there are multiple recursive processes. 
This build has been tested on a Stellaris EVALBOT Robotic Evaluation Board, listed here http://www.ti.com/lit/ml/spmu168/spmu168.pdf . 

Contributors:
Jonathan L.
Nicholas K.

