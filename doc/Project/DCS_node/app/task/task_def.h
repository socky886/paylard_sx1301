/*
 * task_def.h
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: ases-jack
 */

#ifndef APP_TASK_TASK_DEF_H_
#define APP_TASK_TASK_DEF_H_

#include "FreeRTOS.h"
#include "os_task.h"


#define MAX_TASK_NUM   2
/*
 * Main Task configuration
 */
#define MAIN_TASK_STACK_SIZE  (1024 * 4)
#define MAIN_TASK_PRIORITY    (tskIDLE_PRIORITY + 2)   // | portPRIVILEGE_BIT

#define IDLE_TASK_STACK_SIZE  32
#define IDLE_TASK_PRIORITY    (tskIDLE_PRIORITY)



#endif /* APP_TASK_TASK_DEF_H_ */
