/*
 * interrupt.c
 *
 *  Created on: 2018骞�1鏈�21鏃�
 *      Author: ases-jack
 */
#include "interrupt.h"
#include "esm.h"
#include "het.h"
#include "can.h"
#include "gio.h"
#include "FreeRTOS.h"
#include "os_task.h"
#include "can_lib.h"
#include "counter.h"
#include "reg_tcram.h"
#include "print.h"
#include "glib.h"
#include "Registers_FMC_BE.h"
#include "can_cmd.h"

uint8 uart_receive_flag = FALSE;


/*
 * EDAC
 */
volatile uint64* singleErrorAddr = 0;
uint32 singleErrorCnter = 0;
volatile uint32* FlashCorrctableErrorAddr = 0;
uint32 FlashUnCorrctableErrorAddr = 0;
uint32 UncorrectableErrorAddr = 0;
/*
 * Rw speed measue
 */
uint32 rw_speed[2] = {0, 0};
/*
 * Semaphore
 */
SemaphoreHandle_t xCanSemaphore = NULL;
SemaphoreHandle_t xGnssSemaphore = NULL;
SemaphoreHandle_t xTeleCmdSemaphore = NULL;
SemaphoreHandle_t xSemaphoreRfOneFrameBin = NULL;
SemaphoreHandle_t xSemaphoreUart = NULL;

void esmGroup1Notification(uint32 channel)
{
	uint32 tcram1ErrStat, tcram2ErrStat;
	uint32 data;
	//print("Enter Notification %d\n", channel);
	switch (channel) {
	case 2:
		/* DMA 鍐呭瓨璁块棶鍏佽杩濊 */
		break;
	case 3:
		/* DMA 鍐呭瓨濂囧伓鏍￠獙閿欒 */
		break;
	case 6:
		/* FMC 鍙籂姝ｇ殑 ECC 閿欒-鎬荤嚎 1 鍜屾�荤嚎 2 鎺ュ彛锛堜笉鍖呮嫭鍒扮粍7锛� */
		if (FMC_Reg->FcorErrCnt.u32Register > 0)
		{
			FlashCorrctableErrorAddr = (volatile uint32*) FMC_Reg->FcorErrAdd.FCOR_ERR_ADD_BITS.COR_ERR_ADD;
			FMC_Reg->FedAcStatus.u32Register = 0x00010006U;
			esmREG->SR1[0U] = 0x40U;
            /* The nERROR pin will become inactive once the LTC counter expires */
            esmREG->EKR = 0x5U;
		}
		break;
	case 26:
		/* RAM 鍋舵暟缁� (B0TCM) - 鍙籂姝ｇ殑 ECC 閿欒 */
		/* Check for error status */

		tcram1ErrStat = tcram1REG->RAMERRSTATUS & 0x1U;
		/*SAFETYMCUSW 139 S MR:13.7  <APPROVED> "LDRA Tool issue" */
		/*SAFETYMCUSW 139 S MR:13.7  <APPROVED> "LDRA Tool issue" */
		if(tcram1ErrStat == 0U)
		{
			/* TCRAM module does not reflect 1-bit error reported by CPU */

		}
		else
		{
			/* 澶勭悊璇ユ晠闅� */
			//print("B0TCM 1-bit error addr:0x%.8x\n", tcram1REG->RAMSERRADDR + 0x08000000);
			singleErrorCnter++;
			singleErrorAddr = (volatile uint64*)(tcram1REG->RAMSERRADDR + 0x08000000);
			tcram1REG->RAMINTCTRL = 0U;
			data = *singleErrorAddr;
			*singleErrorAddr = data;
			tcram1REG->RAMINTCTRL = 1U;
			/* Clear single bit error flag in TCRAM module */
			tcram1REG->RAMERRSTATUS = 0x1U;
			tcram1REG->RAMOCCUR = 0U;
			/* Clear ESM status */
			esmREG->SR1[0U] = 0x04000000U;
		}
		break;
	case 28:
		/* RAM 濂囨暟缁� (B1TCM) - 鍙籂姝ｇ殑 ECC 閿欒 */
		/* Check for error status */
		tcram2ErrStat = tcram2REG->RAMERRSTATUS & 0x1U;
		/*SAFETYMCUSW 139 S MR:13.7  <APPROVED> "LDRA Tool issue" */
		/*SAFETYMCUSW 139 S MR:13.7  <APPROVED> "LDRA Tool issue" */
		if(tcram2ErrStat == 0U)
		{
			/* TCRAM module does not reflect 1-bit error reported by CPU */
		}
		else
		{
			/* 澶勭悊璇ユ晠闅� */
			//print("B1TCM 1bit error addr:0x%.8x\n", tcram2REG->RAMSERRADDR + 0x08000000);
			singleErrorCnter++;
			singleErrorAddr = (volatile uint64*)(tcram2REG->RAMSERRADDR + 0x08000000);
			tcram2REG->RAMINTCTRL = 0U;
			data = *singleErrorAddr;
			*singleErrorAddr = data;
			tcram2REG->RAMINTCTRL = 1U;
			/* Clear single bit error flag in TCRAM module */
			tcram2REG->RAMERRSTATUS = 0x1U;
			tcram2REG->RAMOCCUR = 0U;

			/* Clear ESM status */
			esmREG->SR1[0U] = 0x10000000U;
		}
		break;
	case 35:
		/* FMC 鍙籂姝ｇ殑閿欒-鍒扮粍 7 鐨勮闂� */
		break;
	case 36:
		/* FMC 涓嶅彲绾犳鐨勯敊璇�-鍒扮粍 7 鐨勮闂� */
		break;
	default:
		;
	}
}

