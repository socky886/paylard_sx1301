/*
 * time.h
 *
 *  Created on: 2018年2月1日
 *      Author: ases-jack
 */

#ifndef LIB_SYSTIME_H_
#define LIB_SYSTIME_H_

#include "types_def.h"
#include "FreeRTOS.h"
#include "os_task.h"


#pragma pack(push, 4)
typedef struct {
	uint32 sec;
	uint16 ms;
}SYS_TIME;
#pragma pack(pop)


#define TIMING_TYPE_ABSOLUTE   0x11
#define TIMING_TYPE_RELATIVE   0x22
#define TIMING_TYPE_EVENLY     0x33


/* GNSS time is base on 2017-1-1 12:00:00
 *  Systime - Gnsstime
 */
#define GNSS_TIME_OFFSET    41860800

/*
 * Funcion
 */
extern SYS_TIME GetSysTime(void);

extern uint8 AbsoluteTiming(SYS_TIME time);

extern uint8 RelativeTiming(SYS_TIME time);

extern uint8 SetEvenTimingArgument(uint32 period);

extern void EvenTiming(uint32 TimeDivisionFlag);

extern void SaveTimeToFram(void);

extern void RestoreTimeFromFram(void);

extern void SetTimeFromGnssTime(uint32 gnsstime);

extern void SetTimeFromGncBroadcast(uint32 time);


#endif /* LIB_SYSTIME_H_ */
