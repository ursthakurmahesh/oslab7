#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096
#define NUM_THREADS 3
#define INCREMENTS_PER_THREAD 100000

// Shared counter across all threads in the same address space
volatile int shared_counter = 0;

// Thread worker function
void worker(void *arg) {
  int id = (int)(long)arg;
  printf(1, "[Thread %d] Started execution. Incrementing shared_counter %d times...\n", 
         id, INCREMENTS_PER_THREAD);

  for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
    // Critical Section without synchronization:
    // 1. Read shared_counter into register
    // 2. Increment register
    // 3. Write back to shared_counter
    // Interleavings will cause lost updates!
    shared_counter++;
  }

  printf(1, "[Thread %d] Finished increments. Exiting.\n", id);
  exit();
}

int main(int argc, char *argv[]) {
  printf(1, "\n======================================================\n");
  printf(1, "  xv6 User-Level Threads Test: Race Condition Demo     \n");
  printf(1, "======================================================\n");
  printf(1, "Initial shared_counter = %d\n", shared_counter);
  printf(1, "Spawning %d threads, each performing %d increments...\n", 
         NUM_THREADS, INCREMENTS_PER_THREAD);
  printf(1, "Expected theoretical counter value: %d\n\n", NUM_THREADS * INCREMENTS_PER_THREAD);

  void *stacks[NUM_THREADS];
  int tids[NUM_THREADS];

  for (int i = 0; i < NUM_THREADS; i++) {
    // Allocate 1 page for user stack per thread
    stacks[i] = malloc(PGSIZE);
    if (!stacks[i]) {
      printf(1, "Error: Failed to allocate stack for thread %d\n", i + 1);
      exit();
    }

    // Stack grows downwards in x86; pass top of page
    void *stack_top = (char *)stacks[i] + PGSIZE;
    tids[i] = clone(worker, (void *)(long)(i + 1), stack_top);
    if (tids[i] < 0) {
      printf(1, "Error: clone() failed for thread %d\n", i + 1);
      exit();
    }
    printf(1, "[Main] Created Thread %d with PID %d\n", i + 1, tids[i]);
  }

  // Join all threads
  for (int i = 0; i < NUM_THREADS; i++) {
    int joined_pid = join();
    printf(1, "[Main] Reaped Thread PID %d via join()\n", joined_pid);
    free(stacks[i]);
  }

  printf(1, "\n------------------------------------------------------\n");
  printf(1, "Final shared_counter Value: %d\n", shared_counter);
  printf(1, "Expected Value:             %d\n", NUM_THREADS * INCREMENTS_PER_THREAD);
  int lost_updates = (NUM_THREADS * INCREMENTS_PER_THREAD) - shared_counter;
  printf(1, "Lost Updates (due to race): %d\n", lost_updates);
  printf(1, "------------------------------------------------------\n");

  if (shared_counter < (NUM_THREADS * INCREMENTS_PER_THREAD)) {
    printf(1, "Observation: Threads correctly shared memory space (counter is visible\n");
    printf(1, "across all threads), but lost updates occurred due to lack of synchronization.\n");
    printf(1, "This verifies Question 1 requirements and motivates Question 2 (Semaphores)!\n\n");
  } else {
    printf(1, "Unexpected: No race condition detected in this run.\n\n");
  }

  exit();
}
