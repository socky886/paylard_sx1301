/*
 * inject.h
 *
 *  Created on: 2018骞�2鏈�2鏃�
 *      Author: ases-jack
 */

#ifndef APP_TM_INJECT_H_
#define APP_TM_INJECT_H_

#include "types_def.h"
#include "hw.h"

/*
 * Payload work mode
 */
#define PAYLOAD_FAKE_WORK_MODE     0x11
#define PAYLOAD_HISTORY_WORK_MODE  0x22
#define PAYLOAD_REAL_WORK_MODE     0x33

typedef enum {MAIN = 0xA5,  BACK = 0x5A,  ALL = 0xAA, COLD = 0x00} BURN_TYPE;

#pragma pack(push, 1)
typedef struct {
    uint32   tx_freq;                 /*!< 0x00   */
    uint8    tx_power;                /*!< 0x04   */
    uint8    tx_datarate;             /*!< 0x05   */
    uint8    tx_bandwidth;            /*!< 0x06   */
    SW_TYPE  TxSendEnable;            /*!< 0x07 下行发射是否开启   */
    uint8    TxSendInterval;          /*!< 0x08  Telemetry send interval */
}INJECT_DATA;
#pragma pack(pop)

extern INJECT_DATA inject;

#define INJECT_DATA_BASE_ADDR  ((uint32)(&inject))
#define INJECT_DATA_BASE_SIZE  (sizeof(inject))
#define INJECT_DATA_LAST_ADDR  ((uint32)&inject + sizeof(inject))



extern void InitInjectData(void);

extern void RestoreInjectDataFromFram(void);

extern void SaveInjectDataToFram(void);

#endif /* APP_TM_INJECT_H_ */
