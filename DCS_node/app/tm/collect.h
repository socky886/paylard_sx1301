/*
 * collect.h
 *
 *  Created on: 2019��2��25��
 *      Author: ases-jack
 */

#ifndef APP_TM_COLLECT_H_
#define APP_TM_COLLECT_H_

#include "can.h"
#include "sys_config.h"


#define PACK_SWITCH_TO_BYTE(a, b, c, d, e, f, g, h)    \
    ((GetSwitchStatus(a)==ON) << 7) | ((GetSwitchStatus(b)==ON) << 6) | ((GetSwitchStatus(c)==ON) << 5) | \
    ((GetSwitchStatus(d)==ON) << 4) | ((GetSwitchStatus(e)==ON) << 3) | ((GetSwitchStatus(f)==ON) << 2) | \
    ((GetSwitchStatus(g)==ON) << 1) | ((GetSwitchStatus(h)==ON) << 0)

extern uint8 local_tm[100];
extern uint8 local_tm_length;

extern void MakeLocalTelemetry(void);

#endif /* APP_TM_COLLECT_H_ */