#pragma WEAK(esmGroup2Notification)
void esmGroup2Notification(uint32 channel)
{
	switch(channel){
	case 4:
		/* FMC 涓嶅彲绾犳鐨勯敊璇�-鎬荤嚎 1 璁块棶鏃剁殑鍦板潃濂囧伓鏍￠獙閿欒*/
		FlashUnCorrctableErrorAddr =  FMC_Reg->FcorErrAdd.FCOR_ERR_ADD_BITS.COR_ERR_ADD;
		break;
	case 6:
		/* B0 TCM锛堝伓锛夋棤娉曟洿姝ｇ殑閿欒锛堝嵆鍐椾綑鍦板潃瑙ｇ爜锛�*/
		UncorrectableErrorAddr = tcram1REG->RAMUERRADDR;
		//print("B0TCM 2bit error addr:%x\n", UncorrectableErrorAddr);
		break;
	case 8:
		/* B1 TCM锛堝鏁帮級鏃犳硶鏇存鐨勯敊璇紙鍗冲啑浣欏湴鍧�瑙ｇ爜锛�*/
		UncorrectableErrorAddr = tcram2REG->RAMUERRADDR;
		//print("B1TCM 2bit error addr:%x\n", UncorrectableErrorAddr);
		break;
	case 10:
		/* B0 TCM锛堝伓锛夊湴鍧�鎬荤嚎濂囧伓鏍￠獙閿欒 */
		break;
	case 12:
		/* B1 TCM锛堝鏁帮級鍦板潃鎬荤嚎濂囧伓鏍￠獙閿欒 */
		break;
	default:
		;
	}
}

void pwmNotification(hetBASE_t * hetREG,uint32 pwm, uint32 notification)
{
	uint32 edgeCnt1, edgeCnt2;
	//1 sec timer inerrupt without rti
	if((pwm == pwm7) && (notification == pwmEND_OF_PERIOD)){
		edgeCnt1 = edgeGetCounter(hetRAM1, edge0);
		edgeCnt2 = edgeGetCounter(hetRAM1, edge1);
		edgeResetCounter(hetRAM1, edge0);
		edgeResetCounter(hetRAM1, edge1);
		rw_speed[0] = edgeCnt1 * 10;
		rw_speed[1] = edgeCnt2 * 10;
	}
}

void canErrorNotification(canBASE_t *node, uint32 notification)
{
//    if(node == canREG1) {
//		ADD(CNT_CAN_1_ERROR);
//	} else {
//	    ADD(CNT_CAN_2_ERROR);
//	}
}

void canStatusChangeNotification(canBASE_t *node, uint32 notification)
{

}

void canMessageNotification(canBASE_t *node, uint32 messageBox)
{
    canIRQ(node, messageBox);
}

void gioNotification(gioPORT_t *port, uint32 bit)
{
	if(port == gioPORTB) {
		if(bit == 3) {
			/* TXDONE */
		    gioSetBit(gioPORTB, 0, 0);

		}
	}
}

void sciNotification(sciBASE_t *sci, uint32 flags)
{
	if(sci == sciREG) {
	    if(flags == SCI_RX_INT) {
	    	uart_receive_flag = TRUE;
	    }
	}
}
