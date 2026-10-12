#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_P 10
#define MAX_R 10

// Print matrix helper
void print_matrices(int n, int m, int alloc[MAX_P][MAX_R], int req[MAX_P][MAX_R], int avail[MAX_R], const char *req_label) {
  printf(1, "\nProcess |  Allocation     |  %-12s   |  Available\n", req_label);
  printf(1, "--------+-----------------+-----------------+-----------------\n");
  for (int i = 0; i < n; i++) {
    printf(1, "   P%-2d  |  ", i);
    for (int j = 0; j < m; j++) printf(1, "%d ", alloc[i][j]);
    printf(1, "       |  ");
    for (int j = 0; j < m; j++) printf(1, "%d ", req[i][j]);
    printf(1, "       |  ");
    if (i == 0) {
      for (int j = 0; j < m; j++) printf(1, "%d ", avail[j]);
    }
    printf(1, "\n");
  }
  printf(1, "\n");
}

// =========================================================================
// Task b: Deadlock Detection Algorithm
// =========================================================================
int detect_deadlock(int n, int m, int alloc[MAX_P][MAX_R], int req[MAX_P][MAX_R], int avail[MAX_R], int deadlocked_list[MAX_P]) {
  int work[MAX_R];
  int finish[MAX_P];
  int num_deadlocked = 0;

  for (int j = 0; j < m; j++)
    work[j] = avail[j];

  for (int i = 0; i < n; i++) {
    int has_alloc = 0;
    for (int j = 0; j < m; j++) {
      if (alloc[i][j] > 0) {
        has_alloc = 1;
        break;
      }
    }
    finish[i] = (has_alloc == 0) ? 1 : 0;
  }

  // Step 2 & 3: Find process whose Request <= Work
  int found;
  do {
    found = 0;
    for (int i = 0; i < n; i++) {
      if (!finish[i]) {
        int can_satisfy = 1;
        for (int j = 0; j < m; j++) {
          if (req[i][j] > work[j]) {
            can_satisfy = 0;
            break;
          }
        }
        if (can_satisfy) {
          for (int j = 0; j < m; j++)
            work[j] += alloc[i][j];
          finish[i] = 1;
          found = 1;
        }
      }
    }
  } while (found);

  // Step 4: Identify deadlocked processes
  for (int i = 0; i < n; i++) {
    if (!finish[i]) {
      deadlocked_list[num_deadlocked++] = i;
    }
  }

  return num_deadlocked;
}

// =========================================================================
// Task d: Banker's Algorithm Safety Algorithm
// =========================================================================
int is_safe_state(int n, int m, int alloc[MAX_P][MAX_R], int need[MAX_P][MAX_R], int avail[MAX_R], int safe_seq[MAX_P]) {
  int work[MAX_R];
  int finish[MAX_P];
  int count = 0;

  for (int j = 0; j < m; j++)
    work[j] = avail[j];

  for (int i = 0; i < n; i++)
    finish[i] = 0;

  int found;
  do {
    found = 0;
    for (int i = 0; i < n; i++) {
      if (!finish[i]) {
        int can_satisfy = 1;
        for (int j = 0; j < m; j++) {
          if (need[i][j] > work[j]) {
            can_satisfy = 0;
            break;
          }
        }
        if (can_satisfy) {
          for (int j = 0; j < m; j++)
            work[j] += alloc[i][j];
          finish[i] = 1;
          safe_seq[count++] = i;
          found = 1;
        }
      }
    }
  } while (found);

  return (count == n);
}

