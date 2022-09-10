/*
 * inject.c
 *
 *  Created on: 2018骞�2鏈�2鏃�
 *      Author: ases-jack
 */

#include "inject.h"
#include "sys_config.h"
#include "MR25H256.h"
#include "crc.h"
#include "errorlog.h"
#include "compiler.h"
#include <stddef.h>
#include "hw.h"
#include "sx1301_config.h"

INJECT_DATA inject;

void InitInjectData(void)
{
    #if !DCS_CPU
        inject.tx_freq = 399000000;//下行频点
    #else
        inject.tx_freq = 400000000;//下行频点
    #endif

        inject.TxSendEnable = OFF;
        inject.tx_power = 27;                /*!< 0x04   */
        inject.tx_datarate = DR_LORA_SF12;   /*!< 0x05   */
        inject.tx_bandwidth = BW_250KHZ;
        inject.TxSendInterval = 10;          /*!< 0x0E  Tx send interval (s)*/
}

void RestoreInjectDataFromFram(void)
{
    uint8 ReadBuf[INJECT_DATA_BASE_SIZE];
    if(RestoreDataFromFram(ReadBuf, INJECT_DATA_BASE_SIZE, INJECT_DATA_PART_BASE, INJECT_DATA_PART_INTERVAL) == FALSE) {
        InitInjectData();
        SaveInjectDataToFram();
        ErrorLog(__LINE__, (uint32)RestoreInjectDataFromFram, 0);
    } else {
        memcpy((void*)&inject, ReadBuf, INJECT_DATA_BASE_SIZE);
    }
}

void SaveInjectDataToFram(void)
{
    SaveDataToFram((void*)&inject, INJECT_DATA_BASE_SIZE, INJECT_DATA_PART_BASE, INJECT_DATA_PART_INTERVAL);
}
