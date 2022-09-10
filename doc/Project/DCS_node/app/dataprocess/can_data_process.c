/*
 * can_data_process.c
 *
 *  Created on: 2018��12��19��
 *      Author: ases-jack
 */

#include "can_data_process.h"
#include "can_lib.h"
#include "sys_config.h"
#include "string.h"
#include "collect.h"
#include "hw.h"
#include "counter.h"
#include "print.h"
#include "sx1301_lora.h"
#include "inject.h"
#include "systime.h"

TypeQueue InjectDataQueue;
uint8 InjectDataQueueElement[5 * INJECT_DATA_LEN];

TypeQueueElem *tc;


uint8 TxDataGnc[GNC_BROADCAST_LEN];

uint8 TxDataInject[INJECT_DATA_LEN - 2];

uint8 haveInjectDataToSend = FALSE;

void CanInitQueue(void)
{
    InitQueue(&InjectDataQueue, InjectDataQueueElement, INJECT_DATA_LEN, 5);
}
//TODO
void CanDataProcess(canBASE_t *node, CAN_DATA* pCanData)
{
    uint8 data[3];
    uint32 time;
    if(pCanData->last_cnt == 1) {
        //遥测
        if(pCanData->data_ptr[0] == CAN_GET_TM_CMD) {
            canTransmitData(node, TRANS_MSG_BOX, MY_NODE_ID, CAN_GET_TM_DATA, local_tm, local_tm_length, CAN_GET_TM_CMD);
        }
    }else if(pCanData->last_cnt == GNC_BROADCAST_LEN) {
        //GNC数据广播
        memcpy(TxDataGnc, pCanData->data_ptr, GNC_BROADCAST_LEN);

        time = (TxDataGnc[0] << 24) | (TxDataGnc[1] << 16) | (TxDataGnc[2] << 8) | TxDataGnc[3];

        SetTimeFromGncBroadcast(time);

        ADD(CNT_GNC_BROADCAST);

    } else if(pCanData->last_cnt == INJECT_DATA_LEN) {
        //数据上注
        data[0] = 0x00;
        canTransmitData(node, TRANS_MSG_BOX, MY_NODE_ID, CAN_SEND_DATA, data, 1, 0);
        PushQueueEx(pCanData->data_ptr, &InjectDataQueue);
        memset((uint8*)&g_Receive, 0, sizeof(g_Receive));
        ADD(CNT_INJECT_DATA);

    }

}

static uint8 g_fr[INJECT_DATA_LEN];
static TC_FRAME* gpfr = (TC_FRAME*)g_fr;

void InjectDataProcess(void){
    if(PopQueueEx(&InjectDataQueue, gpfr) == TRUE) {
        print("I");
        //can上注数据的解析
        if(gpfr->head == 0x3A11){
            //自用
            uint8 data_len = gpfr->pack.len;
            uint32 data_offset = gpfr->pack.offset;

            if((data_len > 15) || ((data_offset + data_len) > sizeof(inject))) {
                return ;
            }

            memcpy(&inject + data_offset, gpfr->pack.data, data_len);

        }else if(gpfr->head == 0x4A11){
            //下行发射用 特别数据
            memcpy(TxDataInject, &gpfr->pack, sizeof(TxDataInject));
            haveInjectDataToSend = TRUE;


        }

    }

}

