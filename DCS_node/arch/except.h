/*
 * except.h
 *
 *  Created on: 2018骞�2鏈�22鏃�
 *      Author: ases-jack
 */

#ifndef EXCEPT_H_
#define EXCEPT_H_

#include "types_def.h"

/*
 * Exception frame saved at the top of the exception/interrupt stacks.
 */
typedef struct arm_exception_frame {
	uint32	sp;		/* saved stack pointer (r13) */
	uint32	lr;		/* saved link register (r14) */
	uint32  cpsr;   /* saved cpsr in except */
	uint32	spsr;	/* saved cpsr */
	uint32	r[13];	/* saved r0 - r12 */
	uint32	pc;		/* saved pc (r15) */
} EXCEPTION_FRAME;


#define FSR_CAUSE_MASK(x)	(((x & (1 << 10)) >> 6) | (x & 0xF))

#define CPSR_MODE(_x)	((_x) & 0x1f)
#define CPSR_MODE_USER		0x10
#define CPSR_MODE_FIQ		0x11
#define CPSR_MODE_IRQ		0x12
#define CPSR_MODE_SUPERVISOR	0x13
#define CPSR_MODE_ABORT		0x17
#define CPSR_MODE_UNDEFINED	0x1b
#define CPSR_MODE_SYSTEM	0x1f
#define CPSR_STATE_THUMB	0x20

extern EXCEPTION_FRAME except_frame;
extern uint32 except_fsr;
extern uint32 except_far;
extern uint32 except_entry;

/*
 * Function
 */
static const char *arm_abort_decode_status(uint32 fsr);
static const char *arm_decode_mode(uint32 status_reg);
static void arm_exception_abort(EXCEPTION_FRAME *frame,	const char *kind, uint32 fsr, uint32 far);
static void save_except_context(EXCEPTION_FRAME *frame, uint32 fsr, uint32 far, uint32 entry);

/*
 * Asm function
 */
/*
 * Read Instruction Fault Status
 */
extern uint32 arm_read_ifsr(void);
/*
 * Read Data Fault Status
 */
extern uint32 arm_read_dfsr(void);
/*
 * Read Data Fault Address
 */
extern uint32 arm_read_dfar(void);
/*
 * Read Instruction Fault Address
 */
extern uint32 arm_read_ifar(void);
/*
 * Restore exception context
 */
extern void restore_last_except_context(void);
/*
 * Init exception context
 */
extern void init_last_except_context(void);
#endif /* EXCEPT_H_ */
