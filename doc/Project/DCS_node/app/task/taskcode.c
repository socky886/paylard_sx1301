/*
 * taskcode.c
 *
 *  Created on: 2018楠烇拷1閺堬拷16閺冿拷
 *      Author: ases-jack
 */
#include "taskcode.h"
#include "FreeRTOS.h"
#include "os_task.h"
#include "types_def.h"
#include "interrupt.h"
#include "hw.h"
#include "can_lib.h"
#include "print.h"
#include "systime.h"
#include "counter.h"
#include "can_lib.h"
#include "errorlog.h"
#include "glib.h"
#include "can_cmd.h"
#include "hw.h"
#include "queue.h"
#include "randomizer.h"
#include "fec.h"
#include "can_data_process.h"
#include "can_lib.h"
#include "collect.h"
#include "inject.h"
#include "crc.h"
#include "os_mpu_wrappers.h"
#include "sx1301_lora.h"
#include "sx1255.h"
#include "sx1301_config.h"
#include "SN65LVDS31PW.h"

/* Structure that will hold the TCB of the task being created. */
StaticTask_t xMainTaskBuffer;
/* Buffer that the task being created will use as its stack. Note this is an array of StackType_t variables. The size of StackType_t is dependent on the RTOS port. */
StackType_t xMainTaskStack[ MAIN_TASK_STACK_SIZE + 0x10];    /* 0x10 is gap betwwen task stack to another global var */

/* Structure that will hold the TCB of the task being created. */
StaticTask_t xIdleTaskBuffer;
/* Buffer that the task being created will use as its stack. Note this is an array of StackType_t variables. The size of StackType_t is dependent on the RTOS port. */
StackType_t xIdleTaskStack[ IDLE_TASK_STACK_SIZE ];


const TaskParameters_t task_params[MAX_TASK_NUM] = {
	{
		vMainTask,            /* Function that implements the task. */
		"MainTask",           /* Text name for the task. */
		MAIN_TASK_STACK_SIZE, /* The number of indexes in the xStack array. */
		NULL,                 /* Parameter passed into the task. */
        MAIN_TASK_PRIORITY,   /* Priority at which the task is created. */
		xMainTaskStack,       /* Array to use as the task's stack. */
		/* No region is set in this example. */
		{
			/* Base address  Length  Parameters */
			{0,              0,      0},
			{0,              0,      0}
        }
	},
	{
		NULL,            /* Function that implements the task. */
		"IdleTask",           /* Text name for the task. */
		IDLE_TASK_STACK_SIZE, /* The number of indexes in the xStack array. */
		NULL,                 /* Parameter passed into the task. */
		IDLE_TASK_PRIORITY,   /* Priority at which the task is created. */
		xIdleTaskStack,       /* Array to use as the task's stack. */
		/* No region is set in this example. */
		{
			/* Base address  Length  Parameters */
			{0,              0,      0},
			{0,              0,      0}
        }
	}
};

/*
 * 获得微妙数，最多4294967295us
 */
uint32
get_clock_cpu (void)
{
    TickType_t current;
    if(xTaskGetCurrentTaskHandle() != NULL) {
        current = xTaskGetTickCount();
    } else {
        current = xTaskGetTickCountFromISR();
    }
    return ((current * 1000) & 0xFFFFFFFF);
}


TaskHandle_t task_handle[MAX_TASK_NUM];


struct lgw_pkt_rx_s rxpkt[8]; /* array containing up to 4 inbound packets metadata */

void vMainTask( void * pvParameters )
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(TASKTIME);
    uint32 start, end, interval;
    int nb_pkt;
    struct lgw_pkt_rx_s *p; /* pointer on a RX packet */

	uint32 TimeDivisionFlag = 1;
	SYS_TIME current_time = {0, 0};
    int i;

    sciReceive(sciREG, 14+20, (uint8 *)&dcsData);

	/* Enter the loop that defines the task behavior. */
	for( ;; )
	{
        start = get_clock_cpu();
		current_time = GetSysTime();
		print("%8d.%.3d:", current_time.sec, current_time.ms);

        FeedWatchDog();

        nb_pkt = lgw_receive(sizeof(rxpkt) / sizeof((rxpkt)[0]), rxpkt);
        if(nb_pkt != 0){
            /* display received packets */
            for(i=0; i < nb_pkt; ++i) {
                p = &rxpkt[i];
                print("---\nRcv pkt #%d >>", i+1);
                if(p->if_chain < 9){
                    ADD(CNT_IF_CHAIN_0 + p->if_chain);
                }
                if (p->status == STAT_CRC_OK) {
                    ADD(CNT_CORRECT_RECEIVE);
                    print(" if_chain:%2d", p->if_chain);
                    print(" tstamp:%010u", p->count_us);
                    print(" size:%3u", p->size);

                    print("\n");
                    print(" RSSI:%+6.1f SNR:%+5.1f (min:%+5.1f, max:%+5.1f) payload:\n", p->rssi, p->snr, p->snr_min, p->snr_max);
                    print_mem(p->payload, p->size, 10);

                    switch (p-> modulation) {
                        case MOD_LORA: print(" LoRa"); break;
                        case MOD_FSK: print(" FSK"); break;
                        default: print(" modulation?");
                    }
                    switch (p->datarate) {
                        case DR_LORA_SF7: print(" SF7"); break;
                        case DR_LORA_SF8: print(" SF8"); break;
                        case DR_LORA_SF9: print(" SF9"); break;
                        case DR_LORA_SF10: print(" SF10"); break;
                        case DR_LORA_SF11: print(" SF11"); break;
                        case DR_LORA_SF12: print(" SF12"); break;
                        default: print(" datarate?");
                    }
                    switch (p->coderate) {
                        case CR_LORA_4_5: print(" CR1(4/5)"); break;
                        case CR_LORA_4_6: print(" CR2(2/3)"); break;
                        case CR_LORA_4_7: print(" CR3(4/7)"); break;
                        case CR_LORA_4_8: print(" CR4(1/2)"); break;
                        default: print(" coderate?");
                    }
                    print(" #\n");
                } else{
                    ADD(CNT_WRONG_RECEIVE);
                }
            }
        }


        if(uart_receive_flag == TRUE){
            SendDataToEarth();
            uart_receive_flag = FALSE;
            sciReceive(sciREG, 14+20, (uint8 *)&dcsData);
        }

        print ("{RXR:%d,RXW:%d,TX:%d}",AllCounter[CNT_CORRECT_RECEIVE], AllCounter[CNT_WRONG_RECEIVE], AllCounter[CNT_TRANSMIT_DATA]);

        end = get_clock_cpu();
        if (end < start) {
            interval = 0x0 - start + end;
        } else {
            interval = end - start;
        }
        if (interval > (180*1000)) {
            print("<%d>", interval/1000);
            ADD(CNT_TASK_TIME_OUT);
        } else {
            print("$%d", interval/1000);
        }
        print("\n");
		TimeDivisionFlag++;
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
	}
}

void InitTask(void) {
	int i = 0;

	for (i = 0; i < (MAX_TASK_NUM - 1); i++) {
		task_handle[i] = xTaskCreateStatic(task_params[i].pvTaskCode,
				                           task_params[i].pcName,
										   task_params[i].usStackDepth,
										   task_params[i].pvParameters,
										   task_params[i].uxPriority,
										   task_params[i].puxStackBuffer,
										   &xMainTaskBuffer);
		if( task_handle[i] == NULL ) {
			ErrorLog(__LINE__, (uint32)InitTask, (uint32)i);
		}
	}
}
