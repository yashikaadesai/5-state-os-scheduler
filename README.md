# 5-State OS Scheduler

This project implements a **simulation of the 5-State Process Model** in C++, designed for CSE 325 (Computer Systems).  
It models how an operating system kernel handles multiple processes using a single CPU core, including preemption, blocking, and process termination.

---
## Overview

This program simulates process management in a single-core system where processes transition among **five distinct states**:

1. **New** – Process created and initialized.  
2. **Ready** – Waiting in the ready queue to be scheduled.  
3. **Running** – Currently executing on the CPU.  
4. **Blocked** – Waiting for I/O or system call completion.  
5. **Terminated** – Execution complete.

The simulation executes one instruction per clock cycle, manages blocked processes, and handles interrupts (hardware and software) according to the specification.

---

## Features

- Reads multiple input process files (`process1`, `process2`, …)  
- Maintains **Ready**, **Running**, and **Blocked** queues  
- Executes one instruction per CPU cycle  
- Simulates **system calls** (`SYS_CALL`) that block a process  
- Implements **timer interrupts** every 5 cycles for preemption  
- Handles **software interrupts** like termination or runtime errors  
- Logs all state transitions and interrupts in a formatted `LOG.txt` file

---

