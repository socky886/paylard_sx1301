/*
 * except.c
 *
 *  Created on: 2018骞�2鏈�22鏃�
 *      Author: ases-jack
 */
#include <stdio.h>
#include "esm.h"
#include "reg_tcram.h"
#include "reg_flash.h"
#include "except.h"
#include "sys_config.h"
#include "crc.h"
#include "print.h"
#include "counter.h"
#include "glib.h"
#include "FreeRTOS.h"
#include "os_task.h"
#include "compiler.h"
#include "hw.h"

static char status_str[32];

/*
 * Exception context
 */
EXCEPTION_FRAME except_frame;
uint32 except_fsr = 0;
uint32 except_far = 0;
uint32 except_entry = 0;

static const char * arm_abort_decode_status(uint32 fsr)
{
	/* mask bit 10 & low 4 bits for cause decoding */
	uint32 fs = FSR_CAUSE_MASK(fsr);

	switch (fs) {
	case 0x01:
		return ("Alignment fault with valid far");
	case 0x00:
		return ("Background fault of MPU fault with valid far");
	case 0x0d:
		return ("Permission fault of MPU fault with valid far");
	case 0x02:
		return ("Debug event");
	case 0x08:
		return ("Synchronous external abort with valid far");
	case 0x14:
		return ("IMPLEMENTATION DEFINED and Lock down");
	case 0x1A:
		return ("IMPLEMENTATION DEFINED and Coprocessor abort");
	case 0x19:
		return ("Memory access synchronous parity error with valid far");
	case 0x16:
		return ("Asynchronous external abort with unknown far");
	case 0x18:
		return ("Memory access asynchronous parity error with unknown far");
	}

	snprintf(status_str, sizeof(status_str), "<unknown cause 0x%02x>", fs);

	return (status_str);
}

static const char * arm_decode_mode(uint32 status_reg)
{
	switch (CPSR_MODE(status_reg)) {
	case CPSR_MODE_USER:
		return("user");
	case CPSR_MODE_FIQ:
		return("FIQ");
	case CPSR_MODE_IRQ:
		return("IRQ");
	case CPSR_MODE_SUPERVISOR:
		return("supervisor");
	case CPSR_MODE_ABORT:
		return("abort");
	case CPSR_MODE_UNDEFINED:
		return("undefined");
	case CPSR_MODE_SYSTEM:
		return("system");
	}
	return("<unknown>");
}

void DataAbortHandler(EXCEPTION_FRAME* frame)
{
	uint32 fsr = arm_read_dfsr();
	uint32 far = arm_read_dfar();
	uint32 esmsr3 = esmREG->SR1[2];

	if((esmsr3 & 0x08) == 0x08) {
		/* uncorrectable ECC error on B0TCM */
		if((tcram1REG->RAMCTRL & 0x100) == 0x100) {
			tcram1REG->RAMERRSTATUS = 0x20;
			esmREG->SR1[2] = 0x08;
			esmREG->EKR = 0x05;   /* The nERROR pin will become inactive once the LTC counter expires */
			return ;
		}
	} else if((esmsr3 & 0x20) == 0x20) {
		/* uncorrectable ECC error on B1TCM */
		if((tcram2REG->RAMCTRL & 0x100) == 0x100) {
			tcram2REG->RAMERRSTATUS = 0x20;
			esmREG->SR1[2] = 0x20;
			esmREG->EKR = 0x05;   /* The nERROR pin will become inactive once the LTC counter expires */
			return ;
		}
	} else if((esmsr3 & 0x80) == 0x80) {
		/* uncorrectable ECC error on ATCM */
		uint32 diagctrl = flashWREG->FDIAGCTRL;
		if(((diagctrl >> 16) & 0xF) == 5) {
			/* Flash diagnostic mode is enabled */
			flashWREG->FEDACSTATUS = 1 << 8;
			esmREG->SR1[2] = 0x80;
			esmREG->EKR = 0x05;   /* The nERROR pin will become inactive once the LTC counter expires */
			return ;
		}
	}
	frame->pc -= 8;
	ADD(CNT_EXCEPTION_ERROR);
	if (fsr == 0x408) {
		//Asynchronous Parity/ECC Error
	}
	save_except_context(frame, fsr, far, (uint32)DataAbortHandler);
	SaveCounterToFram();
	arm_exception_abort(frame, "data abort", fsr, far);
	SoftReset();
}

