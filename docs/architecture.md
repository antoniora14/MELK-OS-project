# MELK OS Architecture

MELK OS stands for **Micro Embedded Learning Kernel**.

It is an educational bare-metal embedded kernel for ARM Cortex-M microcontrollers. The current target platform is the Texas Instruments TM4C123GH6PM Cortex-M4 microcontroller on the EK-TM4C123GXL LaunchPad.

MELK OS is not Linux, not POSIX-compatible, and not a production RTOS. It is designed to teach how embedded kernels are built from the reset vector up to task scheduling, context switching, synchronization primitives, and inter-task communication.

---

## Current Architecture Overview

The current MELK OS architecture is organized into the following layers:

```text
Application Tasks
        |
        v
Kernel Services
(os_sleep, mutex, semaphore, message queue)
        |
        v
Task Management + Scheduler
(TCB, task states, round-robin scheduling)
        |
        v
Context Switching
(SVC, PendSV, PSP/MSP)
        |
        v
Kernel Tick
(SysTick 1 ms)
        |
        v
Drivers
(GPIO, UART)
        |
        v
System / Boot
(Reset_Handler, vector table, .data/.bss init, PLL clock)
```

---

## Boot Flow

```text
Reset
  ↓
Vector Table at 0x00000000
  ↓
Reset_Handler()
  ↓
Initialize .data
  ↓
Clear .bss
  ↓
SystemInit()
  ↓
kernel_main()
  ↓
Initialize drivers
  ↓
Initialize kernel services
  ↓
Create tasks
  ↓
Start scheduler
  ↓
Run first task through SVC
  ↓
Switch tasks through PendSV
```

---

## Main Kernel Components

### Boot Layer

Responsible for the lowest-level startup sequence:

- Vector table
- Reset handler
- `.data` initialization
- `.bss` clearing
- Jump to `kernel_main()`

### System Layer

Responsible for MCU-level configuration:

- PLL clock setup
- 80 MHz system clock
- System clock API

### Driver Layer

Current implemented drivers:

- GPIO driver
- UART0 driver

### Kernel Core

Responsible for kernel-level control:

- `kernel_main()`
- `kernel_print()`
- SysTick setup
- Task creation
- Scheduler startup

### Task Management

Responsible for:

- Task Control Blocks
- Task states
- Static task table
- Static task stacks
- Idle task

### Scheduler

Responsible for:

- Round-robin task selection
- Ready task selection
- Preemption support
- Idle task fallback

### Context Switch Layer

Responsible for:

- Using PSP for tasks
- Using MSP for handlers
- Starting the first task with SVC
- Switching tasks with PendSV
- Saving/restoring R4-R11

### Kernel Services

Current implemented services:

- `os_sleep()`
- Blocking mutex
- Blocking counting semaphore
- Static blocking message queue

---

## Task State Model

MELK OS currently uses the following task states:

```text
TASK_STATE_UNUSED
TASK_STATE_READY
TASK_STATE_RUNNING
TASK_STATE_SLEEPING
TASK_STATE_BLOCKED
TASK_STATE_TERMINATED
```

The most important runtime states are:

| State | Meaning |
|---|---|
| `READY` | The task can be selected by the scheduler. |
| `RUNNING` | The task is currently executing. |
| `SLEEPING` | The task is waiting for time to expire. |
| `BLOCKED` | The task is waiting for a resource or event. |

---

## Kernel Design Rules

MELK OS currently follows these design constraints:

- No dynamic memory allocation.
- No external dependencies.
- Bare-metal C.
- Minimal assembly only where required.
- PendSV is the only real context switch mechanism.
- SysTick updates time and requests scheduling but does not switch context directly.
- SVC is used to start the first task.
- Blocking kernel services are task-context only.
- ISRs must not call blocking services.

---

## Naming Convention

```text
kernel_*    Kernel initialization/control
os_*        Public OS-like services
task_*      Task management
uart_*      UART driver
gpio_*      GPIO driver
device_*    Future device model
shell_*     Future UART shell
```

---

## Current Status

Implemented and validated:

- Bare-metal boot
- GPIO driver
- UART console
- PLL system clock at 80 MHz
- SysTick 1 ms kernel tick
- Static task management
- Idle task
- Cooperative scheduler foundation
- Preemptive scheduler using PendSV
- Non-blocking task sleep
- Blocking mutex
- Blocking counting semaphore
- Static blocking message queue

Planned future work:

- Device model
- UART shell
- ISR-safe signaling APIs
- More advanced scheduler features
- More complete debugging tools
