/*
 * glib.h
 *
 *  Created on: 2018年1月16日
 *      Author: ases-jack
 */

#ifndef LIB_GLIB_H_
#define LIB_GLIB_H_

#include "types_def.h"
#include "FreeRTOS.h"
#include "os_task.h"

#define Min(a, b)    ((a < b)? a:b)
#define Max(a, b)    ((a > b)? a:b)

#pragma SWI_ALIAS(prvRaisePrivilege, 1);
extern BaseType_t prvRaisePrivilege( void );
#define portRESET_PRIVILEGE( xRunningPrivileged ) if( xRunningPrivileged == 0 ) portSWITCH_TO_USER_MODE()

#define WriteMemory(addr, data)    *((volatile uint32*) (addr))=(data)
#define ReadMemory(addr)         *((volatile uint32*) (addr))

extern void Delay(volatile int time);

extern uint8 CheckSum(uint8* data, uint32 len);

extern void SoftReset(void);

extern void InitWatchDog(void);

extern void FeedWatchDog(void);

#endif /* LIB_GLIB_H_ */
