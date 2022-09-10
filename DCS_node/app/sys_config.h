/*
 * sys_config.h
 *
 *  Created on: 2018闁跨喐鏋婚幏锟�1闁跨喐鏋婚幏锟�14闁跨喐鏋婚幏锟�
 *      Author: ases-jack
 */

#ifndef SYS_CONFIG_H_
#define SYS_CONFIG_H_

#include "types_def.h"

#define MAIN_VERSION   "2.0.0"

#define DCS_CPU      0    //0 burn in Module 0,385M, 1 burn in Module 1,401M

#define SIMPLE_MODE  0x55
#define NORMAL_MODE  0xAA


#define ENABLE_WATCHDOG
//#undef ENABLE_WATCHDOG

/*
 * Reset Type
 */
#define EXTERNAL_RESET       0x0008U
#define RESET_POWERON        0x11U
#define RESET_OSC_FAILURE    0x22U
#define RESET_WATCHDOG       0x44U
#define RESET_ICEPICK        0x88U
#define RESET_CPU            0x99U
#define RESET_SW             0xAAU
#define RESET_EXT            0x55U
#define RESET_UNKNOWN        0x00U

/*
 * Keep some ram ragion unclear at startup
 */
//#undef POR_CLEAR_ENTIRE_RAM
#define POR_CLEAR_ENTIRE_RAM

#define FLASH_BASE_ADDR   0x00000000U
#define END_OF_FLASH      0x00140000U
#define IS_LEGAL_FLASH_ADDR(x)  (x < END_OF_FLASH)
#define NOT_LEGAL_FLASH_ADDR(x) (!IS_LEGAL_FLASH_ADDR(x))

#define RAM_BASE_ADDR     0x08000000U
#define END_OF_USER_RAM   0x0802F000U
#define IS_LEGAL_RAM_ADDR(x)  ((x < END_OF_USER_RAM) && (x >= RAM_BASE_ADDR))
#define NOT_LEGAL_RAM_ADDR(x) (!IS_LEGAL_RAM_ADDR(x))

#define ECC_RAM_BASE_ADDR     0x08400000U
#define END_OF_USER_ECC_RAM   0x0842F000U
/*
 * CAN node id
 */
#define CAN_BOARDCAST_ID  0x0
#define CAN_EPS_NODE_ID   0x1

#if !DCS_CPU
#define MY_NODE_ID        11
#else
#define MY_NODE_ID        12
#endif

/*
 * CAN cmd code
 */
#define CAN_GET_TM_CMD   0x1A
#define CAN_DATA_TYPE    0x1A

/*
 * APID define
 */
#define APID_SAT_MAIN    0x401	  //Satellite important telemetry
#define APID_IES_OBC     0x402    //Satellite platform OBC telemetry
#define APID_IES_SW      0x403    //OBC software telemetry
#define APID_IES_GNC     0x404    //GNC telemetry
#define APID_IMU_A       0x406    //IMU A register content
#define APID_IES_TC_A    0x407
#define APID_IES_TC_B    0x408
#define APID_CARRY_ONE   0x409	//Carry on payload data
#define APID_CARRY_TWO   0x40A	//Carry on payload data
#define APID_CARRY_THREE 0x40B	//Carry on payload data
#define APID_CARRY_FOUR  0x40C	//Carry on payload data
#define APID_CARRY_FIVE  0x40D  //Carry on payload data
#define APID_CARRY_SIX   0x40E
#define APID_GNSS_DATA   0x405    //GNSS data

#define APID_YH_TM       APID_CARRY_ONE
#define APID_YH_PHOTO    APID_CARRY_TWO
#define APID_WXT_TM      APID_CARRY_THREE
#define APID_ROUTE_A     APID_CARRY_FOUR
#define APID_ROUTE_B     APID_CARRY_FIVE
#define APID_SPC_TELEMETRY         APID_CARRY_SIX


#define APID_DELAY_SYS_TM   0x501    //Delay System telemetry
#define APID_DELAY_GNC_TM   0x502    //Delay GNC telemetry

