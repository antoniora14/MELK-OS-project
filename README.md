# 🧠 MELK OS

**MELK OS** stands for **Micro Embedded Learning Kernel**.

MELK OS is an educational embedded operating system kernel built from scratch for ARM Cortex-M microcontrollers. It is currently developed for the **EK-TM4C123GXL LaunchPad** using the **TM4C123GH6PM** ARM Cortex-M4 microcontroller.

The project starts at the reset vector and evolves incrementally toward a small kernel with drivers, task management, preemptive scheduling, context switching, kernel services, device abstraction, and a UART shell.

> ⚠️ **Important:**  
> MELK OS is **not Linux**, is **not POSIX-compatible**, and is **not a production RTOS**.  
> It is an educational project designed to learn kernel concepts while maintaining a clean and extensible embedded software architecture.

---

## 🎯 Project Goals

MELK OS is intended to provide practical understanding of:

- ARM Cortex-M startup and exception handling
- Vector tables and reset handlers
- Linker command files and memory layout
- `.data` initialization and `.bss` clearing
- Bare-metal peripheral drivers
- UART-based kernel diagnostics
- SysTick-based system time
- Task Control Blocks and per-task stacks
- Cooperative and preemptive round-robin scheduling
- Real context switching using PSP, MSP, SVC and PendSV
- Non-blocking task sleep and wakeup mechanisms
- Future synchronization and IPC primitives
- Future device abstraction and UART shell support

---

## 🎛️ Target Platform

The TM4C123GH6PM is currently configured to run at:

```text
Board        : EK-TM4C123GXL LaunchPad
MCU          : TM4C123GH6PM
CPU          : ARM Cortex-M4
System Clock : 80 MHz
Clock Source : 16 MHz external crystal + PLL
PLL Path     : 400 MHz PLL VCO divided down to 80 MHz
Kernel Tick  : 1 ms using SysTick
```

---

## 🧰 Current Toolchain

Current development environment:

```text
IDE      : Code Composer Studio 10.4.0
Compiler : TI ARM Compiler v20.2.6.LTS
Target   : TM4C123GH6PM
Debugger : Stellaris In-Circuit Debug Interface
Board    : EK-TM4C123GXL LaunchPad
```

A future build configuration will also be added using:

```text
arm-none-eabi-gcc
Makefile
```

---

## 🚫 Project Scope and Constraints

Current design constraints:

- Bare-metal C implementation
- Minimal assembly, used only where architecture support requires it
- No `malloc()` / `free()`
- No external runtime dependencies
- Static task table and static per-task stacks
- Round-robin scheduler without priorities
- PendSV remains the only real task context switch mechanism
- SysTick only updates time, manages timed wakeups and requests scheduling
- UART console is currently not protected against concurrent task output

MELK OS is not designed for:

- Safety-critical or production automotive systems
- Medical or aerospace products
- POSIX applications
- Replacement of FreeRTOS, Zephyr, ThreadX or other mature RTOS solutions

---

## 📁 Current Repository Structure

```text
MELK_OS/
│
├── boot/
│   └── boot_tm4c123gh6pm.c
│
├── linker/
│   └── tm4c123gh6pm.cmd
│
├── system/
│   ├── system.c
│   └── system.h
│
├── drivers/
│   ├── gpio.c
│   ├── gpio.h
│   ├── systick.c
│   └── systick.h
│   ├── uart.c
│   └── uart.h
│
├── kernel/
│   ├── kernel.c
│   ├── kernel.h
│   ├── printk.c
│   ├── printk.h
│   ├── task.c
│   └── task.h
│   ├── task_scheduler.c
│   └── task_scheduler.h
│   ├── context_switch.c
│   └── context_switch.h
│   ├── context_switch_asm.asm
│
└── README.md
```

> Note: Emojis are used only in this README for visual clarity.  
> Folder names should remain simple and portable, for example `boot/`, `kernel/`, `drivers/`, `system/`, `linker/`, etc.

---

## 🧩 Module Responsibilities

### `boot/`

Contains the lowest-level startup code:

- Vector table placed at address `0x00000000`
- `Reset_Handler`
- Manual `.data` initialization
- Manual `.bss` clearing
- Exception handler bindings
- Transfer of control to `kernel_main()`

### `linker/`

Contains the CCS linker command file and guarantees placement of the vector table at the MCU reset address.

### `system/`

Contains system-level clock initialization:

- `SystemInit()`
- PLL configuration for an 80 MHz system clock
- Compile-time clock validation
- `system_get_clock_hz()`

