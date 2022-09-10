/*
 * hw.h
 *
 *  Created on: 2018骞�1鏈�21鏃�
 *      Author: ases-jack
 */

#ifndef APP_HW_H_
#define APP_HW_H_
#include "gio.h"
#include "het.h"
#include "spi.h"
#include "can.h"
#include "adc.h"

#include "sys_config.h"
#include "FreeRTOS.h"
#include "os_task.h"


/*
 * Value define
 */
#define A_RX_PLL_LOCK_GLAG         0
#define A_TX_PLL_LOCK_GLAG         1
#define A_XOSC_READY_GLAG          2
#define A_RX_TX_LOCK_GLAG          3
#define B_RX_PLL_LOCK_GLAG         4
#define B_TX_PLL_LOCK_GLAG         5
#define B_XOSC_READY_GLAG          6
#define B_RX_TX_LOCK_GLAG          7
#define BB_RESET_SWITCH            8
#define PA_EN_SWITCH               9
#define END_OF_SWITCH_NO          10
#define EMPTY_SWITCH            0xFF


typedef enum {ON = 0x00U, OFF = 0xFFU}  SW_TYPE;
typedef enum {CAN, GIO, ADC} IOFUNC;

typedef struct _SwInfo {
    gioPORT_t  *port;
    uint8  pin;
    IOFUNC func;
    uint8  privilege;
    uint8  isOC;
} SwitchInfoType;


extern uint16 all_ad_data[MAX_AD_CHN];

#define VoteSwitchStatus(x)  ({  \
    uint8 a = x;   \
    uint8 b = x;   \
    uint8 c = x;   \
    uint8 ret = 0;  \
	if(a == b) {   \
		ret = a;  \
	} else if (b == c) {  \
		ret = b;  \
	} else if (a == c) {  \
		ret = a;  \
	} else {  \
		ret = 0;  \
	}  \
	((ret == TRUE) ? ON:OFF);  \
})
#pragma pack(push, 1)
typedef struct {
    uint16_t    head;
    uint16_t    len;
    uint32_t    freq_hz;
    int8_t      rf_power;
    uint8_t     bandwidth;
    uint32_t    datarate;
    uint8       tx_data[256];
}DCS_DATA;
#pragma pack(pop)

extern DCS_DATA dcsData;

extern uint8 GetSwitchStatus(uint8 chn);

extern uint8 GetADCaptureValue(void);

extern void SaveDataToFram(uint8* data, uint32 len, uint32 base, uint32 interval);

extern uint8 RestoreDataFromFram(uint8* data, uint32 len, uint32 base, uint32 interval);

extern void SendDataToEarth(void);

#endif /* APP_HW_H_ */
