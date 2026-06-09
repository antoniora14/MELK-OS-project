# Mutex Service

A mutex is a kernel synchronization primitive used to protect a shared resource from concurrent access.

In MELK OS, the mutex is blocking, non-recursive, and intended for task context only.

---

## Purpose

Use a mutex when only one task should access a resource at a time.

Examples:

- UART console output
- I2C bus transactions
- SPI bus transactions
- Shared configuration structures
- Shared buffers

---

## Basic API

```c
int32_t os_mutex_init(os_mutex_t *mutex);
int32_t os_mutex_lock(os_mutex_t *mutex);
int32_t os_mutex_unlock(os_mutex_t *mutex);
```

---

## Basic Usage

```c
os_mutex_lock(&g_console_mutex);

kernel_print("Protected UART output\n");

os_mutex_unlock(&g_console_mutex);
```

---

## Task State Flow

If the mutex is free:

```text
Task calls os_mutex_lock()
    ↓
Task becomes mutex owner
    ↓
Task continues running
```

If the mutex is already locked:

```text
Task calls os_mutex_lock()
    ↓
Task is added to waiting mask
    ↓
TASK_STATE_RUNNING → TASK_STATE_BLOCKED
    ↓
PendSV switches to another task
```

When the owner unlocks:

```text
Owner calls os_mutex_unlock()
    ↓
Waiting task receives ownership
    ↓
TASK_STATE_BLOCKED → TASK_STATE_READY
```

---

## Owner Rule

A mutex has an owner.

The task that locks the mutex should be the task that unlocks it.

This is valid:

```text
TASK 1 lock
TASK 1 unlock
```

This is invalid:

```text
TASK 1 lock
TASK 2 unlock
```

---

## Non-Recursive Behavior

The mutex is non-recursive.

This is invalid:

```c
os_mutex_lock(&mutex);
os_mutex_lock(&mutex);
```

The second lock attempt returns an error instead of blocking the task forever.

---

## Direct Handoff

When a mutex is unlocked and a task is already waiting, MELK OS transfers ownership directly to a waiting task.

This avoids a third task stealing the mutex before the waiting task resumes.

---

## Why Not Disable Interrupts Around the Whole Resource?

The mutex uses short critical sections internally to protect its metadata.

It does not disable interrupts while the user resource is being used.

This is important because long UART prints or I2C transactions should not stop SysTick for too long.

---

## Summary

Use mutex when the problem is:

```text
Only one task may use this resource at a time.
```
