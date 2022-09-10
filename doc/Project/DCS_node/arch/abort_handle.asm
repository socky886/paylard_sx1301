;---------------------------
; ASES
; Made by jack lee
;---------------------------

;
; typedef struct arm_exception_frame {
; 	uint32	sp;		/* saved stack pointer (r13) */
; 	uint32	lr;		/* saved link register (r14) */
; 	uint32  cpsr;   /* saved cpsr in except */
; 	uint32	spsr;	/* saved cpsr */
; 	uint32	r[13];	/* saved r0 - r12 */
; 	uint32	pc;		/* saved pc (r15) */
; } EXCEPTION_FRAME;
;
; Note that when returning to the mode in which the exception was
; taken that set the I and F bits to mask interrupts and clear the
; T bit so that we stay in ARM mode.
;

;-------------------------------
; undef abort
;-------------------------------
    .text
    .arm
    .ref	UndefAbortHandler
    .def    undef_abort
    .asmfunc
undef_abort
	stmfd	sp!, { r0-r12, lr }
	mrs	r0, spsr
	stmfd	sp!, { r0 }
	mrs r4, cpsr
	stmfd   sp!, { r4 }
	orr	r0, r0, #0xc0
	bic	r0, r0, #(1 << 5)
	mrs	r1, cpsr
	msr	cpsr_c, r0
	mov	r2, sp
	mov	r3, lr
	svc #1                          ; can not change mode in user mode, must be raise privilege
	msr	cpsr_c, r1
	stmfd	sp!, { r2-r3 }
	mov	    r0, sp
	bl		UndefAbortHandler
undef_abort_loop
	b       undef_abort_loop


;-------------------------------
; prefetch abort
;-------------------------------
	.ref    PrefetchAbortHandler
	.def    prefetch_abort
prefetch_abort
	sub	lr, lr, #4
	stmfd	sp!, { r0-r12, lr }
	mrs	r0, spsr
	stmfd	sp!, { r0 }
	mrs r4, cpsr
	stmfd   sp!, { r4 }
	orr	r0, r0, #0xc0
	bic	r0, r0, #(1 << 5)
	mrs	r1, cpsr
	msr	cpsr_c, r0
	mov	r2, sp
	mov	r3, lr
	svc #1                          ; can not change mode in user mode, must be raise privilege
	msr	cpsr_c, r1
	stmfd	sp!, { r2-r3 }
	mov	    r0, sp
	bl		PrefetchAbortHandler
prefetch_abort_loop
	b       prefetch_abort_loop


;-------------------------------
; data abort
;-------------------------------
	.ref    DataAbortHandler
	.ref    _dabort
	.def    data_abort
data_abort
	stmfd	sp!, { r0-r12, lr }     ; save r0-r12 and lr content to stack
	mrs	r0, spsr                    ; save spsr to r0
	stmfd	sp!, { r0 }             ; save spsr content to stack
	mrs r4, cpsr                    ; save current cpsr to stack to indicate which abort was enter
	stmfd   sp!, { r4 }
	orr	r0, r0, #0xc0               ; mask fiq
	bic	r0, r0, #(1 << 5)           ; change to arm if in thumb
	mrs	r1, cpsr                    ; save the cpsr which in abort mode
	msr	cpsr_c, r0                  ; cpsr_c is low 8-bit of cpsr, change mode to user mode
	mov	r2, sp                      ; save the context of abort occure, sp content to backtrace
	mov	r3, lr
	svc #1                          ; can not change mode in user mode, must be raise privilege
	msr	cpsr_c, r1                  ; change mode to abort mode
	stmfd	sp!, { r2-r3 }          ; save user mode 's sp & lr
	mov	    r0, sp                  ; pass the context of abort to function
	bl		DataAbortHandler
	ldmfd   r13!, { r2-r3 }
    ldmfd   r13!, {r4}
	ldmfd   r13!, {r0}
	ldmfd	sp!, { r0-r12, pc }^

;------------------------------
; Read CP15 register
;------------------------------
	.def arm_read_ifsr
arm_read_ifsr
	mrc		p15, #0, r0, c5, c0, #1
	bx		lr

	.def arm_read_dfsr
arm_read_dfsr
	mrc		p15, #0, r0, c5, c0, #0
	bx		lr

	.def arm_read_ifar
arm_read_ifar
	mrc		p15, #0, r0, c6, c0, #2
	bx		lr

	.def arm_read_dfar
arm_read_dfar
	mrc		p15, #0, r0, c6, c0, #0
	bx		lr


