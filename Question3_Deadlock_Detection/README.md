# Question 3: Deadlock Detection & Avoidance in xv6

## Overview
This question implements a comprehensive resource-allocation simulation, the **Deadlock Detection Algorithm**, and the **Banker's Algorithm for Deadlock Avoidance** in xv6 user space. In addition, it simulates an actual deadlock scenario involving real xv6 processes and IPC pipes resulting in a circular wait condition.

---

## Algorithms Implemented

### 1. Deadlock Detection Algorithm (`detect_deadlock()`)
For an $n$-process, $m$-resource type system:
- **Work Vector**: Initialized to `Available[m]`.
- **Finish Vector**: 
  - For $i \in [0, n-1]$: `Finish[i] = (Allocation[i] == 0) ? true : false`.
- **Search Loop**:
  Find an index $i$ such that `Finish[i] == false` and `Request[i][j] <= Work[j]` for all $j \in [0, m-1]$.
  - If found:
    `Work[j] += Allocation[i][j]`
    `Finish[i] = true`
    Repeat search.
- **Deadlock Determination**:
  If no such process can be found and there exist processes with `Finish[i] == false`, those processes are **deadlocked**. The function returns the exact list of deadlocked processes.

### 2. Banker's Algorithm for Deadlock Avoidance (`is_safe_state()`)
- Maintains `Allocation[n][m]`, `Max[n][m]`, `Available[m]`, and computes `Need[n][m] = Max[n][m] - Allocation[n][m]`.
- When a process makes a request `req[m]`:
  1. Verify `req[j] <= Need[p][j]`.
  2. Verify `req[j] <= Available[j]`.
  3. **Hypothetical Allocation**:
     - `Available[j] -= req[j]`
     - `Allocation[p][j] += req[j]`
     - `Need[p][j] -= req[j]`
  4. Run Safety check:
     - If safe, grant the request permanently.
     - If unsafe, roll back hypothetical changes and deny/block the request to prevent future deadlock.

---

## Test Matrices Evaluated

1. **Test Matrix 1: Deadlocked State**
   - 5 processes, 3 resource types.
   - Resource dependencies form an unresolvable cycle.
   - Algorithm identifies deadlocked processes: `{P1, P2, P3, P4}`.

2. **Test Matrix 2: Safe State**
   - 5 processes, 3 resource types.
   - Valid safe execution sequence found: `P1 -> P3 -> P4 -> P0 -> P2`.
   - All processes complete without deadlock.

3. **Test Matrix 3: Banker's Avoidance (Hypothetical Request Leading to Unsafe State)**
   - Process `P4` requests `[3, 3, 0]`.
   - Hypothetical allocation reduces `Available` to `[0, 0, 2]`.
   - Algorithm discovers that no process can finish; system enters an unsafe state.
   - Banker's algorithm **denies** the request and preserves system safety.

4. **Real xv6 Pipe Circular Wait Simulation**
   - Two processes (A and B) and two pipes (Pipe 1 and Pipe 2).
   - Process A holds Pipe 1 write-end, blocks waiting to read Pipe 2.
   - Process B holds Pipe 2 write-end, blocks waiting to read Pipe 1.
   - Formulates resource-allocation matrix and invokes `detect_deadlock()`, confirming circular wait.

---

## How to Run in xv6

1. Compile and start xv6:
   ```bash
   make clean
   make qemu-nox
   ```
2. Run the deadlock program:
   ```bash
   $ deadlock
   ```