// Banker's Algorithm Request Simulation
int request_resource(int p_id, int req[MAX_R], int n, int m, int alloc[MAX_P][MAX_R], int max[MAX_P][MAX_R], int need[MAX_P][MAX_R], int avail[MAX_R]) {
  printf(1, "Process P%d requesting resources: [ ", p_id);
  for (int j = 0; j < m; j++) printf(1, "%d ", req[j]);
  printf(1, "]\n");

  // Condition 1: Request <= Need
  for (int j = 0; j < m; j++) {
    if (req[j] > need[p_id][j]) {
      printf(1, "ERROR: Process P%d exceeded its maximum declared claim (Need)!\n", p_id);
      return 0;
    }
  }

  // Condition 2: Request <= Available
  for (int j = 0; j < m; j++) {
    if (req[j] > avail[j]) {
      printf(1, "DENIED: Resources not available. Process P%d must wait.\n", p_id);
      return 0;
    }
  }

  // Hypothetical allocation
  for (int j = 0; j < m; j++) {
    avail[j] -= req[j];
    alloc[p_id][j] += req[j];
    need[p_id][j] -= req[j];
  }

  int safe_seq[MAX_P];
  if (is_safe_state(n, m, alloc, need, avail, safe_seq)) {
    printf(1, "SAFETY CHECK PASSED: State remains SAFE. Request GRANTED.\nSafe Sequence: ");
    for (int i = 0; i < n; i++) {
      printf(1, "P%d%s", safe_seq[i], (i == n - 1) ? "\n" : " -> ");
    }
    return 1;
  } else {
    printf(1, "SAFETY CHECK FAILED: State would become UNSAFE! Request REJECTED.\n");
    // Rollback
    for (int j = 0; j < m; j++) {
      avail[j] += req[j];
      alloc[p_id][j] -= req[j];
      need[p_id][j] += req[j];
    }
    printf(1, "Rolled back state to preserve system safety.\n");
    return 0;
  }
}

// =========================================================================
// Task c: Real xv6 Pipe Circular Wait Simulation
// =========================================================================
void simulate_pipe_deadlock(void) {
  printf(1, "\n======================================================\n");
  printf(1, "Task c: Actual xv6 Pipe Circular Wait Simulation       \n");
  printf(1, "======================================================\n");

  int p1[2], p2[2];
  if (pipe(p1) < 0 || pipe(p2) < 0) {
    printf(1, "Pipe creation failed!\n");
    return;
  }

  printf(1, "Created Pipe 1 (Write end held by Process A)\n");
  printf(1, "Created Pipe 2 (Write end held by Process B)\n");
  printf(1, "Scenario: Process A holds Pipe 1 and waits to read from Pipe 2.\n");
  printf(1, "          Process B holds Pipe 2 and waits to read from Pipe 1.\n\n");

  // Formulate Resource Allocation Graph into detection matrix
  int n = 2; // Process A (P0), Process B (P1)
  int m = 2; // Pipe 1 (R0), Pipe 2 (R1)
  int alloc[MAX_P][MAX_R] = {
    {1, 0}, // P0 holds R0
    {0, 1}  // P1 holds R1
  };
  int req[MAX_P][MAX_R] = {
    {0, 1}, // P0 waits for R1
    {1, 0}  // P1 waits for R0
  };
  int avail[MAX_R] = {0, 0}; // Both pipes are busy

  printf(1, "Pipe Resource Allocation State:\n");
  print_matrices(n, m, alloc, req, avail, "Pipe Request");

  int deadlocked[MAX_P];
  int count = detect_deadlock(n, m, alloc, req, avail, deadlocked);

  if (count > 0) {
    printf(1, "DEADLOCK DETECTED! Circular wait identified between:\n");
    for (int i = 0; i < count; i++) {
      printf(1, " -> Process %s (P%d)\n", (deadlocked[i] == 0) ? "A" : "B", deadlocked[i]);
    }
    printf(1, "Circular dependency: Process A -> Pipe 2 -> Process B -> Pipe 1 -> Process A\n");
  }

  // Close pipe file descriptors
  close(p1[0]); close(p1[1]);
  close(p2[0]); close(p2[1]);
}

