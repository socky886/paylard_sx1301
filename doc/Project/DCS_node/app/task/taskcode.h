/*
 * taskcode.h
 *
 *  Created on: 2018年1月16日
 *      Author: ases-jack
 */

#ifndef APP_TASK_TASKCODE_H_
#define APP_TASK_TASKCODE_H_

#include "task_def.h"

#define TASKTIME (100)
#define TOSECMUL (1000 / TASKTIME)

extern const TaskParameters_t task_params[MAX_TASK_NUM];

extern StaticTask_t xIdleTaskBuffer;

extern void vMainTask( void * pvParameters );

extern void vIdleTask( void * pvParameters );

extern void InitTask(void);

#endif /* APP_TASK_TASKCODE_H_ */
