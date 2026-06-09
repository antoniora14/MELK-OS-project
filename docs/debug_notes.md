# MELK OS Debug Notes

This document contains practical debugging notes for MELK OS on Code Composer Studio and the EK-TM4C123GXL LaunchPad.

---

## UART Debug Console

MELK OS uses UART0 as its debug console.

Recommended terminal settings:

```text
Baud rate : 115200
Data bits : 8
Parity    : None
Stop bits : 1
Flow ctrl : None
```

---

## Disable Semihosting

Because MELK OS uses `SVC_Handler` to start the first task, semihosting should be disabled in CCS.

Recommended settings:

```text
Enable Semihosting      = Disabled
Enable CIO function use = Disabled
```

If semihosting is enabled, CCS may try to interpret the kernel's SVC as a semihosting request.

---

## Debugging Timing

Do not use breakpoints to measure real task timing.

When the core stops:

- SysTick stops advancing normally.
- Tasks stop executing.
- Scheduler timing is disturbed.

For timing validation, prefer:

- Runtime counters
- UART timestamps
- GPIO toggling
- Oscilloscope or logic analyzer

---

## Debugging `os_sleep()`

Useful variables:

```text
sleep sample count
last elapsed ticks
minimum elapsed ticks
maximum elapsed ticks
early wakeup count
```

Expected rule:

```text
elapsed_ticks >= requested_ticks
early_wakeup_count == 0
```

---

## Debugging Mutex

Useful validation counters:

```text
g_mutex_critical_depth
g_mutex_violation_count
g_mutex_protected_print_count
g_mutex_error_count
```

Expected:

```text
g_mutex_violation_count == 0
g_mutex_error_count == 0
```

A critical depth greater than 1 indicates more than one task entered the protected section at the same time.

---

## Debugging Semaphore

Useful validation counters:

```text
g_semaphore_active_users
g_semaphore_max_active_users
g_semaphore_violation_count
g_semaphore_wait_success_count
g_semaphore_post_success_count
g_semaphore_error_count
```

Expected for a resource capacity of 2:

```text
g_semaphore_max_active_users == 2
g_semaphore_violation_count == 0
g_semaphore_error_count == 0
```

---

## Debugging Message Queue

Useful validation counters:

```text
g_message_queue_send_success_count
g_message_queue_receive_success_count
g_message_queue_error_count
g_message_queue_sequence_error_count
```

Expected:

```text
g_message_queue_send_success_count > 0
g_message_queue_receive_success_count > 0
g_message_queue_error_count == 0
g_message_queue_sequence_error_count == 0
```

---

## Observing Blocked Tasks

To observe a task entering `TASK_STATE_BLOCKED`, place a temporary breakpoint inside the blocking primitive:

- `os_mutex_lock()`
- `os_semaphore_wait()`
- `os_message_queue_send()`
- `os_message_queue_receive()`

Then inspect:

```text
g_task_table[].state
waiting_task_mask
available_items
available_spaces
```

Breakpoints are acceptable for observing state transitions, but not for measuring timing.

---

## Recommended Debug Workflow

```text
1. Build in Debug mode.
2. Flash and run normally.
3. Observe UART output.
4. Let the system run for 20-30 seconds.
5. Suspend execution.
6. Inspect Expressions.
7. Resume or reset.
```

---

## Common Issues

### UART messages mixed

Likely cause:

```text
UART output not protected by mutex.
```

### Task does not wake from sleep

Check:

```text
wakeup_tick
SysTick_Handler
os_scheduler_tick()
task_wake_expired_sleeping_tasks()
```

### System hangs after blocking

Check:

```text
TASK_STATE_BLOCKED
waiting_task_mask
whether another task can post/unlock the resource
idle task fallback
```

### Semihosting error involving SVC

Disable semihosting and CIO in CCS.