#define APID_SPC_WORK_DATA     0x503
#define APID_CAMERA_PHOTO      0x504
#define APID_ROUTEA_WORK_DATA  0x505
#define APID_ROUTEB_WORK_DATA  0x506
#define APID_DCS_PACKET_DATA   0x507
#define APID_FILE_CONTENT      0x508
#define APID_DIR_CONTENT       0x509

#define APID_ERROR_LOG 0x601    //Error log

#define APID_SIMPLE_TC_TM    0x415



/*
 * VC ID
 */
#define VC_ONE    1   // Telemetry
#define VC_TWO    2   // RAM data
#define VC_THREE  3   // Error log data
#define VC_FOUR   4   // Old Telemetry
#define VC_FIVE   5   // Galaxy data
#define VC_SIX    6   // SPC computed data
#define VC_SEVEN  7   // Photo data
#define VC_EIGHT  8   // ROute data
#define VC_NINE   9   // Dcs Packet
#define VC_TEN    10
#define VC_ELEVEN 11
#define VC_TWELVE 12
#define VC_MAX    13

#define VC_TM     VC_ONE
#define VC_RAM    VC_TWO
#define VC_ERRLOG VC_THREE
#define VC_YY     VC_FOUR
#define VC_GALAXY VC_FIVE
#define VC_SPC    VC_SIX
#define VC_PHOTO  VC_SEVEN
#define VC_ROUTE  VC_EIGHT
#define VC_DCS    VC_NINE
#define VC_FILE   VC_TEN
#define VC_DIR_LS VC_ELEVEN
#define VC_FAKE   VC_TWELVE

#define TELEMETRY_DATA_RANDOMIZE     0
#define VC_OBC_TELECMD   VC_ONE
#define VC_EPS_TELECMD   VC_TWO


#define MAX_MPDU_DATA 205
/*
 * Satellite ID
 */
#define SATELLITE_ID   0x53   //

#define X  0
#define Y  1
#define Z  2

#define MAKEWORD(h, l)   ((((h) & 0xFF) << 8) | ((l) & 0xFF))
#define MAKEDWORD(h, l)   ((((h) & 0xFFFF) << 16) | ((l) & 0xFFFF))

/*
 * FRAM RAM partition define
 */
#define COUNTER_DATA_PART_BASE       0x00000U
#define COUNTER_DATA_PART_INTERVAL   0x00080U
#define EXCEPT_CONTEXT_PART_BASE     0x00100U
#define EXCEPT_CONTEXT_PART_INTERVAL 0x00080U
#define INJECT_DATA_PART_BASE        0x00200U
#define INJECT_DATA_PART_INTERVAL    0x00300U
#define TIME_DATA_PART_BASE          0x00800U
#define TIME_DATA_PART_INTERVAL      0x00200U
#define FAST_REBOOT_INFO_BASE        0x00C00U
#define SWITCH_STATUS_BASE           0x01000U
#define SWITCH_STATUS_INTERVAL       0x00010U

//#define DELAY_TM_PTR_ADDR            0x01000U
//#define DELAY_TM_INFO_INTERVAL       0x00010U
//#define DELAY_TM_PART_BASE           (DELAY_TM_PTR_ADDR + DELAY_TM_INFO_INTERVAL * 2)

/*
 * reboot in 5 times
 */
#define MAX_QUICK_REBOOT_TIMES       5

/*
 * RAM Download
 */
#define MAX_RAM_DOWNLOAD_SIZE   (198 * 4)
/*
 * Delay tm max send block num per two sec
 */
#define MAX_SEND_BLOCK_NUM   1

/*
 * AD & Switch channel counter
 * tms570 - 23
 * mux - 16
 * ad1 - 16
 * ad2 - 16
 * tms5702 - 5
 * mux2 - 2
 * acg - 32
 */
#define MAX_AD_CHN       (4)

/*
 * GNC simulation
 */
//#define SIMULATION_GNC
#undef SIMULATION_GNC
#define GNSS_OPEN_INTERVAL     (12 * 3600)
#define GNSS_OPENING_TIME      (60 * 5)

#endif /* SYS_CONFIG_H_ */
