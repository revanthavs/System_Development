Hello,

This github page contiain my University work experience that is related to System development. 

These are the projects I worked on:

1. Multithread System Simulator
2. Process Management Programs
3. Communication Peer Processes using TCPs and FIFOs

The main objective of these projects are:

Multithread System Simulator:

A C program, that utilizes pthreads to simulate the concurrent execution of 
a set of jobs. The system has a number of resource types, and each resource type has a 
number of available units. All resource units in the system are non-sharable non-preemptable resources.

The Simulator program is assigns one thread to simulate the execution of each job.
Job threads run concurrently in an independent manner, except for synchronizing
with each other when accessing shared resources.

Process Management programs:

Shell programs like Bourne shell (sh), C shell (csh), Bash, and Korn shell (ksh) provide 
powerful interactive programming environments that allow users to utilize many of the
services provided by complex multiprocessing operating systems.

This project gave me experience in using Unix system calls for accessing 
and utilizing system time values, process environment variables, process resource limits,
and process management functions (e.g., fork(), waitpid(), and execl()).

Communication Peer Processes using TCPs and FIFOs:

In recent years, many information technology companies have increased their reliance on data centers
to provide the needed computational and communication services. A data center typically houses hundreds
to tens of thousands hosts. Each host (also called a blade) includes CPU, memory, and disk storage.
The blades are often stacked in racks with each rack holding 20 to 40 blades. At the top of each rack,
there is a packet switch, referred to as the Top of Rack (TOR) switch, that interconnects the blades in
the rack with each other, and provides connectivity with other blades in the data center and the Internet.

In this project, we consider a simplified hypothetical data center network where the TOR switches are 
connected to a special device, called a master switch. The architecture of the network uses a new 
network design paradigm, called software-defined networking where the TOR switches are assumed to be 
simple, fast, and inexpensive devices. The master switch collects and processes information that 
enable it to give routing information to the TOR switches.

This project gave me experience in developing client-server programs that utilize TCP sockets
for communication over the Internet, FIFOs for communication among peers, 
and I/O multiplexing (select or poll) for nonblocking I/O.
