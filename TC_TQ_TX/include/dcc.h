/** @file dcc.h
*   @brief DCC Driver Definition File
*   @date 07-July-2017
*   @version 04.07.00
*   
*/

/* 
* Copyright (C) 2009-2016 Texas Instruments Incorporated - www.ti.com  
* 
* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


#ifndef __DCC_H__
#define __DCC_H__

#include "reg_dcc.h"
#ifdef __cplusplus
extern "C" {
#endif
/* USER CODE BEGIN (0) */
/* USER CODE END */

/* DCC General Definitions */

/** @def dcc1CNT0_CLKSRC_HFLPO
*   @brief Alias name for DCC1 Counter 0 Clock Source HFLPO
*
*   This is an alias name for the Clock Source HFLPO for DCC1 Counter 0.
*
*   @note This value should be used for API argument @a cnt0_Clock_Source
*/
#define dcc1CNT0_CLKSRC_HFLPO 0x00000005U

/** @def dcc1CNT0_CLKSRC_TCK
*   @brief Alias name for DCC1 Counter 0 Clock Source TCK
*
*   This is an alias name for the Clock Source TCK for DCC1 Counter 0.
*
*   @note This value should be used for API argument @a cnt0_Clock_Source
*/
#define dcc1CNT0_CLKSRC_TCK 0x0000000AU

/** @def dcc1CNT0_CLKSRC_OSCIN
*   @brief Alias name for DCC1 Counter 0 Clock Source OSCIN
*
*   This is an alias name for the Clock Source OSCIN for DCC1 Counter 0.
*
*   @note This value should be used for API argument @a cnt0_Clock_Source
*/
#define dcc1CNT0_CLKSRC_OSCIN 0x0000000FU

/** @def dcc1CNT1_CLKSRC_PLL1
*   @brief Alias name for DCC1 Counter 1 Clock Source PLL1
*
*   This is an alias name for the Clock Source PLL for DCC1 Counter 1.
*
*   @note This value should be used for API argument @a cnt1_Clock_Source
*/
#define dcc1CNT1_CLKSRC_PLL1 0x0000A000U

/** @def dcc1CNT1_CLKSRC_LFLPO
*   @brief Alias name for DCC1 Counter 1 Clock Source LFLPO
*
*   This is an alias name for the Clock Source LFLPO for DCC1 Counter 1.
*
*   @note This value should be used for API argument @a cnt1_Clock_Source
*/
#define dcc1CNT1_CLKSRC_LFLPO 0x0000A002U

/** @def dcc1CNT1_CLKSRC_HFLPO
*   @brief Alias name for DCC1 Counter 1 Clock Source HFLPO
*
*   This is an alias name for the Clock Source HFLPO for DCC1 Counter 1.
*
*   @note This value should be used for API argument @a cnt1_Clock_Source
*/
#define dcc1CNT1_CLKSRC_HFLPO 0x0000A003U

/** @def dcc1CNT1_CLKSRC_EXTCLKIN1
*   @brief Alias name for DCC1 Counter 1 Clock Source EXTCLKIN1
*
*   This is an alias name for the Clock Source EXTCLKIN1 for DCC1 Counter 1.
*
*   @note This value should be used for API argument @a cnt1_Clock_Source
*/
#define dcc1CNT1_CLKSRC_EXTCLKIN1 0x0000A005U

/** @def dcc1CNT1_CLKSRC_VCLK
*   @brief Alias name for DCC1 Counter 1 Clock Source VCLK
*
*   This is an alias name for the Clock Source VCLK for DCC1 Counter 1.
*
*   @note This value should be used for API argument @a cnt1_Clock_Source
*/
#define dcc1CNT1_CLKSRC_VCLK 0x0000A008U

/** @def dcc1CNT1_CLKSRC_N2HET1_31
*   @brief Alias name for DCC1 Counter 1 Clock Source N2HET1_31
*
*   This is an alias name for the Clock Source N2HET1_31 for DCC1 Counter 1.
*
*   @note This value should be used for API argument @a cnt1_Clock_Source
*/
#define dcc1CNT1_CLKSRC_N2HET1_31 0x0000500FU

/** @def dccNOTIFICATION_DONE
*   @brief Alias name for DCC Done notification
*
*   This is an alias name for the DCC Done notification.
*
*   @note This value should be used for API argument @a notification
*/
#define dccNOTIFICATION_DONE 0x0000A000U

/** @def dccNOTIFICATION_ERROR
*   @brief Alias name for DCC Error notification
*
*   This is an alias name for the DCC Error notification.
*
*   @note This value should be used for API argument @a notification
*/
#define dccNOTIFICATION_ERROR 0x000000A0U


