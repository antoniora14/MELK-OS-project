# Counting Semaphore Service

A counting semaphore is a kernel synchronization primitive used to count available resources or pending events.

In MELK OS, the semaphore is blocking and intended for task context only.

---

## Purpose

Use a semaphore when a task must wait for:

- A limited number of resource units
- A pending event
- A free buffer
- A message or item becoming available

---

## Basic API

```c
int32_t os_semaphore_init(os_semaphore_t *semaphore,
                          uint32_t initial_count,
                          uint32_t maximum_count);

int32_t os_semaphore_wait(os_semaphore_t *semaphore);
int32_t os_semaphore_post(os_semaphore_t *semaphore);
```

---

## Resource Counting Example

```c
os_semaphore_init(&g_resource_semaphore, 2U, 2U);
```

This means:

```text
Two resource units are available.
Maximum resource count is two.
```

A task consumes a unit:

```c
os_semaphore_wait(&g_resource_semaphore);
```

A task releases a unit:

```c
os_semaphore_post(&g_resource_semaphore);
```

---

## Task State Flow

If count is greater than zero:

```text
Task calls os_semaphore_wait()
    ↓
count--
    ↓
Task continues running
```

If count is zero:

```text
Task calls os_semaphore_wait()
    ↓
No resource available
    ↓
TASK_STATE_RUNNING → TASK_STATE_BLOCKED
    ↓
PendSV switches to another task
```

When another task posts:

```text
Task calls os_semaphore_post()
    ↓
Waiting task receives the resource
    ↓
TASK_STATE_BLOCKED → TASK_STATE_READY
```

---

## Difference From Mutex

| Feature | Mutex | Semaphore |
|---|---|---|
| Has owner | Yes | No |
| Counts multiple resources | No | Yes |
| Protects exclusive access | Yes | Not necessarily |
| Can signal events | Not usually | Yes |

A mutex is best for protecting a shared resource.

A semaphore is best for counting resources or events.

---

## Direct Handoff

If a task posts a resource while another task is waiting, MELK OS reserves that resource directly for the waiting task.

This avoids another runnable task taking the resource before the blocked task resumes.

---

## Example Use Cases

- Free buffer pool
- ADC channel slots
- Available instruments
- Pending received messages
- Event notification between tasks

---

## Summary

Use semaphore when the problem is:

```text
How many resources or events are available?
```
