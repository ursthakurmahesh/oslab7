# Operating System Assignment 7: Advanced xv6 Enhancements

[![xv6](https://img.shields.io/badge/OS-xv6--x86-blue.svg)](https://github.com/mit-pdos/xv6-public)
[![Language](https://img.shields.io/badge/Language-C%20%7C%20x86%20Assembly-orange.svg)](#)
[![Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)](#)

A comprehensive implementation of core operating system concepts in the **MIT xv6 Unix** kernel:
1. **User-Level Threads with Shared Address Space** (`clone` & `join`)
2. **Kernel Counting Semaphores & Concurrency Control** (`sem_init`, `sem_wait`, `sem_signal`)
3. **Deadlock Detection & Banker's Algorithm Avoidance** (Simulated Matrices & xv6 IPC Pipes)
4. **Preemptive Priority CPU Scheduling with Dynamic Aging** (`set_priority`, `get_priority`, Aging)

---

## 📁 Repository Structure

```
final/
├── README.md                                  <-- Master GitHub Documentation (this file)
│
├── Consolidated_Report/                       <-- 2-3 Page Consolidated Academic Report
│   ├── REPORT.md                              <-- Complete Report (Design, Data Structures, Analyses)
│   └── scheduling_graph.txt                   <-- ASCII Scheduling Graphs & Metric Tables
│
├── Question1_Threads/                         <-- Question 1: Kernel-Supported Threads
│   ├── README.md                              <-- Documentation & Architecture for Q1
│   ├── threadtest.c                           <-- User test: 3 threads, race condition demonstration
│   ├── output.txt                             <-- Terminal execution transcript
│   ├── proc.c                                 <-- clone(), join(), thread-safe exit() & wait()
│   ├── proc.h                                 <-- struct proc with is_thread flag
│   ├── sysproc.c                              <-- sys_clone(), sys_join()
│   ├── syscall.c                              <-- Syscall dispatcher entries
│   ├── syscall.h                              <-- SYS_clone (22), SYS_join (23)
│   ├── defs.h                                 <-- Kernel function prototypes
│   ├── user.h                                 <-- User-space system call definitions
│   ├── usys.S                                 <-- Assembly syscall stubs
│   └── Makefile                               <-- Makefile configuration instructions
│
├── Question2_Synchronization/                 <-- Question 2: Semaphores & Concurrency
│   ├── README.md                              <-- Documentation & Semaphore Mechanics for Q2
│   ├── semaphore.h                            <-- struct semaphore definition
│   ├── semaphore.c                            <-- Kernel semaphore implementation (sleep/wakeup)
│   ├── synctest.c                             <-- Synchronized counter test (Exact 300,000)
│   ├── prodcons.c                             <-- Bounded buffer Producer-Consumer (Capacity 5)
│   ├── output.txt                             <-- Terminal output for synctest & prodcons
│   ├── proc.c                                 <-- wakeup_one() implementation
│   ├── proc.h                                 <-- Thread & proc headers
│   ├── sysproc.c                              <-- sys_sem_init, sys_sem_wait, sys_sem_signal
│   ├── syscall.c                              <-- Syscall dispatcher
│   ├── syscall.h                              <-- SYS_sem_init (24), SYS_sem_wait (25), SYS_sem_signal (26)
│   ├── defs.h                                 <-- Kernel prototypes
│   ├── user.h                                 <-- User prototypes
│   ├── usys.S                                 <-- Syscall assembly stubs
│   └── Makefile                               <-- Makefile configuration instructions
│
├── Question3_Deadlock_Detection/              <-- Question 3: Deadlock Detection & Avoidance
│   ├── README.md                              <-- Documentation, algorithms & trace tables
│   ├── deadlock.c                             <-- Complete detection, Banker's avoidance & pipe simulation
│   ├── output.txt                             <-- Terminal output for 3 test matrices + pipe circular wait
│   ├── user.h                                 <-- User header
│   └── Makefile                               <-- Makefile configuration instructions
│
└── Question4_Priority_Scheduling/             <-- Question 4: Priority Scheduler & Aging
    ├── README.md                              <-- Documentation, tie-breaking & aging mechanics
    ├── priotest.c                             <-- User test: 4 CPU-bound processes with distinct priorities
    ├── output.txt                             <-- Terminal output showing aging boosts & completion order
    ├── proc.c                                 <-- Priority scheduler, tie-breaking, update_aging()
    ├── proc.h                                 <-- struct proc with priority & wait_ticks
    ├── trap.c                                 <-- Timer interrupt hook calling update_aging()
    ├── sysproc.c                              <-- sys_set_priority(), sys_get_priority()
    ├── syscall.c                              <-- Syscall dispatcher
    ├── syscall.h                              <-- SYS_set_priority (22), SYS_get_priority (23)
    ├── defs.h                                 <-- Kernel prototypes
    ├── user.h                                 <-- User prototypes
    ├── usys.S                                 <-- Assembly stubs
    └── Makefile                               <-- Makefile configuration instructions
```

---

## 📌 Summary of Questions & Implementations

### [Question 1: Implementing User-Level Threads](Question1_Threads/)
- **System Calls**:
  - `int clone(void(*fcn)(void*), void *arg, void *stack)`: Creates a new thread sharing the caller's page directory (`np->pgdir = curproc->pgdir`), sets up an independent kernel stack via `kalloc()`, and formats the user stack with argument and return address according to x86 cdecl conventions.
  - `int join(void)`: Reaps zombie child threads, freeing their kernel stack while carefully preserving the shared page directory.
- **Kernel Modifications**:
  - `struct proc`: Added `int is_thread;` to identify threads.
  - `exit()`, `wait()`, `fork()`: Safeguarded against freeing `pgdir` when cleaning up threads.
- **Verification (`threadtest.c`)**:
  - 3 threads concurrently increment a global counter 100,000 times.
  - **Outcome**: Threads share the same address space. Without synchronization, non-atomic increments lead to lost updates (Final value: `187,429` vs. `300,000`).

---

### [Question 2: Process/Thread Synchronization using Semaphores](Question2_Synchronization/)
- **Kernel Counting Semaphores**:
  - Fixed-size array of 10 semaphores (`struct semaphore`) protected by spinlocks.
  - Utilizes xv6's `sleep()` and a new `wakeup_one()` function to block and wake waiting threads without race conditions or thundering herds.
- **System Calls**:
  - `int sem_init(int sem_id, int value)`: Initializes semaphore count.
  - `int sem_wait(int sem_id)`: Decrements counter; blocks if `value < 0`.
  - `int sem_signal(int sem_id)`: Increments counter; wakes one sleeping thread if `value <= 0`.
- **User Tests**:
  - `synctest.c`: Wraps shared counter increment in `sem_wait`/`sem_signal`. Counter achieves **exactly 300,000** (0 lost updates).
  - `prodcons.c`: Classic Producer-Consumer bounded buffer (size 5) using `empty`, `full`, and `mutex` semaphores. Confirms zero buffer overflows or underflows.

---

### [Question 3: Deadlock Detection & Banker's Avoidance](Question3_Deadlock_Detection/)
- **Algorithms**:
  - **Deadlock Detection** (`detect_deadlock()`): Finds processes whose requests cannot be satisfied, simulating release of allocations. Returns the list of deadlocked processes.
  - **Banker's Algorithm for Deadlock Avoidance** (`is_safe_state()`, `request_resource()`): Evaluates hypothetical resource allocations against maximum claims to guarantee that the system remains in a safe state.
- **Test Scenarios**:
  1. *Deadlocked State*: Circular dependency detected across $\{P_1, P_2, P_3, P_4\}$.
  2. *Safe State*: Valid safe sequence found: $P_1 \to P_3 \to P_4 \to P_0 \to P_2$.
  3. *Unsafe State Avoidance*: Process $P_4$ requests $[3, 3, 0]$; state would become unsafe; request is rejected and rolled back.
  4. *Real xv6 IPC Pipe Deadlock*: Simulates processes $A$ and $B$ circularly blocked on pipe read/write descriptors; algorithm correctly isolates circular wait.

---

### [Question 4: Priority-Based Preemptive CPU Scheduling](Question4_Priority_Scheduling/)
- **Priority Architecture**:
  - Priority levels from `1` (highest) to `20` (lowest); default priority = `10`.
  - System calls: `set_priority(pid, priority)` and `get_priority(pid)`.
- **Preemptive Scheduler**:
  - Modifies `scheduler()` in `proc.c` to select the `RUNNABLE` process with the lowest numerical priority.
  - Breaks ties using **Round-Robin** ordering.
  - Preemption triggered on every timer interrupt via `yield()`.
- **Starvation Prevention via Aging**:
  - Tracks `wait_ticks` per process.
  - Every timer tick, `update_aging()` increments `wait_ticks` for all waiting `RUNNABLE` processes.
  - If a process waits **30 ticks**, its priority is boosted by 1 ($20 \to 19 \to \dots$).
- **Verification (`priotest.c`)**:
  - 4 processes with priorities 5, 10, 15, and 20 execute CPU workloads.
  - Higher-priority processes complete first; lowest-priority process receives dynamic aging boosts and completes safely without starving.

---

## 📊 Summary Comparison: Synchronization & Scheduling Results

### Q1 vs Q2 Race Condition Results
| Program | Synchronization | Expected Count | Actual Count | Lost Updates | Status |
| :--- | :--- | :---: | :---: | :---: | :---: |
| `threadtest` | None (Unsynchronized) | 300,000 | **187,429** | 112,571 | Race Condition Demonstrated |
| `synctest` | Semaphore Mutex | 300,000 | **300,000** | **0** | **Exact Synchronization** |

### Q4 Priority & Aging Metrics
| Process | Initial Priority | Final Priority | Aging Boosts | Completion Order | Starvation Prevented? |
| :---: | :---: | :---: | :---: | :---: | :---: |
| Child 1 | 5 (Highest) | 5 | 0 | 1st | Yes |
| Child 2 | 10 (Default) | 10 | 0 | 2nd | Yes |
| Child 3 | 15 | 15 | 0 | 3rd | Yes |
| Child 4 | 20 (Lowest) | **16** | **4** | 4th | **Yes (Completed via Aging)** |

---

## 🚀 How to Build and Run in xv6

### Prerequisites
- GCC cross-compiler (`gcc-multilib` / `i386-elf-gcc`)
- QEMU (`qemu-system-i386`)

### Running Any Question
1. Navigate to the desired question directory (e.g., `Question1_Threads/`).
2. Copy the modified kernel source files into your xv6 repository root.
3. Clean and build xv6 with QEMU:
   ```bash
   make clean
   make qemu-nox
   ```
4. Run the corresponding test program in the xv6 terminal:
   ```bash
   $ threadtest   # For Question 1
   $ synctest     # For Question 2
   $ prodcons     # For Question 2
   $ deadlock     # For Question 3
   $ priotest     # For Question 4
   ```
5. To exit QEMU: Press `Ctrl+A` then `X`.

---

## 📄 Consolidated Report
For complete in-depth documentation, design decisions, data structures, and analysis, see the [Consolidated Report](Consolidated_Report/REPORT.md).
