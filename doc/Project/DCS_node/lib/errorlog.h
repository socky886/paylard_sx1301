/*
 * errorlog.h
 *
 *  Created on: 2018年2月26日
 *      Author: ases-jack
 */

#ifndef LIB_ERRORLOG_H_
#define LIB_ERRORLOG_H_

#include "types_def.h"
#include "list.h"

typedef struct {
	uint32 time;
	uint32 line;
	uint32 func;
	uint32 data;
	struct list_node node;
}ERROR_INFO;

#define MAX_ERROR_LOG_ENTRY    20

extern struct list_node error_log_list;

extern uint32 error_log_cnt;

extern void InitErrorLogSystem(void);

extern void ErrorLog(uint32 line, uint32 func, uint32 data);

extern ERROR_INFO* GetNewestErrorLog();

extern void ServiceErrorLog(void);

#endif /* LIB_ERRORLOG_H_ */
