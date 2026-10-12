#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "semaphore.h"

// Fixed-size array of 10 counting semaphores
struct semaphore sem_table[MAX_SEM];
struct spinlock sem_table_lock;

// Initialize kernel semaphore subsystem
void
sem_init_system(void)
{
  initlock(&sem_table_lock, "sem_table");
  for (int i = 0; i < MAX_SEM; i++) {
    initlock(&sem_table[i].lk, "semaphore");
    sem_table[i].value = 0;
    sem_table[i].active = 0;
  }
}

// System Call: sem_init(int sem_id, int value)
// Initializes semaphore sem_id with the given value
int
sem_init(int sem_id, int value)
{
  if (sem_id < 0 || sem_id >= MAX_SEM || value < 0)
    return -1;

  acquire(&sem_table_lock);
  acquire(&sem_table[sem_id].lk);

  sem_table[sem_id].value = value;
  sem_table[sem_id].active = 1;

  release(&sem_table[sem_id].lk);
  release(&sem_table_lock);
  return 0;
}

// System Call: sem_wait(int sem_id)
// Decrements the semaphore; blocks if value < 0
int
sem_wait(int sem_id)
{
  if (sem_id < 0 || sem_id >= MAX_SEM)
    return -1;

  acquire(&sem_table[sem_id].lk);
  if (!sem_table[sem_id].active) {
    release(&sem_table[sem_id].lk);
    return -1;
  }

  // Dijkstra P operation: decrement first
  sem_table[sem_id].value--;

  // If value < 0, all available units are exhausted; sleep until signaled
  if (sem_table[sem_id].value < 0) {
    sleep(&sem_table[sem_id], &sem_table[sem_id].lk);
  }

  release(&sem_table[sem_id].lk);
  return 0;
}

// System Call: sem_signal(int sem_id)
// Increments the semaphore; wakes a waiting process if any
int
sem_signal(int sem_id)
{
  if (sem_id < 0 || sem_id >= MAX_SEM)
    return -1;

  acquire(&sem_table[sem_id].lk);
  if (!sem_table[sem_id].active) {
    release(&sem_table[sem_id].lk);
    return -1;
  }

  // Dijkstra V operation: increment
  sem_table[sem_id].value++;

  // If value <= 0, at least one process was blocked on this semaphore
  if (sem_table[sem_id].value <= 0) {
    wakeup_one(&sem_table[sem_id]);
  }

  release(&sem_table[sem_id].lk);
  return 0;
}