### `drivers/`

Contains low-level peripheral drivers:

- GPIO Port F RGB LED control
- UART0 serial console driver
- UART baudrate derived from the active system clock

### `kernel/`

Contains kernel functionality:

- Kernel entry and diagnostic printing
- SysTick-based OS time
- Task Control Blocks and static stacks
- Cooperative and preemptive scheduling
- SVC/PendSV context switching
- Non-blocking task sleep support

---

## 🏗️ Current Kernel Architecture

### Boot Flow

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
SystemInit() configures PLL clock
  ↓
kernel_main()
  ↓
Initialize GPIO, UART0 and SysTick
  ↓
Initialize task system and create tasks
  ↓
Initialize scheduler and context switching
  ↓
SVC starts the first task using PSP
  ↓
Preemptive task execution
```

### Scheduling and Context Switching Flow

```text
Application task running on PSP
  ↓
SysTick interrupt every 1 ms
  ├── Increment OS tick counter
  ├── Wake tasks whose sleep time has expired
  └── Request scheduling when required
          ↓
       PendSV pending
          ↓
PendSV_Handler
  ├── Save R4-R11 and PSP of outgoing task
  ├── Select next READY task using round-robin scheduling
  ├── Restore PSP and R4-R11 of incoming task
  └── Exception return resumes the selected task
