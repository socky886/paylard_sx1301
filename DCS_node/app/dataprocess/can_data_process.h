/*
 * can_data_process.h
 *
 *  Created on: 2018��12��19��
 *      Author: ases-jack
 */

#ifndef APP_DATAPROCESS_CAN_DATA_PROCESS_H_
#define APP_DATAPROCESS_CAN_DATA_PROCESS_H_

#include "can_lib.h"
#include "queue.h"

#define GNC_BROADCAST_LEN 44
#define INJECT_DATA_LEN   64


extern TypeQueue InjectDataQueue;

extern uint8 TxDataGnc[GNC_BROADCAST_LEN];
extern uint8 TxDataInject[INJECT_DATA_LEN - 2];

extern uint8 haveInjectDataToSend;

#pragma pack(push, 1)
typedef struct {
    uint32 offset;
    uint8 len;
    uint8 data[INJECT_DATA_LEN - 7];
}INJECT_PACK;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint16         head;
    INJECT_PACK   pack;
}TC_FRAME;
#pragma pack(pop)

extern void CanInitQueue(void);
extern void CanDataProcess(canBASE_t *node, CAN_DATA* pCanData);
extern void InjectDataProcess(void);


#endif /* APP_DATAPROCESS_CAN_DATA_PROCESS_H_ */
