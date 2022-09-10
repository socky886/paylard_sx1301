/*
 * collect.c
 *
 *  Created on: 2019��2��25��
 *      Author: ases-jack
 */

#include "collect.h"
#include "hw.h"
#include "can_data_process.h"
#include "glib.h"
#include "init.h"
#include "counter.h"
#include "can_lib.h"
#include "interrupt.h"
#include "inject.h"
#include "taskcode.h"
#include "print.h"
#include "systime.h"

uint8 local_tm[100];
uint8 local_tm_length = 0;

void MakeLocalTelemetry(void)
{
    int i = 0;
    uint32 pos = 0;
    local_tm[pos++] = PACK_SWITCH_TO_BYTE(0,1,2,3,4,5,6,7);
//    local_tm[pos++] = PACK_SWITCH_TO_BYTE(8,9,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);

    local_tm[pos++] = ResetType;
    memcpy(&local_tm[pos], AllCounter, LAST_COUNTER);
    pos += LAST_COUNTER;

    memcpy(&local_tm[pos], (uint8*)&inject, sizeof(INJECT_DATA));
    pos += sizeof(INJECT_DATA);

    memcpy(&local_tm[pos], all_ad_data, sizeof(all_ad_data));
    pos += sizeof(all_ad_data);

    SYS_TIME curr = GetSysTime();
    memcpy(&local_tm[pos], &curr.sec, 4);
    pos += sizeof(all_ad_data);

    local_tm_length = pos;

    print("{");

    for(i = 0; i < 11; i++){
        print (" %d", AllCounter[i + 7]);
    }
    print("}");
}

