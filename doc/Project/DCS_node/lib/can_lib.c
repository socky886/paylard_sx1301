/*
 * can_lib.c
 *
 *  Created on: 2017閿熸枻鎷�12閿熸枻鎷�21閿熸枻鎷�
 *      Author: ases-jack
 */
#include <string.h>
#include "can.h"
#include "can_lib.h"
#include "init.h"
#include "FreeRTOS.h"
#include "os_task.h"
#include "interrupt.h"
#include "sys_config.h"
#include "queue.h"
#include "counter.h"
#include "errorlog.h"
#include "hw.h"
#include "print.h"
#include "systime.h"
#include "can_cmd.h"
#include "can_data_process.h"


CAN_DATA g_Transmit;
CAN_DATA g_Receive;

uint8 recv_buff[220 + 5];

uint8 Is_FinishTransmit = FALSE;
uint8 Is_FinishReceive = FALSE;
uint8 Is_FinishTelecmd = FALSE;

uint8 tc_last_len;
uint8 pa_switch_status = 0;

void canMessageBoxInit(void)
{
    canUpdateID(canREG1, CAN_GET_TM_BOX, CAN_GET_TELEMETRY_ID << 18);        //Get Telemetry packet
    canUpdateID(canREG1, CAN_GET_CMD_BOX, CAN_SWITCH_CMD_ID << 18);          //Send cmd
    canUpdateID(canREG1, CAN_DATA_INJECT_BOX, CAN_TELEMETRY_INJECT_ID << 18);//Telemetry packet inject
    canUpdateID(canREG1, CAN_BUS_RESET_BOX, CAN_BUS_RESET_ID << 18);          //Bus reset



    canUpdateID(canREG2, CAN_GET_TM_BOX, CAN_GET_TELEMETRY_ID << 18);         //Get Telemetry packet
    canUpdateID(canREG2, CAN_GET_CMD_BOX, CAN_SWITCH_CMD_ID << 18);           //Send cmd
    canUpdateID(canREG2, CAN_DATA_INJECT_BOX, CAN_TELEMETRY_INJECT_ID << 18); //Telemetry packet inject
    canUpdateID(canREG2, CAN_BUS_RESET_BOX, CAN_BUS_RESET_ID << 18);          //Bus reset

}

void canTransmitData(canBASE_t *node, uint32 messageBox, uint8 node_id, uint8 frame_type, uint8 *data, uint32 len, uint8 data_type)
{
    int i;
    Is_FinishTransmit = FALSE;
    uint8 canBuf[8];
    memset(canBuf, 0, 8);
    if(len <= 8){
        canUpdateID(node, messageBox, 0x20000000 | (canGenerateID(node_id, frame_type, CAN_SINGLE_FRAME) << 18));
        canUpdateDlc(node, messageBox, len);
        memcpy(canBuf, data, len);
        canTransmit(node, messageBox, canBuf);
        while(canIsTxMessagePending(node, messageBox) != 0);
        Is_FinishTransmit = TRUE;
        return ;
    } else if(len > 1790) {
        ErrorLog(__LINE__, (uint32)canTransmitData, len);
        return ;
    } else {
        canUpdateID(node, messageBox, 0x20000000 | (canGenerateID(node_id, frame_type, CAN_FIRST_FRAME) << 18));
        canUpdateDlc(node, messageBox, 8);
        canBuf[0] = 1;
        canBuf[1] = (len - 5) / 7 + 1 + (((len - 5) % 7 > 0) ? 1 : 0);
        canBuf[2] = data_type;
        g_Transmit.frame_sum = canBuf[1];
        g_Transmit.frame_index = 1;
        memcpy(&canBuf[3], data, 5);
        g_Transmit.msgBoxId = messageBox;
        g_Transmit.last_cnt = len - 5;
        g_Transmit.data_ptr = data + 5;
        //g_Transmit.func = transmit;
        g_Transmit.node_id = node_id;
        g_Transmit.frame_type = frame_type;
        canTransmit(node, messageBox, canBuf);
        for(i = 1; i < g_Transmit.frame_sum; i++) {
            g_Transmit.frame_index++;
            canBuf[0] = g_Transmit.frame_index;
            if(i != (g_Transmit.frame_sum - 1)) {
                memcpy(&canBuf[1], g_Transmit.data_ptr, 7);
                g_Transmit.data_ptr += 7;
                g_Transmit.last_cnt -= 7;
                while(canIsTxMessagePending(node, messageBox) != 0);
                canUpdateID(node, messageBox, 0x20000000 | (canGenerateID(node_id, frame_type, CAN_MIDDLE_FRAME) << 18));
            } else {
                memcpy(&canBuf[1], g_Transmit.data_ptr, g_Transmit.last_cnt);
                while(canIsTxMessagePending(node, messageBox) != 0);
                canUpdateID(node, messageBox, 0x20000000 | (canGenerateID(node_id, frame_type, CAN_LAST_FRAME) << 18));
                canUpdateDlc(node, messageBox, g_Transmit.last_cnt + 1);
            }
            canTransmit(node, messageBox, canBuf);
        }
        memset((uint8*)&g_Transmit, 0, sizeof(g_Transmit));
        //while(canIsTxMessagePending(node, messageBox) != 0);
    }
}

