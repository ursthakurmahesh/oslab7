# Question 2: Process/Thread Synchronization using Semaphores in xv6

## Overview
In Question 1, multiple threads incrementing a shared variable concurrently resulted in severe race conditions (lost updates) because memory operations were not atomic. In this question, we implement **Kernel-Level Counting Semaphores** in xv6 using xv6's `sleep()` and `wakeup()` primitives.

---

## Semaphore Design & Implementation

### Data Structure (`semaphore.h`)
```c
#define MAX_SEM 10

struct semaphore {
  int value;           // Current count (< 0 means abs(value) threads are waiting)
  int active;          // 1 if initialized, 0 if free
  struct spinlock lk;  // Protects the semaphore's internal state
};
```
The kernel maintains a global fixed-size table:
```c
struct semaphore sem_table[MAX_SEM];
```

### System Calls

#### 1. `int sem_init(int sem_id, int value)`
- Validates `0 <= sem_id < MAX_SEM` and `value >= 0`.
- Locks the semaphore spinlock, initializes `value`, sets `active = 1`, and initializes the spinlock.
- Returns `0` on success, `-1` on error.

#### 2. `int sem_wait(int sem_id)`
Classic Dijkstra P operation:
```c
acquire(&sem->lk);
sem->value--;
if (sem->value < 0) {
    // Block the current process/thread until signaled
    sleep(sem, &sem->lk);
}
release(&sem->lk);
```
- Atomic decrement inside spinlock.
- If `value < 0`, the process puts itself to sleep on the channel address `sem`.
- `sleep()` atomically releases `sem->lk` and sleeps. Upon wakeup, re-acquires `sem->lk`.

#### 3. `int sem_signal(int sem_id)`
Classic Dijkstra V operation:
```c
acquire(&sem->lk);
sem->value++;
if (sem->value <= 0) {
    // Wake up one waiting process
    wakeup_one(sem);
}
release(&sem->lk);
```
- Atomic increment inside spinlock.
- If `value <= 0`, at least one process is blocked waiting; `wakeup_one(sem)` transitions one waiting process from `SLEEPING` to `RUNNABLE`.

---

## User Programs

### 1. `synctest.c`
- Modifies Question 1's `threadtest.c` to wrap the shared counter increment in `sem_wait()` and `sem_signal()` using a binary semaphore (mutex initialized to 1):
  ```c
  sem_init(0, 1); // Mutex
  ...
  sem_wait(0);
  shared_counter++; // Critical section protected!
  sem_signal(0);
  ```
- **Result**: Final counter is **exactly 300,000**, with 0 lost updates!

### 2. `prodcons.c`
- Implements classic Bounded-Buffer Producer-Consumer with buffer size 5:
  - Semaphore 0: `mutex = 1` (mutual exclusion for buffer access)
  - Semaphore 1: `empty = 5` (tracks available empty slots)
  - Semaphore 2: `full = 0` (tracks filled slots)
- A producer produces 10 items, and a consumer consumes 10 items.
- Logs every item produced/consumed with current buffer indices.
- Verifies absence of buffer overflow and underflow.

---

## How to Run in xv6

1. Compile and launch QEMU:
   ```bash
   make clean
   make qemu-nox
   ```
2. Test Synchronized Threads:
   ```bash
   $ synctest
   ```
3. Test Producer-Consumer:
   ```bash
   $ prodcons
   ```
