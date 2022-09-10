/*
 * glib.c
 *
 *  Created on: 2018年1月16日
 *      Author: ases-jack
 */
#include "glib.h"
#include "errorlog.h"
#include "rti.h"
#include "print.h"


void Delay(volatile int time)
{
	int temp = time;
	while(time--){
		volatile int mil_time = temp;
		while(mil_time--);
	}
}

uint8 CheckSum(uint8* data, uint32 len)
{
	int i = 0;
	uint8 sum = 0;
	if(data == NULL){
		ErrorLog(__LINE__, (uint32)CheckSum, (uint32)data);
		return 0;
	}
	for(i = 0; i < len; i++){
		sum += data[i];
	}
	return sum;
}

void SoftReset(void)
{
#define SYS_EXCEPTION_CTRL_REG        (*(volatile uint32 *)0xFFFFFFE0U)
	BaseType_t xRunningPrivileged = (BaseType_t)0;
	xRunningPrivileged = prvRaisePrivilege();
	SYS_EXCEPTION_CTRL_REG |= 0x8000;
	portRESET_PRIVILEGE( xRunningPrivileged );
}

void InitWatchDog(void)
{
#ifdef ENABLE_WATCHDOG
	print("Init watchdog\n");
	/* Initialize and start DWD */
	dwwdInit(Generate_Reset, 4095, Size_100_Percent);   // 0.728s
	dwdCounterEnable();
#endif

}

void FeedWatchDog(void)
{
#ifdef ENABLE_WATCHDOG
	BaseType_t xRunningPrivileged = (BaseType_t)0;
	xRunningPrivileged = prvRaisePrivilege();
	dwdReset();
	portRESET_PRIVILEGE( xRunningPrivileged );
#endif
}
