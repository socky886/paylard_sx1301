

/*
 * main.c
 *
 *
 *
 *
 *                             _oo0oo_
 *                            088888880
 *                            88" . "88
 *                            (| -_- |)
 *                             0\ = /0
 *                          ___/'---'\___
 *                       .' \\\\|     |// '.
 *                      / \\\\|||  :  |||// \\
 *                     /_ ||||| -:- |||||-   \\
 *                     |   | \\\\\\  -  /// | |
 *                     | \_|  ''\---/''  |_/  |
 *                     \  .-\__  '-'  __/-.  /
 *                   ___'. .'  /--.--\  '. .'___
 *                ."" '<  '.___\_<|>_/___.' >'  "".
 *               | | : '-  \'.;'\ _ /';.'/ - ' : | |
 *               \  \ '_.   \_ __\ /__ _/   .-' /  /
 *           ====='-.____'.___ \_____/___.-'____.-'=====
 *                             '=---='
 *        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *
 *
 */

#include "sys_core.h"
#include "sys_config.h"
#include "init.h"
#include "taskcode.h"
#include "print.h"
#include "counter.h"
#include "os_task.h"
#include "glib.h"

#define SN " DCS_node "


void main(void)
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       {
    InitHw();
    FeedWatchDog();
    print_logo();
    FeedWatchDog();
    print("\nSatellite platform system:%s\n", SN);
    print("Designed by ASES\n");
    print("Code by Jack Lee\n");
    print("Version:%s@FreeRTOS_%s\n", MAIN_VERSION, tskKERNEL_VERSION_NUMBER);
    print("Init hardware...");
    print("Done\n");

    print("Init buffer...");
    InitBuf();

    print("Done\n");
    print("CPU has RESET %d time%c \nLast reset is %s\n", CNT(CNT_SOFTWARE_START), (CNT(CNT_SOFTWARE_START) > 1)? 's':' ', GetResetTypeName());

    print("Init Semaphore...");
    InitSemaphore();
    print("Done\n");

    print("Enable interrupt\n");
    _enable_interrupt_();

    print("Init task...");
    InitTask();
    print("Done\n");

    AddSoftwareStartCounter();

    vTaskStartScheduler();
    while(TRUE);
}
