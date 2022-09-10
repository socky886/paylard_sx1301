/*
 * counter.h
 *
 *  Created on: 2018骞�1鏈�25鏃�
 *      Author: ases-jack
 */

#ifndef LIB_COUNTER_H_
#define LIB_COUNTER_H_

#include "types_def.h"

#define ADD(chn)    AddCounter(chn, 1)
#define CNT(chn)    GetCounter(chn)

#define CNT_EXCEPTION_ERROR          0
#define CNT_TRANSMIT_DATA            1
#define CNT_GNC_BROADCAST            2
#define CNT_INJECT_DATA              3
#define CNT_SX1301_REINIT            4
#define CNT_SOFTWARE_START           5
#define CNT_TASK_TIME_OUT            6
#define CNT_CORRECT_RECEIVE          7
#define CNT_WRONG_RECEIVE            8
#define CNT_IF_CHAIN_0               9
#define CNT_IF_CHAIN_1               10
#define CNT_IF_CHAIN_2               11
#define CNT_IF_CHAIN_3               12
#define CNT_IF_CHAIN_4               13
#define CNT_IF_CHAIN_5               14
#define CNT_IF_CHAIN_6               15
#define CNT_IF_CHAIN_7               16
#define CNT_IF_CHAIN_8               17
#define LAST_COUNTER                 18

extern uint8 AllCounter[LAST_COUNTER];

extern void AddCounter(uint8 chn, uint8 value);
extern uint8 GetCounter(uint8 chn);
extern void RestoreCounterFromFram(void);
extern void SaveCounterToFram(void);
extern void InitCounter(void);
extern void AddSoftwareStartCounter(void);

#endif /* LIB_COUNTER_H_ */
