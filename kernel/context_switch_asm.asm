    .thumb

    .global SVC_Handler
    .global PendSV_Handler
    .global os_get_exception_number
    .global os_irq_save
    .global os_irq_restore

    .ref os_get_current_task_stack_pointer_internal
    .ref os_save_current_task_stack_pointer
    .ref os_schedule_next_stack_pointer
    .ref os_context_switch_mark_started

; =============================================================================
; SVC_Handler
;
; Starts the first task.
;
; Flow:
;   1. Get current task stack pointer.
;   2. Restore R4-R11.
;   3. Load PSP.
;   4. Switch Thread mode to PSP.
;   5. Exception return into first task.
; =============================================================================

    .sect ".text:SVC_Handler"
    .align 4
    .thumbfunc SVC_Handler

SVC_Handler:
    .asmfunc

        BL      os_get_current_task_stack_pointer_internal
        CBZ     R0, svc_fault_loop

        LDMIA   R0!, {R4-R11}
        MSR     PSP, R0

        MOVS    R0, #2
        MSR     CONTROL, R0
        ISB

        BL      os_context_switch_mark_started

        MOVW    LR, #0xFFFD
        MOVT    LR, #0xFFFF

        BX      LR

svc_fault_loop:
        B       svc_fault_loop

    .endasmfunc


; =============================================================================
; PendSV_Handler
;
; Cooperative context switch.
;
; On exception entry, Cortex-M automatically saves:
;   R0, R1, R2, R3, R12, LR, PC, xPSR
;
; PendSV manually saves:
;   R4-R11
;
; =============================================================================

    .sect ".text:PendSV_Handler"
    .align 4
    .thumbfunc PendSV_Handler

PendSV_Handler:
    .asmfunc

        ; Get current PSP.
        MRS     R0, PSP

        ; Preserve EXC_RETURN from LR.
        ; Reserve 8 bytes to keep MSP aligned.
        SUB     SP, SP, #8
        STR     LR, [SP, #4]

        ; If PSP is zero, skip saving current task.
        CBZ     R0, pendsv_select_next

        ; Save R4-R11 on current task stack.
        STMDB   R0!, {R4-R11}

        ; Save updated PSP into current TCB.
        BL      os_save_current_task_stack_pointer

pendsv_select_next:

        ; Select next task and get its saved PSP.
        BL      os_schedule_next_stack_pointer
        CBZ     R0, pendsv_fault_loop

        ; Restore EXC_RETURN.
        LDR     LR, [SP, #4]
        ADD     SP, SP, #8

        ; Restore R4-R11 from next task stack.
        LDMIA   R0!, {R4-R11}

        ; Update PSP.
        MSR     PSP, R0

        ; Exception return.
        ; CPU restores R0-R3, R12, LR, PC, xPSR automatically.
        BX      LR

pendsv_fault_loop:
        B       pendsv_fault_loop

    .endasmfunc


; =============================================================================
; os_get_exception_number
;
; Returns IPSR:
;   0     -> Thread mode
;   != 0  -> Handler mode / active exception
;
; Kernel services that may block use this helper to reject calls from ISRs.
; =============================================================================

    .sect ".text:os_get_exception_number"
    .align 4
    .thumbfunc os_get_exception_number

os_get_exception_number:
    .asmfunc
        MRS     R0, IPSR
        BX      LR
    .endasmfunc


; =============================================================================
; os_irq_save / os_irq_restore
;
; Small critical-section primitives used by kernel services.
;
; os_irq_save():
;   - Returns current PRIMASK in R0
;   - Disables maskable interrupts
;
; os_irq_restore(primask):
;   - Restores PRIMASK from R0
; =============================================================================

    .sect ".text:os_irq_save"
    .align 4
    .thumbfunc os_irq_save

os_irq_save:
    .asmfunc
        MRS     R0, PRIMASK
        CPSID   I
        BX      LR
    .endasmfunc

    .sect ".text:os_irq_restore"
    .align 4
    .thumbfunc os_irq_restore

os_irq_restore:
    .asmfunc
        MSR     PRIMASK, R0
        BX      LR
    .endasmfunc
