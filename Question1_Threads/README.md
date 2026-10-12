# Question 1: Implementing User-Level Threads in xv6

## Overview
xv6 natively supports only `fork()`, where every new process receives its own completely isolated address space (`pgdir`). In this question, we implement kernel-supported threads that **share the same address space** while maintaining separate execution contexts (kernel stacks, user stacks, and trap frames).

---

## Key System Calls Implemented

### 1. `int clone(void(*fcn)(void*), void *arg, void *stack)`
Creates a new thread:
- **Address Space Sharing**: Points `np->pgdir = curproc->pgdir` and `np->sz = curproc->sz` without allocating or copying new page tables.
- **Dedicated Kernel Stack**: Allocates a separate kernel stack via `kalloc()` to allow independent kernel execution and trap handling.
- **Trap Frame Initialization**: Copies the caller's trapframe `*np->tf = *curproc->tf` and reconfigures:
  - `np->tf->esp = (uint)stack - 8;` (aligned stack pointer)
  - `*(uint*)(np->tf->esp + 4) = (uint)arg;` (first argument for x86 cdecl)
  - `*(uint*)(np->tf->esp) = 0xffffffff;` (fake return address)
  - `np->tf->eip = (uint)fcn;` (entry point function address)
- **Mark Process as Thread**: Sets `np->is_thread = 1; np->parent = curproc;`.
- **Descriptor Duplication**: Duplicates file descriptors (`filedup`) and cwd (`idup`).
- Returns the thread's `pid`.

### 2. `int join(void)`
Waits for an active thread created by `clone()`:
- Scans the process table for children with `p->is_thread == 1 && p->parent == curproc`.
- When a thread enters `ZOMBIE` state:
  - Frees only its kernel stack (`kfree(p->kstack)`).
  - **Crucial**: Leaves `p->pgdir` intact (does **NOT** call `freevm(p->pgdir)`), preserving the shared address space for the parent and peer threads.
  - Resets proc table entry to `UNUSED`.
  - Returns reaped thread `pid`.
- If thread children are still running, caller sleeps on `&ptable.lock`.

---

## Kernel Modifications

### `proc.h`
Added thread metadata flag to `struct proc`:
```c
int is_thread;               // 1 if thread created via clone(), 0 if normal process
```

### `proc.c`
- Implemented `clone()` and `join()`.
- Modified `exit()`: If a thread calls `exit()`, wakes up parent without corrupting shared resources.
- Modified `wait()`: When cleaning up a zombie child process, only calls `freevm(p->pgdir)` if `!p->is_thread`.

---

## User Test Program: `threadtest.c`
The user program creates **3 threads** using `clone()`. Each thread increments a global counter `100,000` times.

```c
volatile int counter = 0;

void worker(void *arg) {
    for (int i = 0; i < 100000; i++) {
        counter++; // Shared memory increment without locks -> Race condition!
    }
    exit();
}
```

### Observed Behavior (Race Condition)
Since x86 `counter++` compiles into multiple machine instructions:
```asm
movl counter, %eax
addl $1, %eax
movl %eax, counter
```
Context switches occur mid-operation, resulting in lost updates. The final counter is significantly less than the expected `300,000` (typically ~`180,000` - `220,000`). This demonstrates shared address space and sets up Question 2 (Synchronization).

---

## How to Run in xv6

1. Apply the kernel modifications or copy the files into xv6 root.
2. Compile and launch QEMU:
   ```bash
   make clean
   make qemu-nox
   ```
3. Run the user test:
   ```bash
   $ threadtest
   ```
