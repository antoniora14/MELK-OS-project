# MELK OS Kernel Services

Phase 6 introduces basic kernel services for task waiting, synchronization, and communication.

Implemented services:

- `os_sleep()`
- Mutex
- Counting semaphore
- Static message queue

---

## Service Overview

| Service | Purpose | Task State Used |
|---|---|---|
| `os_sleep()` | Wait for time to expire | `TASK_STATE_SLEEPING` |
| Mutex | Protect exclusive resources | `TASK_STATE_BLOCKED` |
| Semaphore | Count resources or events | `TASK_STATE_BLOCKED` |
| Message Queue | Transfer data between tasks | `TASK_STATE_BLOCKED` |

---

## `os_sleep()`

`os_sleep()` suspends the current task without busy-waiting.

Example:

```c
os_sleep(500U);
```

The task transitions:

```text
RUNNING → SLEEPING
```

SysTick wakes the task when the wakeup tick expires:

```text
SLEEPING → READY
```

Use `os_sleep()` when a task needs to wait for time.

---

## Mutex

A mutex protects a shared resource so that only one task can use it at a time.

Example:

```c
os_mutex_lock(&g_console_mutex);

kernel_print("Hello from task\n");

os_mutex_unlock(&g_console_mutex);
```

Use a mutex for:

- UART output
- I2C transactions
- SPI transactions
- Shared configuration structures
- Shared buffers

A mutex has an owner. The task that locks the mutex should be the task that unlocks it.

---

## Semaphore

A semaphore counts available resources or pending events.

Example:

```c
os_semaphore_init(&g_resource_semaphore, 2U, 2U);
```

This means two resource units are available.

A task consumes one unit with:

```c
os_semaphore_wait(&g_resource_semaphore);
```

A task releases or signals one unit with:

```c
os_semaphore_post(&g_resource_semaphore);
```

Use a semaphore for:

- Buffer pools
- Resource slots
- Available channels
- Pending events
- Producer/consumer synchronization

A semaphore does not have an owner.

---

## Message Queue

A message queue transfers fixed-size messages between tasks.

Example:

```c
os_message_queue_send(&g_queue, &message);
os_message_queue_receive(&g_queue, &message);
```

The queue blocks the receiver when empty and blocks the sender when full.

Internally, the current static message queue uses:

```text
1 mutex
2 semaphores
1 circular buffer
```

The mutex protects internal queue data:

```text
head
tail
storage[]
```

The semaphores count:

```text
available_items  → messages ready to read
available_spaces → free spaces available for writing
```

---

## Blocking Rules

Blocking services must only be called from task context.

Do not call blocking services from:

- SysTick handler
- PendSV handler
- SVC handler
- Peripheral ISRs

Future MELK OS versions may add explicit ISR-safe APIs such as:

```text
os_semaphore_post_from_isr()
os_message_queue_send_from_isr()
```

---

## Conceptual Summary

```text
os_sleep()
    Wait for time.

Mutex
    Protect exclusive access.

Semaphore
    Count resources or events.

Message Queue
    Transfer data between tasks.
```