uint8 receive(canBASE_t *node, uint32 messageBox)
{
    uint8 canBuf[8];
    uint32 msg_id;
    uint32 index;
    uint8 dlc;
    BaseType_t xHigherPriorityTaskWoken = (BaseType_t)0;


    memset(canBuf, 0, 8);
    canGetData(node, messageBox, canBuf);
    msg_id = canGetID(node, messageBox) >> 18;
    dlc = getCanDlc(node, messageBox);
    index = msg_id & 0x3;

    switch(index){
    case CAN_SINGLE_FRAME:
        if(g_Receive.data_ptr == NULL) {
            g_Receive.data_ptr = recv_buff;
        }
        memcpy(g_Receive.data_ptr, canBuf, dlc);
        g_Receive.last_cnt = dlc;
        Is_FinishReceive = TRUE;
        xSemaphoreGiveFromISR(xCanSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        return TRUE;
    case CAN_FIRST_FRAME:
        if(g_Receive.data_ptr == NULL) {
            g_Receive.data_ptr = recv_buff;
        }
        memcpy(g_Receive.data_ptr, canBuf + 3, 5);
        g_Receive.frame_sum = canBuf[1];
        if(1 != canBuf[0]){
            ErrorLog(__LINE__, (uint32)receive, canBuf[0]);
            return FALSE;
        }
        g_Receive.last_cnt = 5;
        break;
    case CAN_MIDDLE_FRAME:
        if(g_Receive.data_ptr == NULL) {
            ErrorLog(__LINE__, (uint32)receive, (uint32)g_Receive.data_ptr);
            return FALSE;
        }
        if(canBuf[0] > g_Receive.frame_sum){
            ErrorLog(__LINE__, (uint32)receive, canBuf[0]);
            return FALSE;
        }
        memcpy(g_Receive.data_ptr + (canBuf[0] - 2) * 7 + 5, canBuf + 1, 7);
        g_Receive.last_cnt += 7;
        break;
    case CAN_LAST_FRAME:
        if(g_Receive.data_ptr == NULL) {
            ErrorLog(__LINE__, (uint32)receive, (uint32)g_Receive.data_ptr);
            return FALSE;
        }
        if(canBuf[0] > g_Receive.frame_sum){
            ErrorLog(__LINE__, (uint32)receive, canBuf[0]);
            g_Receive.func = NULL;
            g_Receive.data_ptr = NULL;
            return FALSE;
        }
        memcpy(g_Receive.data_ptr + (canBuf[0] - 2) * 7 + 5, canBuf + 1, dlc - 1);
        g_Receive.last_cnt += dlc - 1;
        Is_FinishReceive = TRUE;
        g_Receive.func = NULL;
        xSemaphoreGiveFromISR(xCanSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        return TRUE;
    }
    return FALSE;
}

void canIRQ(canBASE_t *node, uint32 messageBox)
{
    uint8 canBuf[8];
    if(messageBox == CAN_GET_CMD_BOX) {
        /* CAN指令 */
        AddCanCmdToQueue(node, messageBox);
    } else if(messageBox == CAN_BUS_RESET_BOX) {
        /* CAN总线恢复 */
        canInit();
        canMessageBoxInit();
//        ADD(CNT_CAN_REINIT);
    } else if((messageBox == CAN_GET_TM_BOX) || (messageBox == CAN_DATA_INJECT_BOX) || (messageBox == CAN_GNC_BROADCAST_BOX)){
        /* 遥测    数据上注  GNC数据广播 */
        if(receive(node, messageBox) == TRUE) {
            CanDataProcess(node, &g_Receive);
        }
    } else {
        /* No empty the messagebox, will not generate another interrupt */
        /* So when the can message is late, read it and drop it */
        canGetData(node, messageBox, canBuf);
    }
}

void canUpdateDlc(canBASE_t *node, uint32 messageBox, uint8 dlc)
{
    while ((node->IF1STAT & 0x80U) ==0x80U)
    {
    } /* Wait */
    node->IF1MCTL &= (~0xF);
    node->IF1MCTL |= dlc;
    node->IF1CMD = 0x90;
    node->IF1NO = messageBox;
    while ((node->IF1STAT & 0x80U) ==0x80U)
    {
    } /* Wait */
}

uint8 getCanDlc(canBASE_t *node, uint32 messageBox)
{
    while ((node->IF1STAT & 0x80U) ==0x80U)
    {
    } /* Wait */
    node->IF1CMD = 0x10;
    node->IF1NO = messageBox;
    while ((node->IF1STAT & 0x80U) ==0x80U)
    {
    } /* Wait */
    return (node->IF2MCTL & 0xF);
}

uint16 canGenerateID(uint8 node_id, uint8 frame_type, uint8 frame_order)
{
    return ((frame_type << 7) | (node_id << 2) | (frame_order));
}
