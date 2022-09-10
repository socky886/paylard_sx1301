/*
 * time.c
 *
 *  Created on: 2018骞�2鏈�1鏃�
 *      Author: ases-jack
 */

#include "systime.h"
#include "sys_config.h"
#include "stdlib.h"
#include "MR25H256.h"
#include "crc.h"
#include "errorlog.h"
#include "hw.h"

/*
 * SYS_TIME = {0, 0} is mean that time is UTC:2000-1-1 12:00:00
 */
static uint32 RollOverCnter = 0;
static TickType_t LastTickCount = 0;
static sint64 time_offset = 0;   /* unit: ms */

static sint32 period_timing;
static uint32 period_cnter = 0;

static uint64 GetSysTimeMs(void)
{
	uint64 curr = 0;
	TickType_t current = 0;
	//taskENTER_CRITICAL();
	if(xTaskGetCurrentTaskHandle() != NULL) {
		current = xTaskGetTickCount();
	} else {
		current = xTaskGetTickCountFromISR();
	}
	if(current < LastTickCount){
		RollOverCnter++;
	}
	if(RollOverCnter > 0xFFFF) {
		RollOverCnter = 0;
		ErrorLog(__LINE__, (uint32)GetSysTimeMs, RollOverCnter);
		return 0;
	}
	LastTickCount = current;
	if(time_offset > 0) {
		curr = (((uint64)RollOverCnter * 0x100000000U) + (uint64)current + (uint64)time_offset);
	} else {
		curr = (((uint64)RollOverCnter * 0x100000000U) + (uint64)current - (uint64)llabs(time_offset));
	}
	//taskEXIT_CRITICAL();
	return curr;
}

SYS_TIME GetSysTime(void)
{
	SYS_TIME t;
	uint64 time = GetSysTimeMs();
	t.sec = (uint32)(time / 1000U);
	t.ms = (uint16)(time % 1000U);
	return t;
}

uint8 AbsoluteTiming(SYS_TIME time)
{
	uint64 curr = GetSysTimeMs();
	sint64 diff = 0;
	if(time.ms >= 1000){
		ErrorLog(__LINE__, (uint32)AbsoluteTiming, time.ms);
		return FALSE;
	}
	uint64 newtime = (uint64)time.sec * 1000U + (uint64)time.ms;
	diff = (sint64)newtime - (sint64)curr;

	time_offset += diff;
	return TRUE;
}

uint8 RelativeTiming(SYS_TIME time)
{
	uint8 sign = (time.sec & 0x80000000) == 0x80000000;
	if(time.ms >= 1000) {
		ErrorLog(__LINE__, (uint32)RelativeTiming, time.ms);
		return FALSE;
	}
	sint64 ms = (uint64)(time.sec & 0x7FFFFFFF) * 1000U + (uint64)time.ms;
	if(sign == 0) {
		time_offset += ms;
	} else {
		time_offset -= ms;
	}
	return TRUE;
}

uint8 SetEvenTimingArgument(uint32 period)
{
	if((period & 0x7FFFFFFF) > 0xFFFFFF) {
		ErrorLog(__LINE__, (uint32)SetEvenTimingArgument, period);
		return FALSE;
	}
	if((period & 0x80000000) == 0) {
		period_timing = period;
	} else {
		period_timing = ((sint32)(period & 0x7FFFFFFF)) * (-1);
	}
	return TRUE;
}

void EvenTiming(uint32 TimeDivisionFlag)
{
	if((TimeDivisionFlag % 2 == 0) && (period_timing != 0)) {
		if(period_cnter == 0) {
			period_cnter = (uint32)labs(period_timing);
			if(period_timing >= 0) {
				time_offset++;
			} else {
				time_offset--;
			}
		}
		period_cnter--;
	}
}

void SaveTimeToFram(void)
{
	uint8 CheckData[24];
	memcpy(CheckData, (uint8*)&RollOverCnter, sizeof(RollOverCnter));
	memcpy(CheckData + 4, (uint8*)&LastTickCount, sizeof(LastTickCount));
	memcpy(CheckData + 8, (uint8*)&time_offset, sizeof(time_offset));
	memcpy(CheckData + 16, (uint8*)&period_timing, sizeof(period_timing));
	memcpy(CheckData + 20, (uint8*)&period_cnter, sizeof(period_cnter));
	SaveDataToFram(CheckData, 24, TIME_DATA_PART_BASE, TIME_DATA_PART_INTERVAL);
}

void RestoreTimeFromFram(void)
{
	uint8 ReadData[24];
	if(RestoreDataFromFram(ReadData, 24, TIME_DATA_PART_BASE, TIME_DATA_PART_INTERVAL) == FALSE) {
		ErrorLog(__LINE__, (uint32)RestoreTimeFromFram, 0);
		return ;
	}
	RollOverCnter = *((uint32*)ReadData);
	LastTickCount = *((TickType_t*)(ReadData + 4));
	time_offset   = *((sint64*)(ReadData + 8));
	period_timing = *((sint32*)(ReadData + 16));
	period_cnter  = *((uint32*)(ReadData + 20));

	time_offset += (sint64)LastTickCount + 500U;    /* Plus 500ms to include the reboot time */
	LastTickCount = 0;
}

void SetTimeFromGnssTime(uint32 gnsstime)
{
	SYS_TIME curr = GetSysTime();
	if(gnsstime > GNSS_TIME_OFFSET) {
		curr.sec = gnsstime - GNSS_TIME_OFFSET;
		AbsoluteTiming(curr);
	}
}

void SetTimeFromGncBroadcast(uint32 time)
{
    SYS_TIME curr = GetSysTime();
    curr.sec = time;
    AbsoluteTiming(curr);

}