int main(int argc, char *argv[]) {
  printf(1, "\n######################################################\n");
  printf(1, "     xv6 Deadlock Detection & Avoidance Simulator     \n");
  printf(1, "######################################################\n");

  // ---------------------------------------------------------
  // TEST CASE 1: Deadlocked State (Deadlock Detection)
  // ---------------------------------------------------------
  printf(1, "\n>>> TEST CASE 1: Deadlocked System (Deadlock Detection) <<<\n");
  int n1 = 5, m1 = 3;
  int alloc1[MAX_P][MAX_R] = {
    {0, 1, 0},
    {2, 0, 0},
    {3, 0, 3},
    {2, 1, 1},
    {0, 0, 2}
  };
  int req1[MAX_P][MAX_R] = {
    {0, 0, 0},
    {2, 0, 2},
    {0, 0, 1},
    {1, 0, 0},
    {0, 0, 2}
  };
  int avail1[MAX_R] = {0, 1, 0};

  print_matrices(n1, m1, alloc1, req1, avail1, "Request");

  int deadlocked1[MAX_P];
  int d_count = detect_deadlock(n1, m1, alloc1, req1, avail1, deadlocked1);

  if (d_count > 0) {
    printf(1, "RESULT: Deadlock detected! %d deadlocked process(es):\n", d_count);
    printf(1, "Deadlocked Process Set: { ");
    for (int i = 0; i < d_count; i++) printf(1, "P%d ", deadlocked1[i]);
    printf(1, "}\n");
  } else {
    printf(1, "RESULT: System is safe. No deadlock.\n");
  }

  // ---------------------------------------------------------
  // TEST CASE 2: Safe State (Banker's Algorithm Safety)
  // ---------------------------------------------------------
  printf(1, "\n>>> TEST CASE 2: Safe System (Banker's Safety Check) <<<\n");
  int n2 = 5, m2 = 3;
  int alloc2[MAX_P][MAX_R] = {
    {0, 1, 0},
    {2, 0, 0},
    {3, 0, 2},
    {2, 1, 1},
    {0, 0, 2}
  };
  int max2[MAX_P][MAX_R] = {
    {7, 5, 3},
    {3, 2, 2},
    {9, 0, 2},
    {2, 2, 2},
    {4, 3, 3}
  };
  int need2[MAX_P][MAX_R];
  for (int i = 0; i < n2; i++)
    for (int j = 0; j < m2; j++)
      need2[i][j] = max2[i][j] - alloc2[i][j];

  int avail2[MAX_R] = {3, 3, 2};

  print_matrices(n2, m2, alloc2, need2, avail2, "Need");

  int safe_seq2[MAX_P];
  if (is_safe_state(n2, m2, alloc2, need2, avail2, safe_seq2)) {
    printf(1, "RESULT: System is in a SAFE STATE!\nSafe Sequence: ");
    for (int i = 0; i < n2; i++) {
      printf(1, "P%d%s", safe_seq2[i], (i == n2 - 1) ? "\n" : " -> ");
    }
  } else {
    printf(1, "RESULT: System is in an UNSAFE STATE!\n");
  }

  // ---------------------------------------------------------
  // TEST CASE 3: Safe State becoming Unsafe (Banker's Avoidance)
  // ---------------------------------------------------------
  printf(1, "\n>>> TEST CASE 3: Request Leading to Unsafe State (Banker's Avoidance) <<<\n");
  int req_p4[MAX_R] = {3, 3, 0};
  printf(1, "Process P4 issues resource request [3, 3, 0]...\n");
  request_resource(4, req_p4, n2, m2, alloc2, max2, need2, avail2);

  print_matrices(n2, m2, alloc2, need2, avail2, "Need");

  // ---------------------------------------------------------
  // TEST CASE 4: Pipe Circular Wait Simulation
  // ---------------------------------------------------------
  simulate_pipe_deadlock();

  printf(1, "\n######################################################\n");
  printf(1, "             All Deadlock Tests Completed             \n");
  printf(1, "######################################################\n\n");

  exit();
}
