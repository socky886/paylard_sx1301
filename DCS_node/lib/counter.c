/*
 * counter.c
 *
 *  Created on: 2018骞�1鏈�25鏃�
 *      Author: ases-jack
 */
#include "counter.h"
#include "FreeRTOS.h"
#include "os_task.h"
#include "MR25H256.h"
#include "sys_config.h"
#include "crc.h"
#include "hw.h"
#include "errorlog.h"

uint8 AllCounter[LAST_COUNTER];


void AddCounter(uint8 chn, uint8 value)
{
	//taskENTER_CRITICAL();
	if(chn >= LAST_COUNTER) {
		ErrorLog(__LINE__, (uint32)AddCounter, chn);
		return ;
	}
	AllCounter[chn] += value;
	//taskEXIT_CRITICAL();
}

uint8 GetCounter(uint8 chn)
{
	if(chn >= LAST_COUNTER) {
		ErrorLog(__LINE__, (uint32)AddCounter, chn);
		return 0;
	}
	return AllCounter[chn];
}

void InitCounter(void)
{
	uint8 software_start_counter = 0;
	memset(AllCounter, 0, LAST_COUNTER);
	MR_Read_Data(COUNTER_DATA_PART_BASE + CNT_SOFTWARE_START, &software_start_counter, 1);

	if(software_start_counter != 0xFF) {
		AllCounter[CNT_SOFTWARE_START] = software_start_counter;
	} else {
		AllCounter[CNT_SOFTWARE_START] = 0;
	}
}

void SaveCounterToFram(void)
{
	SaveDataToFram(AllCounter, LAST_COUNTER, COUNTER_DATA_PART_BASE, COUNTER_DATA_PART_INTERVAL);
}

void RestoreCounterFromFram(void)
{
	if(RestoreDataFromFram(AllCounter, LAST_COUNTER, COUNTER_DATA_PART_BASE, COUNTER_DATA_PART_INTERVAL) == FALSE) {
		ErrorLog(__LINE__, (uint32)RestoreCounterFromFram, 0);
		InitCounter();
	}
}

void AddSoftwareStartCounter(void)
{
	ADD(CNT_SOFTWARE_START);
	SaveCounterToFram();
}
