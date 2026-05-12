# 🧠 MELK OS

**MELK OS** stands for **Micro Embedded Learning Kernel**.

MELK OS is an educational embedded operating system kernel built from scratch for ARM Cortex-M microcontrollers.

The goal of this project is to understand and implement the fundamental concepts behind embedded kernels, low-level firmware development, memory layout, interrupt handling, device drivers, scheduling, context switching, basic kernel services, device abstraction, and command-line interaction through a UART shell.

> ⚠️ **Important:**  
> MELK OS is **not based on Linux**.  
> It is not a Linux distribution, Linux variant, or POSIX-compatible operating system.

---

## 🎯 Project Goals

The main objective of MELK OS is to build a small educational kernel for microcontrollers, starting from the lowest-level boot process and gradually adding operating system features.

The project is designed to help understand:

- 🧩 ARM Cortex-M startup process
- 🧭 Vector table and reset handler
- 🧠 Linker command files and memory layout
- ⚙️ Bare-metal C programming
- 📚 Stack initialization
- 📦 `.data` and `.bss` initialization
- 🔌 GPIO driver development
- 💬 UART console development
- 🖨️ Kernel print/debug output
- ⏱️ SysTick timer configuration
- 🔁 Cooperative scheduling
- ⚡ Preemptive scheduling
- 🔄 Context switching
- 🧵 Task Control Blocks
- 🚦 Task states and ready queues
- 💤 Basic sleep and delay mechanisms
- 🔐 Mutex and semaphore concepts
- 📬 Message queues
- 🧱 Device model abstraction
- 💻 UART shell and command parser
- 🛠️ Embedded debugging techniques

---

## 🎛️ Target Platform

The initial target platform is:

```text
Board : Texas Instruments EK-TM4C123GXL LaunchPad
MCU   : TM4C123GH6PM
Core  : ARM Cortex-M4F
Flash : 256 KB
SRAM  : 32 KB
```

Current Phase 1 development uses the default system clock configuration provided after reset. Explicit system clock and PLL configuration will be implemented in the next phase.

Future support may include other ARM Cortex-M microcontrollers.

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

## ❓ Why MELK OS?

MELK OS is intended as a learning project for developers who want to deeply understand how embedded operating systems work internally.

Instead of using an existing RTOS directly, this project implements core kernel components manually.

The idea is to learn:

- 🚀 How a microcontroller starts executing code after reset
- 🧠 How memory sections are organized
- 📦 How `.data`, `.bss`, stack, and heap are configured
- ⚡ How interrupts are handled
- 🔁 How a scheduler switches between tasks
- 🧵 How each task owns its own stack
- ⏱️ How SysTick can be used as the system tick source
- 🔄 How PendSV is used for context switching
- 🧱 How a basic device model can be designed
- 💻 How a simple UART shell can interact with the kernel
- 🧰 How a small embedded kernel can be structured

---

## 🚫 What MELK OS Is Not

MELK OS is not:

- ❌ A Linux distribution
- ❌ A Linux-based kernel
- ❌ A POSIX operating system
- ❌ A production-ready RTOS
- ❌ A replacement for FreeRTOS, Zephyr, ThreadX, or other mature embedded operating systems
- ❌ A safety-certified operating system
- ❌ A commercial embedded platform

This project is mainly educational and experimental.

---

## 📁 Current Repository Structure