```

### Exception Roles

| Exception | Responsibility in MELK OS |
|---|---|
| `SysTick_Handler` | Maintains the 1 ms kernel tick, processes timed wakeups and requests preemption |
| `SVC_Handler` | Starts the first real task context using PSP |
| `PendSV_Handler` | Performs the real task context switch |

---

## 🧵 Task Model

MELK OS currently uses a fixed-size, statically allocated task model:

- Static task table
- Static private stack per task
- Task 0 reserved for the idle task
- Application tasks created through `task_create()`
- No dynamic allocation
- No priorities yet

### Task States

```c
TASK_STATE_UNUSED
TASK_STATE_READY
TASK_STATE_RUNNING
TASK_STATE_BLOCKED
TASK_STATE_SLEEPING
TASK_STATE_SUSPENDED
```

Current state usage:

| State | Current purpose |
|---|---|
| `TASK_STATE_READY` | Eligible to be scheduled |
| `TASK_STATE_RUNNING` | Currently executing task |
| `TASK_STATE_SLEEPING` | Waiting for a time-based wakeup created by `os_sleep()` |
| `TASK_STATE_BLOCKED` | Reserved for future synchronization/event services |

The Task Control Block stores the saved process stack pointer required for context switching and now stores timing information for sleeping tasks.

---

## 💤 Non-Blocking Task Sleep

The first implemented Phase 6 kernel service is:

```c
void os_sleep(uint32_t milliseconds);
```

### Behavior

When a running task calls `os_sleep()`:

1. The task receives an absolute wakeup tick.
2. Its state changes to `TASK_STATE_SLEEPING`.
3. It immediately yields execution by requesting PendSV.
4. The scheduler ignores the sleeping task while selecting READY tasks.
5. SysTick checks sleeping tasks on every kernel tick.
6. When the wakeup tick expires, the task returns to `TASK_STATE_READY`.
7. The task resumes when selected again by the round-robin scheduler.

This allows a sleeping task to stop consuming CPU time while other tasks or the idle task execute.

### `os_sleep()` vs. `os_delay_ms()`

| API | Current purpose |
|---|---|
| `os_sleep()` | Preferred delay service inside scheduled tasks; blocks the task without busy-waiting |
| `os_delay_ms()` | Retained for early initialization, simple compatibility cases or pre-scheduler code; it waits actively on the system tick |

---

## 🧪 Current Hardware Validation Demo

The current demo creates three application tasks plus the idle task:

| Task | Action | Sleep interval |
|---|---|---:|
| `app_task_1` | Toggle green LED / optional diagnostic event | 500 ms |
| `app_task_2` | Toggle red LED / optional diagnostic event | 1000 ms |
| `app_task_3` | Toggle blue LED / optional diagnostic event | 2000 ms |

Validated on the EK-TM4C123GXL board:

- Three real tasks execute under preemptive scheduling.
- Tasks use PSP while exception handlers use MSP.
- SysTick requests time-slice preemption.
- PendSV switches task contexts successfully.
- `os_sleep()` removes normal task delay busy-waiting.
- Sleeping tasks wake automatically at their requested wakeup times.
- Debug timing statistics confirm that no tested task woke before its requested delay.
- The idle task remains the fallback when all application tasks are sleeping.

### Sleep Timing Validation Criterion

With the current 1 ms kernel tick:

```text
elapsed_ticks >= requested_sleep_milliseconds
```

Validated intervals:

```text
app_task_1 : requested  500 ms
app_task_2 : requested 1000 ms
app_task_3 : requested 2000 ms
```

---

## 🗺️ Development Roadmap

### 🟢 Phase 1 — Bare-Metal Boot, GPIO and UART Console

Status: **Completed and validated on hardware**

Implemented:

- CCS linker command file
- Vector table at `0x00000000`
- `Reset_Handler`
- Manual `.data` initialization and `.bss` clearing
- `SystemInit()` entry
- GPIO Port F LED driver
- UART0 serial driver
- `kernel_print()` and boot messages through UART

---

### 🔵 Phase 2 — System Clock and SysTick Timer

Status: **Completed and validated on hardware**

Implemented:

- PLL-based 80 MHz system clock
- Configurable `SYSTEM_CLOCK_HZ`
- `system_get_clock_hz()`
- UART0 baudrate calculated from active clock
- SysTick configured as a 1 ms kernel tick
- `SysTick_Handler`
- OS tick counter
- `os_get_ticks()`
- `os_delay_ms()`
- SysTick validation/control helpers

---

### 🟣 Phase 3 — Task Management

Status: **Completed and validated on hardware**

Implemented:

- `task.h` and `task.c`
- Task Control Block
- Task entry functions and task states
- Fixed-size static task table
- Static stacks per task
- Initial task stack frame preparation
- `task_system_init()`
- `task_create()`
- Idle task reserved as task 0
- Stack pointer read/write support for context switching

---

### 🟠 Phase 4 — Cooperative Scheduler

Status: **Completed and validated on hardware**

Implemented:

- `task_scheduler.h` and `task_scheduler.c`
- Logical round-robin scheduler
- Selection of READY tasks
- RUNNING/READY state transitions
- Idle task fallback
- `os_yield()` for voluntary scheduling
- Initial UART-based scheduling observation

---

### 🔴 Phase 5 — Preemptive Scheduler

Status: **Completed and validated on hardware**

Implemented:

- Initial Cortex-M task exception frame
- PSP execution for tasks and MSP execution for handlers
- `context_switch.h`, `context_switch.c` and TI-compatible assembly implementation
- `SVC_Handler` to start the first task
- `PendSV_Handler` for real context switching
- R4-R11 save/restore
- Per-task saved PSP storage
- Automatic scheduling request from SysTick
- Configurable time slice
- Runtime enable/disable of scheduler preemption
- Three application tasks executing preemptively on real hardware

---

### 🟡 Phase 6 — Basic Kernel Services

Status: **In Progress — first service implemented and validated**

Implemented and validated:

- Time-based `TASK_STATE_SLEEPING` usage
- Wakeup tick information in the Task Control Block
- `os_sleep(uint32_t milliseconds)`
- Non-blocking sleep for scheduled tasks
- Automatic task wakeup processing driven by SysTick
- Scheduler exclusion of sleeping tasks
- Immediate CPU release when a task sleeps
- Idle task execution when no application task is READY
- Sleep timing verification using 500 ms, 1000 ms and 2000 ms tasks

Pending in this phase:

- Simple mutex service
- Semaphore service
- Message queue service
- Protection strategy for concurrent UART/kernel diagnostic output
- Basic consolidated kernel service status/error codes as needed

---

### 🧩 Phase 7 — Device Model

Status: **Planned**

Planned:

- Device abstraction interface
- `device_register()`
- `device_open()`
- `device_read()`
- `device_write()`
- UART and GPIO device integration

---

### 💻 Phase 8 — UART Shell

Status: **Planned**

Planned:

- Command parser
- `help` command
- `ps` command
- `uptime` command
- `gpio` command
- `reboot` command

---

## 📍 Current Status

Current development stage:

```text
Phase 6 — Basic Kernel Services
os_sleep() implemented and validated; synchronization and IPC services pending
```

Current implemented feature set:

- ✅ Custom bare-metal boot path
- ✅ Linker-controlled vector table at reset address
- ✅ Manual `.data` and `.bss` initialization
- ✅ PLL system clock configuration at 80 MHz
- ✅ GPIO RGB LED driver
- ✅ UART0 serial console
- ✅ Kernel print functions
- ✅ 1 ms SysTick system time
- ✅ Static task management and task stacks
- ✅ Idle task
- ✅ Cooperative round-robin scheduling foundation
- ✅ SVC-based first task startup
- ✅ PendSV-based real context switching
- ✅ PSP/MSP separation for task and handler execution
- ✅ Preemptive round-robin scheduling driven by SysTick
- ✅ Optional manual `os_yield()`
- ✅ Non-blocking `os_sleep()`
- ✅ Automatic timed wakeup of sleeping tasks
- ✅ Hardware/debugger validation of three sleeping application tasks

Next development milestone:

```text
Phase 6 continuation — implement a simple blocking mutex service
```

---

## 🧱 Kernel Naming Convention

The project uses a simple naming convention for clarity:

```text
kernel_*    Kernel-level initialization and diagnostics
task_*      Task management functions
os_*        Public OS-like services and scheduler control
uart_*      UART driver functions
gpio_*      GPIO driver functions
device_*    Device model functions
shell_*     UART shell functions
```

Examples:

```c
kernel_main();
kernel_print();
kernel_print_uint32();

