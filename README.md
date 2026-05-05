# MELK-OS-project
Micro Embedded Linux Kernel. An educational Linux-inspired microkernel for ARM Cortex-M microcontrollers.
> **Important:** MicroEmbeddedLinux is **not Linux** and does not aim to be a Linux distribution or a full POSIX-compliant operating system.  
> It is a learning-oriented microkernel/RTOS-style project created to understand how kernels, schedulers, drivers, interrupts, memory layout, and embedded operating systems work from scratch.

---

## Project Goal

The main goal of this project is to build a small embedded operating system from scratch in C for ARM Cortex-M microcontrollers.

This project is intended to help understand low-level operating system concepts such as:

- Startup code and reset sequence
- Vector table and exception handlers
- Linker scripts and memory layout
- Stack and RAM initialization
- Bare-metal peripheral drivers
- GPIO and UART drivers
- System tick timer
- Cooperative and preemptive scheduling
- Task management
- Context switching
- Basic system calls
- Device abstraction layer
- UART-based shell
- Simple synchronization primitives

The project is educational and focuses on clarity, simplicity, and progressive implementation.

---

## Target Platform

Initial target board:

```text
Board: Texas Instruments EK-TM4C123GXL / Tiva C LaunchPad
MCU:   TM4C123GH6PM
Core:  ARM Cortex-M4F
Flash: 256 KB
SRAM:  32 KB
