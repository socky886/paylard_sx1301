/*
 * can_cmd.c
 *
 *  Created on: 2018骞�7鏈�1鏃�
 *      Author: ases-jack
 */
#include "can_lib.h"
#include "can_cmd.h"
#include "queue.h"
#include "systime.h"
#include "print.h"
#include "hw.h"
#include "inject.h"
#include "sx1301_lora.h"
#include "counter.h"

TypeQueue can_cmd;
uint8 can_cmd_element[3 * 10];


void InitCanCmdQueue(void)
{
	InitQueue(&can_cmd, can_cmd_element, 3, 10);
}


void AddCanCmdToQueue(canBASE_t *node, uint32 messageBox)
{
	uint8 canBuf[3];

	memset(canBuf, 0, 3);
	canGetData(node, messageBox, canBuf);
	if(canBuf[0] == MY_NODE_ID) {
		PushQueueEx(canBuf, &can_cmd);
		canBuf[0] = 0x00;
	} else {
	    canBuf[0] = 0xFF;
	}
	canTransmitData(node, TRANS_MSG_BOX, MY_NODE_ID, CAN_SEND_CMD, canBuf, 1, 0);
}

void ProcessCanCmd(void)
{
	uint8 canBuf[3];
	if(PopQueueEx(&can_cmd, canBuf) == FALSE) {
		return ;
	}
	if(canBuf[1] != canBuf[2]){
	    return;
	}
    //TODO
	switch(canBuf[1]){
	case 1:
	    //基带板复位
	    sx1301init();
	    ADD(CNT_SX1301_REINIT);
	    break;
	case 2:
	    //发射通道开启
        inject.TxSendEnable = ON;
	    break;
    case 3:
        //发射通道关闭
        inject.TxSendEnable = OFF;
        break;
	case 4:
	    //上行计数清零
	    memset(AllCounter + CNT_CORRECT_RECEIVE, 0, LAST_COUNTER - CNT_CORRECT_RECEIVE);
	    break;
	case 5:
	    //下行带宽500KHz
	    inject.tx_bandwidth = BW_500KHZ;
	    break;
    case 6:
        //下行带宽250KHz
        inject.tx_bandwidth = BW_250KHZ;
        break;
    case 7:
        //下行速率切换400bps SF12
        inject.tx_datarate = DR_LORA_SF12;
        break;
    case 8:
        //下行速率切换800bps  SF11
        inject.tx_datarate = DR_LORA_SF11;
        break;
    case 9:
        //下行速率切换1200bps  SF10
        inject.tx_datarate = DR_LORA_SF10;
        break;
	default:
	    break;
	}

}

