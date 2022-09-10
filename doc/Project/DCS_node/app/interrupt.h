/*
 * interrupt.h
 *
 *  Created on: 2018年1月21日
 *      Author: ases-jack
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include "types_def.h"
#include "FreeRTOS.h"
#include "os_task.h"
#include "os_semphr.h"

#define FMC_Reg ((volatile Fapi_FmcRegistersType *)0xFFF87000)

typedef struct {
	uint8 N    :1;
	uint8 Zero :1;
	uint8 C    :1;
	uint8 V    :1;
	uint8 Q    :1;
	uint8 IT1_0:2;
	uint8 J    :1;
	uint8 DNM  :4;
	uint8 GE   :4;
	uint8 IT7_2:6;
	uint8 E    :1;
	uint8 A    :1;
	uint8 I    :1;
	uint8 F    :1;
	uint8 T    :1;
	uint8 M    :5;
}PSR;

typedef struct {
	union {
		PSR    psr;
		uint32 psrv;
	}     spsr;
	uint32  r0;
	uint32  r1;
	uint32  r2;
	uint32  r3;
	uint32  r4;
	uint32  r5;
	uint32  r6;
	uint32  r7;
	uint32  r8;
	uint32  r9;
	uint32  r10;
	uint32  r11;
	uint32  r12;
	uint32  lr;
}INT_REGS;

extern uint8 uart_receive_flag;

extern uint32 singleErrorCnter;
extern volatile uint64* singleErrorAddr;
extern volatile uint32* FlashCorrctableErrorAddr;
extern uint32 FlashUnCorrctableErrorAddr;
extern uint32 UncorrectableErrorAddr;

extern uint32 rw_speed[2];

extern uint32 idle_cnter;

extern SemaphoreHandle_t xCanSemaphore;
extern SemaphoreHandle_t xGnssSemaphore;
extern SemaphoreHandle_t xTeleCmdSemaphore;
extern SemaphoreHandle_t xSemaphoreRfOneFrameBin;
extern SemaphoreHandle_t xSemaphoreUart;

#endif /* INTERRUPT_H_ */
