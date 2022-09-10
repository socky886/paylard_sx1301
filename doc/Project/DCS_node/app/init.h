/*
 * init.h
 *
 *  Created on: 2018骞�1鏈�15鏃�
 *      Author: ases-jack
 */

#ifndef APP_INIT_H_
#define APP_INIT_H_

#include "system.h"
#include "sys_core.h"
#include "spi.h"
#include "sci.h"
#include "gio.h"
#include "rti.h"
#include "can.h"
#include "het.h"
#include "adc.h"
#include "reg_tcram.h"
#include "sys_pmu.h"

#include "MR25H256.h"

#include "FreeRTOS.h"
#include "os_task.h"
#include "os_semphr.h"

#define GET_CONST_STR(f)  (#f)

extern uint8 ResetType;

extern void InitHw(void);

extern void InitBuf(void);

extern void InitSwitch(void);

extern void InitSemaphore(void);

static uint16 GetResetType(void);

extern char* GetResetTypeName(void);

extern void RebootDetect(void);

extern void ClearQuickRebootFlag(void);
#endif /* APP_INIT_H_ */
