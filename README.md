
*This project has been created as part
of the 42 curriculum by mkisala*

# Codexion Master

A concurrent programming simulation in C inspired by the classic Dining Philosophers problem.

Each coder needs two dongles to compile.  
Dongles are shared resources protected by mutexes, and the system implements scheduling policies (`fifo` or `edf`) to decide which coder gains access to them.

The goal is to avoid starvation and burnout while multiple threads compete for shared resources.

---

# Objective

The program simulates multiple coders continuously performing the following cycle:

1. Compile
2. Debug
3. Refactor
4. Attempt to compile again

Before compiling, a coder must acquire two dongles.

The simulation stops when:

- every coder has completed `number_of_compiles_required` compilations
or
- a coder burns out.

---

# Core Concepts

## Coders

Each coder is represented by a thread (`pthread`).

Each coder contains:

- an identifier
- compilation counter
- timestamp of the last compilation start
- current state
- access to the left and right dongles

---

## Dongles

Dongles represent shared resources.

Features:

- protected by `pthread_mutex_t`
- include cooldown after release
- maintain a waiting queue
- support `fifo` and `edf` arbitration policies

---

## Scheduler

### FIFO

First In, First Out.

The dongle is granted to the coder whose request arrived first.

---

### EDF (Earliest Deadline First)

The dongle is granted to the coder with the closest burnout deadline.

Deadline formula:

```text
deadline = last_compile_start + time_to_burnout
```

If two coders share the same deadline, a deterministic tie-breaker must be applied.

---

# Program Arguments

```bash
./codexion \
number_of_coders \
time_to_burnout \
time_to_compile \
time_to_debug \
time_to_refactor \
number_of_compiles_required \
dongle_cooldown \
scheduler
```

## Example

```bash
./codexion 5 800 200 200 200 5 50 edf
```

---

# Parameters

| Argument | Description |
|---|---|
| `number_of_coders` | Number of coders and dongles |
| `time_to_burnout` | Maximum time without starting a compile |
| `time_to_compile` | Time required to compile |
| `time_to_debug` | Time spent debugging |
| `time_to_refactor` | Time spent refactoring |
| `number_of_compiles_required` | Required compilations before stopping |
| `dongle_cooldown` | Cooldown after a dongle is released |
| `scheduler` | Scheduling policy: `fifo` or `edf` |

---

# Coder States

Coders may be in one of the following states:

- taking dongles
- compiling
- debugging
- refactoring
- burned out

---

# Log Format

All logs must be serialized using a mutex.

Format:

```text
timestamp coder_id message
```

## Example

```text
0 1 has taken a dongle
1 1 has taken a dongle
1 1 is compiling
201 1 is debugging
401 1 is refactoring
1204 3 burned out
```

---

# Important Rules

## No Global Variables

All simulation state must be encapsulated inside structs.

---

## One Mutex Per Dongle

Each dongle must protect:

- availability
- cooldown state
- waiting queue

---

## Mandatory Cooldown

After release:

```text
dongle unavailable until:
release_time + dongle_cooldown
```

---

## Monitor Thread

A separate thread continuously monitors burnout conditions.

Requirements:

- precise detection
- burnout log printed within 10 ms
- stop the simulation immediately

---

# Suggested Project Structure

```text
include/
    codexion.h

src/
    main.c
    init.c
    simulation.c
    coder.c
    dongle.c
    scheduler.c
    monitor.c
    logger.c
    utils.c

Makefile
README.md
```

---

# Suggested Structures

## Coder

```c
typedef struct s_coder
{
    int             id;
    pthread_t       thread;
    long            last_compile_start;
    int             compile_count;
    t_state         state;
    t_dongle        *left;
    t_dongle        *right;
    t_simulation    *sim;
} t_coder;
```

---

## Dongle

```c
typedef struct s_dongle
{
    pthread_mutex_t mutex;
    pthread_cond_t  cond;

    int             available;
    long            cooldown_until;

    t_heap          wait_queue;
} t_dongle;
```

---

# Priority Queue (Heap)

The project requires a custom heap implementation.

Used for:

- FIFO ordering
- EDF ordering

Possible operations:

```c
push()
pop()
peek()
heapify_up()
heapify_down()
```

---

# Liveness

The system must avoid starvation.

Especially under EDF scheduling:

- coders with closer deadlines should receive priority
- no coder should wait indefinitely if parameters are feasible

---

# Compilation

```bash
make
```

Required flags:

```bash
-Wall -Wextra -Werror -pthread
```

---

# Main Challenges

- deadlocks
- starvation
- precise synchronization
- timing accuracy
- scheduler fairness
- dongle cooldown handling
- race conditions
- thread-safe logging

---

# Concepts Used

- Threads (`pthread`)
- Mutexes
- Condition variables
- Scheduling algorithms
- Priority queues
- Synchronization
- Concurrent programming
- Resource arbitration

---

# Example Execution

```bash
./codexion 4 800 200 200 200 3 50 fifo
```

Output:

```text
0 1 has taken a dongle
2 1 has taken a dongle
2 1 is compiling
202 1 is debugging
402 1 is refactoring
405 2 has taken a dongle
406 2 has taken a dongle
406 2 is compiling
1505 4 burned out
```

---

# Author

Project developed to study:

- concurrency
- synchronization
- scheduling
- fairness algorithms
- multithreaded programming in C using pthreads