task_system_init();
task_create();
task_set_state();

os_get_ticks();
os_delay_ms();
os_yield();
os_sleep();
os_scheduler_init();
os_scheduler_start();

uart0_init();
uart0_write_char();
uart0_write_string();

gpio_init();
gpio_toggle_red_led();
gpio_toggle_green_led();
gpio_toggle_blue_led();
```

---

## 🛠️ Build and Debug

### Build in Code Composer Studio

1. Open the project in Code Composer Studio.
2. Select the target device:

```text
TM4C123GH6PM
```

3. Build the project:

```text
Project → Build Project
```

4. Start a debug session:

```text
Run → Debug
```

5. Resume firmware execution.

### Custom Entry Point

MELK OS uses its own startup flow and does not start through the standard TI runtime symbol:

```text
_c_int00
```

The kernel entry path begins at:

```text
Reset_Handler
```

If CCS attempts to run to `main`, disable that behavior or configure the run-to symbol appropriately.

### Important Debugger Configuration: Disable Semihosting

MELK OS uses `SVC_Handler` as a real kernel exception handler to start the first task context. CCS semihosting can attempt to intercept SVC and produce debugger console messages such as:

```text
CORTEX_M4_0: Semihosting: Unknown register: SPSR_SVC
```

For normal MELK OS debugging:

```text
Enable Semihosting      : Disabled
Enable CIO function use : Disabled
Diagnostic output       : UART0 console
```

Do not place permanent breakpoints in `SVC_Handler` or `PendSV_Handler` while measuring timing behavior, because stopping the target disturbs scheduler timing.

---

## 📚 Planned Documentation

Planned documentation:

- `docs/architecture.md` — Kernel architecture overview
- `docs/boot.md` — Reset handler and startup sequence
- `docs/memory_map.md` — Flash, SRAM, stacks and linker layout
- `docs/gpio_driver.md` — GPIO driver implementation
- `docs/uart_console.md` — UART console implementation
- `docs/system_clock.md` — Clock and PLL configuration
- `docs/systick_timer.md` — SysTick timer and OS tick
- `docs/task_management.md` — Task Control Block and task states
- `docs/scheduler.md` — Cooperative and preemptive scheduling
- `docs/context_switch.md` — Cortex-M context switching using SVC, PendSV, MSP and PSP
- `docs/kernel_services.md` — Sleep, synchronization and IPC services
- `docs/device_model.md` — Device abstraction layer
- `docs/uart_shell.md` — UART shell and command parser

---

## ⚠️ Disclaimer

MELK OS is an educational and experimental project.

It is not designed for:

- Safety-critical systems
- Automotive production systems
- Medical systems
- Aerospace systems
- Commercial embedded products

The code is written for learning purposes and should be reviewed, tested, and improved before being used in any real embedded application.

---

## 📄 License

This project may be released under the MIT License.

```text
MIT License

Copyright (c) 2026 Juan Antonio Román Alarcón

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files, to deal in the Software
without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
subject to the conditions defined in the full license file.
```

---

## 👨‍💻 Author

**Juan Antonio Román Alarcón**

Embedded Software Developer | Test Automation Engineer | C/C++/C# Developer

This project is part of my personal learning path to deeply understand embedded operating systems, ARM Cortex-M internals, low-level kernel development, device drivers, task scheduling, and embedded software architecture.

---

## ⭐ Final Note

MELK OS is a journey into how operating systems work at the embedded level.

The objective is not to build the next Linux or replace a commercial RTOS. The objective is to learn how a kernel is built from the reset vector upward, gradually evolving into a clear, maintainable and educational embedded operating system kernel.