/** @enum dcc1clocksource
*   @brief Alias names for dcc clock sources
*
*   This enumeration is used to provide alias names for the clock sources:
*/
enum dcc1clocksource
{
    DCC1_CNT0_HF_LPO    = 0x5U,    /**< Alias for DCC1 CNT 0 CLOCK SOURCE 0*/
    DCC1_CNT0_TCK       = 0xAU,    /**< Alias for DCC1 CNT 0 CLOCK SOURCE 1*/
    DCC1_CNT0_OSCIN     = 0xFU,    /**< Alias for DCC1 CNT 0 CLOCK SOURCE 2*/

    DCC1_CNT1_PLL1      = 0x0U,    /**< Alias for DCC1 CNT 1 CLOCK SOURCE 0*/
    DCC1_CNT1_LF_LPO    = 0x2U,    /**< Alias for DCC1 CNT 1 CLOCK SOURCE 2*/
    DCC1_CNT1_HF_LPO    = 0x3U,    /**< Alias for DCC1 CNT 1 CLOCK SOURCE 3*/
    DCC1_CNT1_EXTCLKIN1 = 0x5U,    /**< Alias for DCC1 CNT 1 CLOCK SOURCE 4*/
    DCC1_CNT1_VCLK      = 0x8U,    /**< Alias for DCC1 CNT 1 CLOCK SOURCE 8*/
    DCC1_CNT1_N2HET1_31 = 0xAU     /**< Alias for DCC1 CNT 1 CLOCK SOURCE 9*/
};


/* Configuration registers */
typedef struct dcc_config_reg
{
    uint32 CONFIG_GCTRL;
    uint32 CONFIG_CNT0SEED;
    uint32 CONFIG_VALID0SEED;
    uint32 CONFIG_CNT1SEED;
    uint32 CONFIG_CNT1CLKSRC;
    uint32 CONFIG_CNT0CLKSRC;
} dcc_config_reg_t;


/* Configuration registers initial value */
#define DCC1_GCTRL_CONFIGVALUE       ( (uint32)0xAU \
                                     | (uint32)((uint32)0xAU << 4U) \
                                     | (uint32)((uint32)0x5U << 8U) \
                                     | (uint32)((uint32)0xAU << 12U))
                                     
#define DCC1_CNT0SEED_CONFIGVALUE    78408U
#define DCC1_VALID0SEED_CONFIGVALUE    1584U
#define DCC1_CNT1SEED_CONFIGVALUE    99000U
#define DCC1_CNT1CLKSRC_CONFIGVALUE  ((uint32)((uint32)10U << 12U) | (uint32)DCC1_CNT1_PLL1)
/*SAFETYMCUSW 79 S MR:19.4 <APPROVED> "Values come from GUI drop down option" */
#define DCC1_CNT0CLKSRC_CONFIGVALUE  ((uint32)DCC1_CNT0_OSCIN)


/* USER CODE BEGIN (1) */
/* USER CODE END */

/**
 *  @defgroup DCC DCC
 *  @brief Dual-Clock Comparator Module
 *
 *  The primary purpose of a DCC module is to measure the frequency of a clock signal using a second
 *  known clock signal as a reference. This capability can be used to ensure the correct frequency range for
 *  several different device clock sources, thereby enhancing the system safety metrics.
 *
 *    Related Files
 *   - reg_dcc.h
 *   - dcc.h
 *   - dcc.c
 *  @addtogroup DCC
 *  @{
 */

/* DCC Interface Functions */
void dccInit(void);
void dccSetCounter0Seed(dccBASE_t  *dcc, uint32 cnt0seed);
void dccSetTolerance(dccBASE_t  *dcc, uint32 valid0seed);
void dccSetCounter1Seed(dccBASE_t  *dcc, uint32 cnt1seed);
void dccSetSeed(dccBASE_t  *dcc, uint32 cnt0seed, uint32 valid0seed, uint32 cnt1seed);
void dccSelectClockSource(dccBASE_t  *dcc, uint32 cnt0_Clock_Source, uint32 cnt1_Clock_Source);
void dccEnable(dccBASE_t  *dcc);
void dccDisable(dccBASE_t  *dcc);
uint32 dccGetErrStatus(dccBASE_t  *dcc);

void dccEnableNotification(dccBASE_t  *dcc, uint32 notification);
void dccDisableNotification(dccBASE_t  *dcc, uint32 notification);
void dcc1GetConfigValue(dcc_config_reg_t *config_reg, config_value_type_t type);
/** @fn void dccNotification(dccBASE_t  *dcc,uint32 flags)
*   @brief Interrupt callback
*   @param[in] dcc   - dcc module base address
*   @param[in] flags - status flags
*
* This is a callback function provided by the application.  It is call when
* a dcc is complete or detected error.
*/
void dccNotification(dccBASE_t  *dcc,uint32 flags);

/* USER CODE BEGIN (2) */
/* USER CODE END */
/**@}*/
#ifdef __cplusplus
}
#endif

#endif
