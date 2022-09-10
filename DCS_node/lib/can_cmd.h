/*
 * can_cmd.h
 *
 *  Created on: 2018骞�7鏈�1鏃�
 *      Author: ases-jack
 */

#ifndef LIB_CAN_CMD_H_
#define LIB_CAN_CMD_H_

#include "types_Def.h"
#include "can.h"
#include "hw.h"


extern void InitCanCmdQueue(void);
extern void AddCanCmdToQueue(canBASE_t *node, uint32 messageBox);
extern void ProcessCanCmd(void);

#endif /* LIB_CAN_CMD_H_ */
