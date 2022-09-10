/*
 * errorlog.c
 *
 *  Created on: 2018骞�2鏈�26鏃�
 *      Author: ases-jack
 */

#include "errorlog.h"
#include "stdlib.h"
#include "sys_config.h"
#include "string.h"
#include "systime.h"
#include "counter.h"
#include "hw.h"
#include "print.h"
#include "ncx_slab.h"

struct list_node error_log_list;

uint32 error_log_cnt = 0;

static uint8 errorlogbuff[sizeof(ERROR_INFO) * 64];
static ncx_slab_pool_t *sp;

void InitErrorLogSystem(void)
{
	error_log_list = LIST_INITIAL_VALUE(error_log_list);

    sp = (ncx_slab_pool_t*) errorlogbuff;
	sp->addr = errorlogbuff;
	sp->min_shift = 3;
	sp->end = errorlogbuff + sizeof(ERROR_INFO) * 64;
	ncx_slab_init(sp);
}

void ErrorLog(uint32 line, uint32 func, uint32 data)
{
//	SYS_TIME time;
//	ERROR_INFO* error = (ERROR_INFO*)ncx_slab_alloc(sp, sizeof(ERROR_INFO));
//	if(error == NULL) {
//		return ;
//	}
//	memset(error, 0, sizeof(ERROR_INFO));
//	if(error_log_cnt > MAX_ERROR_LOG_ENTRY) {
//		struct list_node* old = list_remove_tail(&error_log_list);
//		vPortFree(containerof(old, ERROR_INFO, node));
//		error_log_cnt--;
//	}
//	time = GetSysTime();
//	error->time = time.sec;
//	error->line = line;
//	error->func = func;
//	error->data = data;
//
//  print("[E%d:%d:%x:%x]", time, line, func, data);
//	list_add_head(&error_log_list, &error->node);
//	error_log_cnt++;
}

ERROR_INFO* GetNewestErrorLog()
{
	return list_peek_head_type(&error_log_list, ERROR_INFO, node);
}

void ServiceErrorLog(void)
{

	//SendDataToEarth((uint8*)&VC3.vcbuf, MAX_VCDU, TRUE);
}
