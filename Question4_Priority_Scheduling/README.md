# Question 4: Priority-Based Preemptive CPU Scheduling in xv6

## Overview
By default, xv6 implements a simple Round-Robin CPU scheduler without process priorities. In this question, we replace the default scheduler with a **Priority-Based Preemptive Scheduler featuring Dynamic Aging** to prevent starvation of low-priority tasks.

---

## Priority & Aging Specifications

- **Priority Range**: `1` (highest priority) to `20` (lowest priority).
- **Default Priority**: New processes are assigned a default priority of `10`.
- **Preemption**: On every timer interrupt (tick), running processes yield the CPU via `yield()`, allowing the scheduler to pick the highest priority `RUNNABLE` process.
- **Tie-Breaking**: Ties between processes of identical priority are resolved using round-robin ordering.
- **Aging Mechanism**:
  - Each process tracks `wait_ticks` spent in the `RUNNABLE` state.
  - Every timer tick, `update_aging()` increments `wait_ticks` for all `RUNNABLE` processes.
  - When a `RUNNABLE` process waits for **30 ticks** without being scheduled:
    - Its priority is promoted by 1 (numerical value decremented: e.g., $20 \to 19$).
    - `wait_ticks` is reset to 0.
    - Capped at maximum priority `1`.
  - When scheduled, the process's `wait_ticks` is reset to 0.

---

## System Calls Implemented

### 1. `int set_priority(int pid, int priority)`
- Validates `1 <= priority <= 20`.
- Searches `ptable` for process matching `pid`.
- Sets new priority, resets `wait_ticks = 0`.
- Returns the previous priority value on success, `-1` on error.

### 2. `int get_priority(int pid)`
- Searches `ptable` for process matching `pid`.
- Returns current priority, or `-1` if not found.

---

## User Test Program: `priotest.c`
- Forks 4 CPU-bound child processes with distinct priorities:
  - **Child 1**: Priority `5` (High)
  - **Child 2**: Priority `10` (Default / Medium-High)
  - **Child 3**: Priority `15` (Medium-Low)
  - **Child 4**: Priority `20` (Lowest)
- Each child executes an intensive numerical workload.
- **Observed Behavior**:
  - Higher priority processes finish significantly earlier:
    `Child 1 (Pri 5) -> Child 2 (Pri 10) -> Child 3 (Pri 15) -> Child 4 (Pri 20)`.
  - Child 4 does **not** starve; the kernel console prints dynamic aging boosts:
    `[AGING] PID 7 waited 30 ticks -> priority boosted to 19`
    `[AGING] PID 7 waited 30 ticks -> priority boosted to 18`
    and Child 4 completes successfully!

---

## How to Run in xv6

1. Compile and run in QEMU:
   ```bash
   make clean
   make qemu-nox
   ```
2. Execute priority test:
   ```bash
   $ priotest
   ```
