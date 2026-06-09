# Context Switching in MELK OS

MELK OS implements real task context switching on ARM Cortex-M using PSP, MSP, SVC, and PendSV.

---

## ARM Cortex-M Stack Model

MELK OS uses two stack pointers:

| Stack Pointer | Used For |
|---|---|
| MSP | Exceptions and handlers |
| PSP | Thread-mode tasks |

This separation is important because each task owns its own PSP-based stack, while exception handlers continue using the MSP.

---

## Initial Task Stack Frame

Each task stack is prepared to look as if the task was interrupted by an exception.

The initial hardware exception frame contains:

```text
xPSR
PC
LR
R12
R3
R2
R1
R0
```

Important values:

```text
xPSR → Thumb bit set
PC   → task entry function
LR   → task_exit_trap
R0   → task argument
```

The software-saved registers are:

```text
R4-R11
```

These are saved and restored manually by the PendSV handler.

---

## Starting the First Task

The first task is started using SVC.

Flow:

```text
os_scheduler_start()
    ↓
os_start_first_task()
    ↓
SVC instruction
    ↓
SVC_Handler
    ↓
Load first task PSP
    ↓
Switch Thread mode to PSP
    ↓
Exception return into first task
```

SVC is used only to bootstrap the first task context.

After the first task starts, normal task switching is performed by PendSV.

---

## PendSV Context Switch

PendSV is responsible for the real context switch.

General flow:

```text
PendSV_Handler
    ↓
Read current PSP
    ↓
Save R4-R11 on current task stack
    ↓
Store updated PSP in current task TCB
    ↓
Call scheduler to select next task
    ↓
Load next task PSP from TCB
    ↓
Restore R4-R11 from next task stack
    ↓
Write PSP
    ↓
Exception return into next task
```

---

## Why PendSV?

PendSV is designed for deferred context switching on Cortex-M.

MELK OS uses this model:

```text
SysTick → requests scheduling
PendSV  → performs context switch
```

This avoids doing heavy context switching directly inside SysTick.

---

## SysTick and PendSV Relationship

SysTick handles time-related work:

- Increment OS tick
- Wake sleeping tasks
- Update scheduler time slice
- Trigger PendSV when needed

PendSV handles CPU context switching:

- Save current task state
- Restore next task state

---

## Thread Mode vs Handler Mode

Tasks run in Thread mode.

Exceptions run in Handler mode.

MELK OS includes a helper to determine whether code is running in exception context. Blocking services such as mutex, semaphore, and message queue should only be called from Thread mode because they can block the current task.

---

## Debugging Note: Semihosting

Because MELK OS uses SVC for starting the first task, semihosting should be disabled in Code Composer Studio.

Recommended debug configuration:

```text
Enable Semihosting      = Disabled
Enable CIO function use = Disabled
```

The project uses UART0 as its debug console instead of semihosting.

---

## Summary

```text
SVC     → starts the first task
PendSV  → performs every context switch
SysTick → updates time and requests scheduling
PSP     → used by tasks
MSP     → used by handlers
```
