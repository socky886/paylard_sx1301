/*
 * init.c
 *
 *  Created on: 2018楠烇拷1閺堬拷15閺冿拷
 *      Author: ases-jack
 */
#include "system.h"
#include "init.h"
#include "task_def.h"
#include "taskcode.h"
#include "hw.h"
#include "interrupt.h"
#include "print.h"
#include "counter.h"
#include "errorlog.h"
#include "glib.h"
#include "filter.h"
#include "sys_pcr.h"
#include "can_cmd.h"
#include "except.h"
#include "systime.h"
#include "can_lib.h"
#include "can_data_process.h"
#include "inject.h"
#include "MR25H256.h"
#include "sx1301_lora.h"

uint8 ResetType = 0;

void InitHw(void)
{
	uint8 fram_dummy[5] = {0x55,0xAA,0xA5,0x5A,0xFF,};
	GetResetType();
#ifdef ENABLE_WATCHDOG
	InitWatchDog();
#endif
	sciInit();
	gioInit();
	spiInit();
	hetInit();
	adcInit();
	canInit();
	canMessageBoxInit();


    MR_Write_Data(0x0, fram_dummy, 5);

    memset(fram_dummy, 0, 5);

	MR_Read_Data(0x0, fram_dummy, 5);

//	if(ResetType != RESET_POWERON){
//	    RestoreInjectDataFromFram();
//	}
//	else{
//	    InitInjectData();
//	}
//	RestoreInjectDataFromFram();

    FeedWatchDog();


	sx1301init();


}

void InitBuf(void)
{
	RebootDetect();
	InitErrorLogSystem();
	/*
	 * CAUTION:
	 * Not enough power will make OBC reset
	 * and obc will get the reset type is power on
	 */
	if(ResetType != RESET_POWERON){
		RestoreCounterFromFram();
	} else {
		InitCounter();
	}
	restore_last_except_context();
//	InitFilter();
	InitCanCmdQueue();
	CanInitQueue();
}


void InitSemaphore(void)
{
	xCanSemaphore = xSemaphoreCreateBinary();
	xGnssSemaphore = xSemaphoreCreateBinary();
	xTeleCmdSemaphore = xSemaphoreCreateBinary();
	xSemaphoreRfOneFrameBin = xSemaphoreCreateBinary();
	xSemaphoreUart = xSemaphoreCreateBinary();
}

static uint16 GetResetType(void)
{
	uint16 reset;
	reset = SYS_EXCEPTION;
    if ((reset & POWERON_RESET) != 0U) {
    	ResetType = RESET_POWERON;
    }
    else if ((reset & OSC_FAILURE_RESET) != 0U) {
    	ResetType = RESET_OSC_FAILURE;
    }
    else if ((reset & WATCHDOG_RESET) != 0U) {
        /* Reset caused due
         *  1) windowed watchdog violation - Add user code here to handle watchdog violation.
         *  2) ICEPICK Reset - After loading code via CCS / System Reset through CCS
         */
        /* Check the WatchDog Status register */
        if(WATCHDOG_STATUS != 0U) {
        	ResetType = RESET_WATCHDOG;
        } else {
        	ResetType = RESET_ICEPICK;
        }
    }
    /*SAFETYMCUSW 139 S MR:13.7 <APPROVED> "Hardware status bit read check" */
    else if ((reset & CPU_RESET) != 0U) {
    	ResetType = RESET_CPU;
    }
    /*SAFETYMCUSW 139 S MR:13.7 <APPROVED> "Hardware status bit read check" */
    else if ((reset & SW_RESET) != 0U) {
    	ResetType = RESET_SW;
    }
    else if ((reset & EXTERNAL_RESET) != 0U) {
    	ResetType = RESET_EXT;
    }
    else {
    	ResetType = RESET_UNKNOWN;
    }
	SYS_EXCEPTION = 0xFFFFU;
	return ResetType;
}

/*
 * #define EXTERNAL_RESET       0x0008U
 * #define RESET_POWERON        0x11U
 * #define RESET_OSC_FAILURE    0x22U
 * #define RESET_WATCHDOG       0x44U
 * #define RESET_ICEPICK        0x88U
 * #define RESET_CPU            0x99U
 * #define RESET_SW             0xAAU
 * #define RESET_EXT            0x55U
 * #define RESET_UNKNOWN        0x00U
 */
char* GetResetTypeName(void)
{
	char* name;
	switch(ResetType) {
	case RESET_POWERON:
		name = GET_CONST_STR(RESET_POWERON);
		break;
	case RESET_OSC_FAILURE:
		name = GET_CONST_STR(RESET_OSC_FAILURE);
		break;
	case RESET_WATCHDOG:
		name = GET_CONST_STR(RESET_WATCHDOG);
		break;
	case RESET_ICEPICK:
		name = GET_CONST_STR(RESET_ICEPICK);
		break;
	case RESET_CPU:
		name = GET_CONST_STR(RESET_CPU);
		break;
	case RESET_SW:
		name = GET_CONST_STR(RESET_SW);
		break;
	case RESET_EXT:
		name = GET_CONST_STR(RESET_EXT);
		break;
	default:
		name = GET_CONST_STR(RESET_UNKNOWN);
		break;
	}
	return name;
}

void RebootDetect(void)
{
	uint8 reboot_info = 0;
	if(ResetType != RESET_POWERON) {
	    MR_Read_Data(FAST_REBOOT_INFO_BASE, &reboot_info, 1);
		if(reboot_info != 0xFF) {
			if((reboot_info & 0xF0) == 0xF0) {
				reboot_info = (reboot_info & 0xF0) | ((reboot_info & 0x0F) + 1);
				if((reboot_info & 0x0F) > MAX_QUICK_REBOOT_TIMES) {
					print("<<MAXQUICKREBOOT>>");
					ResetType = RESET_POWERON;
					reboot_info = 0xF0;
				}
			} else {
				reboot_info = 0xF0;
			}
		} else {
			reboot_info = 0xF0;
		}
	} else {
		reboot_info = 0xF0;
	}
	MR_Write_Data(FAST_REBOOT_INFO_BASE, &reboot_info, 1);
}

void ClearQuickRebootFlag(void)
{
	uint8 reboot_info = 0;
	MR_Write_Data(FAST_REBOOT_INFO_BASE, &reboot_info, 1);
}