Current Phase 1 structure:

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
│   ├── uart.c
│   └── uart.h
│
├── kernel/
│   ├── kernel.c
│   ├── kernel.h
│   ├── printk.c
│   └── printk.h
│
└── README.md
```

> Note: Emojis are used only in this README for visual clarity.  
> Folder names should remain simple and portable, for example `boot/`, `kernel/`, `drivers/`, `system/`, `linker/`, etc.

---

## 🧩 Folder Purpose

```text
boot/
```

Contains the lowest-level boot code executed after reset:

- Vector table
- `Reset_Handler`
- Default exception handlers
- `.data` initialization
- `.bss` clearing
- Jump to `kernel_main()`

```text
linker/
```

Contains the linker command file for the target MCU.

The current linker file reserves a dedicated `VECTORS` region at `0x00000000` to guarantee that the Cortex-M vector table is always placed at the correct reset address.

```text
system/
```

Contains system-level initialization code.

Current status:

- `SystemInit()` exists
- Clock configuration is still using the reset/default configuration
- Explicit PLL/system clock configuration will be added in the next phase

```text
drivers/
```

Contains low-level peripheral drivers.

Current implemented drivers:

- GPIO driver for Port F red LED
- UART0 driver for serial console output

```text
kernel/
```

Contains early kernel-level code.

Current implemented modules:

- `kernel_main()`
- `kernel_print()`

---

## 🗺️ Development Roadmap

### 🟢 Phase 1 — Bare-Metal Boot, GPIO and UART Console

Status: **Completed**

Implemented:

- Create linker command file for CCS
- Reserve vector table region at `0x00000000`
- Create vector table
- Implement `Reset_Handler`
- Initialize `.data`
- Clear `.bss`
- Jump to `kernel_main()`
- Implement basic `SystemInit()`
- Implement GPIO driver
- Enable GPIO Port F clock
- Configure PF1 as output
- Blink onboard red LED
- Implement UART0 driver
- Configure PA0 and PA1 for UART0
- Implement `uart0_write_char()`
- Implement `uart0_write_string()`
- Implement `kernel_print()`
- Print boot messages through UART0

Current boot flow:

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
gpio_init()
  ↓
uart0_init()
  ↓
kernel_print()
  ↓
Main kernel loop
```

---

### 🔵 Phase 2 — System Clock and SysTick Timer

Planned:

- Configure system clock explicitly
- Configure PLL
- Define `SYSTEM_CLOCK_HZ`
- Configure SysTick for 1 ms tick
- Implement `SysTick_Handler`
- Implement kernel tick counter
- Implement `os_get_ticks()`
- Implement `os_delay_ms()`
- Replace busy-wait delays with tick-based delays

---

### 🟣 Phase 3 — Task Management

Planned:

- Define Task Control Block
- Define task stack
- Define task states
- Implement `task_create()`
- Add idle task
- Prepare task list or task table

---

### 🟠 Phase 4 — Cooperative Scheduler

Planned:

- Implement ready queue
- Implement round-robin scheduling
- Implement `os_yield()`
- Switch between tasks cooperatively
- Add basic scheduler debug output through UART

---

### 🔴 Phase 5 — Preemptive Scheduler

Planned:

- Configure PendSV
- Implement context switch
- Save and restore task context
- Use SysTick to trigger scheduling
- Switch tasks automatically from SysTick

---

### 🟡 Phase 6 — Basic Kernel Services

Planned:

- Implement `os_sleep()`
- Implement simple mutex
- Implement semaphore
- Implement message queue
- Add basic error/status codes

---

### 🧩 Phase 7 — Device Model

Planned:

- Define device abstraction
- Implement `device_register()`
- Implement `device_open()`
- Implement `device_read()`
- Implement `device_write()`
- Add UART and GPIO devices

---

### 💻 Phase 8 — UART Shell

Planned:

- Implement command parser
- Add `help` command
- Add `ps` command
- Add `uptime` command
- Add `gpio` command
- Add `reboot` command

---

## 🧠 Phase 1 Technical Notes

### Vector Table Placement

For ARM Cortex-M, the vector table must be placed at address:

```text
0x00000000
```

At reset, the CPU reads:

```text
0x00000000 → Initial Stack Pointer
0x00000004 → Reset_Handler address
```

For this reason, the linker command file defines a dedicated memory region:

```text
VECTORS origin = 0x00000000
FLASH   origin = 0x00000400
```

This prevents `.text` or other sections from being placed before the vector table.

### Current Memory Layout

```text
VECTORS : 0x00000000 - 0x000003FF
FLASH   : 0x00000400 - 0x0003FFFF
SRAM    : 0x20000000 - 0x20007FFF
STACK   : 0x20008000
```

### UART0 Configuration

UART0 is used as the early kernel console.

```text
UART  : UART0
RX    : PA0
TX    : PA1
Baud  : 115200
Data  : 8 bits
Parity: None
Stop  : 1 bit
Flow  : None
```

