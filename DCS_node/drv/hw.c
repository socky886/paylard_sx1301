/*
 * hw.c
 *
 *  Created on: 2018骞�1鏈�21鏃�
 *      Author: ases-jack
 */
#include "hw.h"
#include "het.h"
#include "spi.h"
#include "glib.h"
#include "can_lib.h"
#include "types_def.h"
#include "FreeRTOS.h"
#include "os_task.h"
#include "os_semphr.h"
#include "interrupt.h"
#include "print.h"
#include "crc.h"
#include "errorlog.h"
#include "filter.h"
#include "counter.h"
#include "MR25H256.h"
#include "sx1301_lora.h"
#include "inject.h"
#include "can_data_process.h"

uint16 all_ad_data[MAX_AD_CHN];



DCS_DATA dcsData;

/*
 * if can, 0 is rx, 1 is tx
 */
const SwitchInfoType sw[] = {
    {hetPORT1,  PIN_HET_10,      GIO,  FALSE, FALSE},   //0  A 接收锁相环锁定标志  118
    {hetPORT1,   PIN_HET_9,      GIO,  FALSE, FALSE},   //1  A 发射锁相环锁定标志   35
    {hetPORT1,   PIN_HET_8,      GIO,  FALSE, FALSE},   //2  A XOSC READY标志 106
    {hetPORT1,   PIN_HET_7,      GIO,  FALSE, FALSE},   //3  A RXTX Lock标志  33
    {hetPORT1,  PIN_HET_16,      GIO,  FALSE, FALSE},   //4  B 接收锁相环锁定标志 139
    {hetPORT1,  PIN_HET_15,      GIO,  FALSE, FALSE},   //5  B 发射锁相环锁定标志 41
    {hetPORT1,  PIN_HET_14,      GIO,  FALSE, FALSE},   //6  B XOSC READY标志 125
    {hetPORT1,  PIN_HET_12,      GIO,  FALSE, FALSE},   //7  B RXTX Lock标志 124
    {gioPORTA,           5,      GIO,  FALSE, FALSE},   //8  RESET
    {gioPORTB,           0,      GIO,  FALSE, FALSE},   //9  PA  126
};


SW_TYPE GetSwitchStatus(uint8 chn)
{
    SW_TYPE status = OFF;
    BaseType_t xRunningPrivileged = (BaseType_t)0;

    if(chn >= END_OF_SWITCH_NO) {
        return OFF;
    }

    if(sw[chn].privilege == TRUE) {
        xRunningPrivileged = prvRaisePrivilege();
    }

    switch(sw[chn].func) {
    case GIO:
        status = VoteSwitchStatus(gioGetBit(sw[chn].port, sw[chn].pin));
        break;
    case CAN:
        if(sw[chn].pin == 0) {
            status = VoteSwitchStatus(canIoRxGetBit((canBASE_t*)sw[chn].port));
        } else {
            status = VoteSwitchStatus(canIoTxGetBit((canBASE_t*)sw[chn].port));
        }
        break;
    case ADC:
        status = VoteSwitchStatus(adcGetEVTPin((adcBASE_t*)sw[chn].port));
        break;
    }


    if(sw[chn].privilege == TRUE) {
        portRESET_PRIVILEGE( xRunningPrivileged );
    }

    return status;
}

uint8 GetADCaptureValue(void)
{
	int i;
	adcData_t adc_data[MAX_AD_CHN];
	uint8 ret = TRUE;
	memset(adc_data, 0, sizeof(adcData_t) * MAX_AD_CHN);
    Delay(10);
    adcStartConversion(adcREG1, adcGROUP1);
    while (adcIsConversionComplete(adcREG1, adcGROUP1) != 8);
    adcGetData(adcREG1, adcGROUP1, adc_data);

	for (i = 0; i < MAX_AD_CHN; i++) {
		all_ad_data[i] = adc_data[i].value;
	}

	print("{ %.3f ", all_ad_data[0] * 3.3 / 4096.0 * 2);           //5VD_V_YC数字电源电压遥测   基带
    print(" %.3f ", all_ad_data[1] * 3.3 / 4096.0 /100 / 0.02);   //5VD_I_YC数字电源电流遥测
    print(" %.3f ", all_ad_data[2] * 3.3 / 4096.0 * 2);           //5VA_V_YC模拟电源电压遥测  射频
    print(" %.3f }", all_ad_data[3] * 3.3 / 4096.0 /100 / 0.02);   //5VA_I_YC模拟电源电流遥测

	return ret;
}

void SaveDataToFram(uint8* data, uint32 len, uint32 base, uint32 interval)
{
	uint16 crc_check;
	if(len > (interval - 2)) {
		/* one block size is bigger than interval */
		ErrorLog(__LINE__, (uint32)SaveDataToFram, len);
		return ;
	}
	crc_check = PacketComputerCRC(data, len, CRC_TYPE_CCITT);
	MR_Write_Data(base, data, len);
	MR_Write_Data(base + interval, data, len);

	MR_Write_Data(base + len, (uint8*)&crc_check, 2);
	MR_Write_Data(base + interval + len, (uint8*)&crc_check, 2);
}

uint8 RestoreDataFromFram(uint8* data, uint32 len, uint32 base, uint32 interval)
{
	uint16 crc_check;
	if(len > (interval - 2)) {
		/* one block size is bigger than interval */
		ErrorLog(__LINE__, (uint32)RestoreDataFromFram, len);
		return FALSE;
	}

	memset(data, 0, len);
	MR_Read_Data(base, data, len);
	MR_Read_Data(base + len, (uint8*)&crc_check, 2);

	if(PacketComputerCRC(data, len, CRC_TYPE_CCITT) != crc_check) {
		memset(data, 0, len);
		MR_Read_Data(base + interval, data, len);
		MR_Read_Data(base + interval + len, (uint8*)&crc_check, 2);
		if(PacketComputerCRC(data, len, CRC_TYPE_CCITT) == crc_check) {
			return TRUE;
		} else {
			return FALSE;
		}
	} else {
		return TRUE;
	}
}

void SendDataToEarth(void){
    uint8_t status_var = 0;

    lgw_status(TX_STATUS, &status_var);

    if(status_var != TX_FREE){
        return;
    }

    txpkt.freq_hz = dcsData.freq_hz;
    txpkt.rf_power = dcsData.rf_power;
    txpkt.bandwidth = dcsData.bandwidth;
    txpkt.datarate = dcsData.datarate;
    memcpy(txpkt.payload, dcsData.tx_data, 20);

    gioSetBit(gioPORTB, 0, 1);

    lgw_send(txpkt); /* non-blocking scheduling of TX packet */

    ADD(CNT_TRANSMIT_DATA);
    print("--TX-->>");
}
