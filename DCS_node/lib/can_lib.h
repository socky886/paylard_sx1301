/*
 * can_lib.h
 *
 *  Created on: 2017閿熸枻鎷�12閿熸枻鎷�21閿熸枻鎷�
 *      Author: ases-jack
 */

#ifndef LIB_CAN_LIB_H_
#define LIB_CAN_LIB_H_

#include "can.h"
#include "sys_config.h"

typedef struct{
    uint16 len;
    uint8 data[128];
}TeleCmd;

#define CAN_GET_TM_BOX          canMESSAGE_BOX1
#define CAN_GET_CMD_BOX         canMESSAGE_BOX2
#define CAN_DATA_INJECT_BOX     canMESSAGE_BOX3
#define CAN_BUS_RESET_BOX       canMESSAGE_BOX4
#define CAN_TIME_BROADCAST_BOX  canMESSAGE_BOX5
#define CAN_GNC_BROADCAST_BOX   canMESSAGE_BOX6
#define TRANS_MSG_BOX           canMESSAGE_BOX7

/*
 *  Msg Id define
 */
#define CAN_TIME_BROADCAST_ID      0x480
#define CAN_GNC_BROADCAST_ID       0x400

#if !DCS_CPU
#define CAN_GET_TELEMETRY_ID       0x22C
#define CAN_SWITCH_CMD_ID          0x12C
#define CAN_TELEMETRY_INJECT_ID    0x1AC
#define CAN_BUS_RESET_ID           0xAC

#else
#define CAN_GET_TELEMETRY_ID       0x230
#define CAN_SWITCH_CMD_ID          0x130
#define CAN_TELEMETRY_INJECT_ID    0x1B0
#define CAN_BUS_RESET_ID           0xB0

#endif

/*
 * Frame order
 */
#define CAN_SINGLE_FRAME  0     /* 单帧  */
#define CAN_FIRST_FRAME   1     /* 首帧  */
#define CAN_MIDDLE_FRAME  2     /* 中间帧  */
#define CAN_LAST_FRAME    3     /* 尾帧 */

/*
 * Frame type
 */
#define CAN_BUS_RECOVERY      1     /* 总线恢复 */
#define CAN_SEND_CMD          2     /* 传送指令 */
#define CAN_SEND_DATA         3     /* 数据注入 */
#define CAN_GET_TM_DATA       4     /* 遥测数据 */
#define CAN_GNC_BROADCAST     5     /* GNC广播 */
#define CAN_TIME_BROADCAST    6     /* 时间广播 */


/*
 * type define
 */
typedef uint8(*CAN_FUNC)(canBASE_t *node, uint32 messageBox);

typedef struct {
    uint32    msgBoxId;
    uint8     node_id;
    uint8     frame_type;
    CAN_FUNC  func;
    uint32    last_cnt;
    uint8*    data_ptr;
    uint8     frame_sum; //总帧数
    uint8     frame_index;
    uint16    msg_id;
}CAN_DATA;

/*
 * Var
 */
/*
 * Var
 */
extern CAN_DATA g_Transmit;
extern CAN_DATA g_Receive;

extern uint8 Is_FinishTransmit;
extern uint8 Is_FinishReceive;

extern uint8 pa_switch_status;
/*
 * Function
 */
extern void canMessageBoxInit(void);
extern uint8 receive(canBASE_t *node, uint32 messageBox);
extern void canUpdateDlc(canBASE_t *node, uint32 messageBox, uint8 dlc);
extern uint8 getCanDlc(canBASE_t *node, uint32 messageBox);
extern uint16 canGenerateID(uint8 node_id, uint8 frame_type, uint8 frame_order);
/*
 * Send Can data
 * len is not include data type
 * data type is last argument
 */
extern void canTransmitData(canBASE_t *node, uint32 messageBox, uint8 node_id, uint8 frame_type, uint8 *data, uint32 len, uint8 data_type);


extern void canIRQ(canBASE_t *node, uint32 messageBox);


#endif /* LIB_CAN_LIB_H_ */