The UART output is available through the LaunchPad virtual COM port.

---

## 🧪 Current Phase 1 Demo

The current firmware:

- Boots using a custom vector table
- Enters `Reset_Handler`
- Initializes memory sections
- Jumps to `kernel_main`
- Initializes GPIO
- Initializes UART0
- Prints boot messages
- Blinks the red LED on PF1

Expected UART output:

```text
================================
 MELK OS - Phase 1
 Bare-metal boot successful
 UART0 console initialized
================================
MELK OS is running...
MELK OS is running...
MELK OS is running...
```

Expected board behavior:

```text
Red LED on PF1 blinks continuously.
```

---

## 🧰 Build Requirements

Current Phase 1 requirements:

- Code Composer Studio 10.4.0
- TI ARM Compiler v20.2.6.LTS
- EK-TM4C123GXL LaunchPad
- Serial terminal application, for example:
  - Tera Term
  - PuTTY
  - RealTerm
  - CCS Terminal

Future GCC build requirements:

- ARM GCC toolchain
- Make
- `arm-none-eabi-gcc`
- `arm-none-eabi-objcopy`
- `arm-none-eabi-gdb`

---

## 🛠️ Build and Debug

### Build in Code Composer Studio

1. Open the project in Code Composer Studio.
2. Select the correct target:

```text
TM4C123GH6PM
```

3. Build the project:

```text
Project → Build Project
```

4. Start debug:

```text
Run → Debug
```

5. Run the firmware:

```text
Resume
```

### Debug Notes

Because MELK OS uses a custom entry point, the project does not start from the standard TI runtime symbol:

```text
_c_int00
```

The entry point is:

```text
Reset_Handler
```

If CCS attempts to run to `main`, disable that option or change the run-to symbol to:

```text
Reset_Handler
```

---

## 📚 Planned Documentation

Planned documentation:

- `docs/architecture.md` — Kernel architecture overview
- `docs/boot.md` — Reset handler and startup sequence
- `docs/memory_map.md` — Flash, SRAM, stack, heap, and linker command file explanation
- `docs/gpio_driver.md` — GPIO driver implementation
- `docs/uart_console.md` — UART console implementation
- `docs/system_clock.md` — Clock and PLL configuration
- `docs/systick_timer.md` — SysTick timer and kernel tick
- `docs/task_management.md` — Task Control Block and task states
- `docs/scheduler.md` — Cooperative and preemptive scheduler design
- `docs/context_switch.md` — Context switching on ARM Cortex-M
- `docs/device_model.md` — Device abstraction layer
- `docs/uart_shell.md` — UART shell and command parser

---

## 📍 Current Status

Current development stage:

```text
Phase 1 — Completed
```

Implemented:

- ✅ Project structure
- ✅ Linker command file
- ✅ Dedicated vector table region
- ✅ Vector table
- ✅ `Reset_Handler`
- ✅ `.data` initialization
- ✅ `.bss` clearing
- ✅ `SystemInit()`
- ✅ `kernel_main()`
- ✅ GPIO driver
- ✅ Red LED blink
- ✅ UART0 driver
- ✅ `kernel_print()`
- ✅ UART boot messages

Next milestone:

```text
Phase 2 — System Clock and SysTick Timer
```

---

## 🧱 Kernel Naming Convention

The project uses a simple naming convention for clarity:

```text
kernel_*    Kernel-level initialization and control
task_*      Task management functions
os_*        Public OS-like services
uart_*      UART driver functions
gpio_*      GPIO driver functions
device_*    Device model functions
shell_*     UART shell functions
```

Examples:

```c
kernel_main();
kernel_print();

task_create();

os_delay_ms();
os_get_ticks();
os_sleep();

uart0_init();
uart0_write_char();
uart0_write_string();

gpio_init();
gpio_red_led_on();
gpio_red_led_off();
gpio_toggle_red_led();

device_register();
device_open();
device_read();
device_write();
```

---

## ⚠️ Disclaimer

MELK OS is an educational project.

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

The goal is not to build the next Linux or the next commercial RTOS.

The goal is to learn how kernels are built from the ground up, starting from the reset vector and gradually evolving into a small educational embedded operating system.