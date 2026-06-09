# MELK OS Scheduler

The MELK OS scheduler is a simple educational round-robin scheduler for ARM Cortex-M microcontrollers.

It currently supports preemptive scheduling using SysTick and PendSV.

---

## Scheduler Goals

The scheduler is designed to demonstrate:

- Static task management
- Task states
- Round-robin task selection
- Preemptive scheduling
- Idle task fallback
- Separation between scheduling decision and context switching

---

## Important Rule

SysTick does not perform the context switch directly.

Instead:

```text
SysTick_Handler()
    ↓
Update kernel tick
    ↓
Wake sleeping tasks
    ↓
Update scheduler tick count
    ↓
Trigger PendSV when scheduling is needed
```

Then:

```text
PendSV_Handler()
    ↓
Save current task context
    ↓
Ask scheduler for next task
    ↓
Restore next task context
```

This follows the common Cortex-M design pattern where PendSV is used as the lowest-priority exception for context switching.

---

## Task Selection

The scheduler selects only tasks in:

```text
TASK_STATE_READY
```

Tasks in these states are skipped:

```text
TASK_STATE_SLEEPING
TASK_STATE_BLOCKED
TASK_STATE_TERMINATED
TASK_STATE_UNUSED
```

If no application task is ready, the scheduler selects the idle task.

---

## Idle Task

The idle task is task ID 0.

Its purpose is to provide a safe fallback when no other task is ready to run.

Example situations where idle can run:

- All application tasks are sleeping.
- All application tasks are blocked waiting for resources.

In a future version, the idle task could enter a low-power mode.

---

## Preemption

Preemption is driven by SysTick.

The kernel uses a configurable time slice:

```text
OS_SCHEDULER_TIME_SLICE_TICKS
```

When the current task consumes its time slice, SysTick triggers PendSV.

The actual context switch occurs later in PendSV.

---

## `os_yield()`

`os_yield()` allows a task to voluntarily request a context switch.

It does not switch immediately in C code. Instead, it triggers PendSV:

```text
Task calls os_yield()
    ↓
PendSV is pended
    ↓
PendSV performs context switch
```

---

## `os_sleep()` Interaction

When a task calls:

```c
os_sleep(milliseconds);
```

The current task transitions:

```text
RUNNING → SLEEPING
```

The task is assigned a wakeup tick.

SysTick periodically checks sleeping tasks. When the wakeup tick expires:

```text
SLEEPING → READY
```

The scheduler can then select the task again.

---

## Blocking Services Interaction

Mutex, semaphore, and message queue can block tasks using:

```text
TASK_STATE_BLOCKED
```

Examples:

```text
os_mutex_lock()             → BLOCKED if mutex is locked
os_semaphore_wait()         → BLOCKED if count is 0
os_message_queue_receive()  → BLOCKED if queue is empty
os_message_queue_send()     → BLOCKED if queue is full
```

Blocked tasks are ignored by the scheduler until the corresponding resource or event wakes them.

---

## Summary

The scheduler does not need to know why a task is blocked or sleeping. It only needs to know whether the task is ready.

```text
READY      → selectable
RUNNING    → currently executing
SLEEPING   → skipped
BLOCKED    → skipped
```

This keeps the scheduler simple and allows kernel services to manage their own wakeup conditions.
