/*
 * os_hook.c
 *
 *  Created on: 2018年2月4日
 *      Author: ases-jack
 */

#include "os_hook.h"
#include "glib.h"
#include "taskcode.h"
#include "sys_config.h"

uint32 idle_cnter = 0;

void vApplicationTickHook( void )
{

}


void vApplicationStackOverflowHook( TaskHandle_t pxTask, signed char *pcTaskName )
{

}

void vApplicationMallocFailedHook( void )
{

}

void vApplicationIdleHook( void )
{
	FeedWatchDog();
	idle_cnter++;
}

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
	*ppxIdleTaskTCBBuffer = &xIdleTaskBuffer;

	*ppxIdleTaskStackBuffer = task_params[1].puxStackBuffer;

	*pulIdleTaskStackSize = task_params[1].usStackDepth;
}