void UndefAbortHandler(EXCEPTION_FRAME* frame)
{
	uint32 fsr, far;
	ADD(CNT_EXCEPTION_ERROR);
	frame->pc -= (frame->spsr & CPSR_STATE_THUMB) ? 2: 4;
	fsr = ~0;
	far = frame->pc;
	save_except_context(frame, fsr, far, (uint32)UndefAbortHandler);
	SaveCounterToFram();
	arm_exception_abort(frame, "undefined instruction", fsr, far);
	SoftReset();
}

void PrefetchAbortHandler(EXCEPTION_FRAME* frame)
{
	uint32 fsr = arm_read_ifsr();
	uint32 far = arm_read_ifar();
	ADD(CNT_EXCEPTION_ERROR);
	frame->pc -= 4;
	if (fsr == 0x409) {
		//Synchronous Parity/ECC Error
	}
	save_except_context(frame, fsr, far, (uint32)PrefetchAbortHandler);
	SaveCounterToFram();
	arm_exception_abort(frame, "prefetch abort", fsr, far);
	SoftReset();
}

static void arm_exception_abort(EXCEPTION_FRAME *frame, const char *kind, uint32 fsr, uint32 far)
{
	print("\nARM %s in %s mode at 0x%08x due to %s:\n", kind, arm_decode_mode(frame->spsr), frame->pc, arm_abort_decode_status(fsr));
	print(" far 0x%08x fsr 0x%08x\n", far, fsr);
	print(" r0 0x%08x 0x%08x 0x%08x 0x%08x\n", frame->r[0], frame->r[1], frame->r[2], frame->r[3]);
	print(" r4 0x%08x 0x%08x 0x%08x 0x%08x\n", frame->r[4], frame->r[5], frame->r[6], frame->r[7]);
	print(" r8 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", frame->r[8], frame->r[9], frame->r[10], frame->r[11], frame->r[12]);
	print(" sp 0x%08x lr 0x%08x spsr 0x%08x  cpsr  0x%08x\n", frame->sp, frame->lr, frame->spsr, frame->cpsr);
}

static void save_except_context(EXCEPTION_FRAME *frame, uint32 fsr, uint32 far, uint32 entry)
{
	uint8 buf[sizeof(EXCEPTION_FRAME) + 12];
	memcpy(buf, frame, sizeof(EXCEPTION_FRAME));
	memcpy(buf + sizeof(EXCEPTION_FRAME), &fsr, 4);
	memcpy(buf + sizeof(EXCEPTION_FRAME) + 4, &far, 4);
	memcpy(buf + sizeof(EXCEPTION_FRAME) + 8, &entry, 4);
	SaveDataToFram(buf, sizeof(EXCEPTION_FRAME) + 12, EXCEPT_CONTEXT_PART_BASE, EXCEPT_CONTEXT_PART_INTERVAL);
}

void restore_last_except_context(void)
{
	uint8 ReadBuf[sizeof(EXCEPTION_FRAME) + 12];
	if(RestoreDataFromFram(ReadBuf, sizeof(EXCEPTION_FRAME) + 12, EXCEPT_CONTEXT_PART_BASE, EXCEPT_CONTEXT_PART_INTERVAL) == TRUE) {
		memcpy(&except_frame, ReadBuf, sizeof(EXCEPTION_FRAME));
		memcpy(&except_fsr, ReadBuf + sizeof(EXCEPTION_FRAME), 4);
		memcpy(&except_far, ReadBuf + sizeof(EXCEPTION_FRAME) + 4, 4);
		memcpy(&except_entry, ReadBuf + sizeof(EXCEPTION_FRAME) + 8, 4);
	} else {
		init_last_except_context();
	}
}

void init_last_except_context(void)
{
	memset(&except_frame, 0, sizeof(EXCEPTION_FRAME));
	except_fsr = 0;
	except_far = 0;
}
