/*
 * main.c
 *
 *  Created on: 2019Äê7ÔÂ30ÈÕ
 *      Author: houzh
 */


#include "TC_CUBESAT_Connections.h"
#include "gio.h"
#include "can.h"
#include "spi.h"
#include "het.h"
#include "sci.h"

#include "sx1276-Fsk.h"
#include "sx1276-LoRaMisc.h"
#include "sx1276-LoRa.h"



/*!
 * Sync word for Private LoRa networks
 */
#define LORA_MAC_PRIVATE_SYNCWORD                   0x12

/*!
 * Sync word for Public LoRa networks
 */
#define LORA_MAC_PUBLIC_SYNCWORD                    0x34

#define data_N 32

uint8_t frame_header_temp[4];
uint8_t tx_data[128];
uint8_t rx_data[32];


/*uint8_t tx_data[32] = {0x02,0x2A,0x04,0x3F,0x00,0x80,0x38,0x55,
                       0xBB,0xED,0xAC,0xCA,0xDE,0xFA,0xCE,0xBE,0xAF,0x00,
                       0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                       0x00,0x00,0x79,0x13};*/



void data_ini(void)
{
   uint8_t i;
    for(i=0;i<data_N;i++)
     {
        tx_data[i] = i;
        rx_data[i] = 0;
     }
}

void SX1276SetPublicNetwork( bool enable )
{
    if( enable == true )
    {
        // Change LoRa modem SyncWord
        tx_vSpiWrite( REG_LR_SYNCWORD, LORA_MAC_PUBLIC_SYNCWORD );
    }
    else
    {
        // Change LoRa modem SyncWord
        tx_vSpiWrite( REG_LR_SYNCWORD, LORA_MAC_PRIVATE_SYNCWORD );
    }
}

uint8_t buffer[512];

void main(void)
{


    uint8_t tx_data_length,REG[60]={0};
    uint32_t frame_header;
    uint8_t i;
    memset(&REG , 0 , sizeof(REG));

     gioInit();
     spiInit();
     canInit();
     hetInit();
     sciInit();
     data_ini();
     bb_poweron();
     pa_poweroff();

     tx_WriteBuffer(0x00, data_N, &tx_data[0]);
     tx_ReadBuffer(0x00,data_N, &rx_data[0]);

     uart_receiver_mode();


     tx_module_reset();
     SX1276LoRaInit();

     SX1276SetPublicNetwork(true);

     tx_vSpiWrite(REG_LR_PACONFIG, 0xff);

     for(i=0;i<60;i++)
     {
         REG[i] = tx_bSpiRead(0x0+i);
     }
     for(i=0;i<100;i++)
     {

     }
     while(1)
     {

             sciReceive(scilinREG, 4,  &frame_header_temp[0]);

             frame_header = frame_header_temp[0]<<8 | frame_header_temp[1];
           //  tx_data_length = frame_header_temp[2]<<8 | frame_header_temp[3];
             tx_data_length = frame_header_temp[3];

             if(frame_header == 0xeb90)
             {
               sciReceive(scilinREG, tx_data_length,  &tx_data[0]);
             }


             SX1276LoRaSetPayloadLength(tx_data_length);

             pa_poweron();


             SX1276LR.RegFifoTxBaseAddr = 0x00; // Full buffer used for Tx

             tx_vSpiWrite( REG_LR_FIFOTXBASEADDR, SX1276LR.RegFifoTxBaseAddr );
             SX1276LR.RegFifoAddrPtr = SX1276LR.RegFifoTxBaseAddr;
             tx_vSpiWrite( REG_LR_FIFOADDRPTR, SX1276LR.RegFifoAddrPtr );
             tx_WriteBuffer(REG_LR_FIFO, tx_data_length, &tx_data[0]);
             SX1276LoRaSetOpMode( RFLR_OPMODE_TRANSMITTER );
             while( (tx_bSpiRead(0x12) & 0x08) != 0x08 );
             tx_vSpiWrite( 0x12, 0x08 );
             SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );
             pa_poweroff();
             i = 100000;
             while(i--);
             asm(" nop");




     }

}
 /*   uint16_t tx_data_length;
        uint32_t i;

         gioInit();
         spiInit();
         canInit();
         hetInit();
         sciInit();
         data_ini();
         bb_poweron();
         pa_poweroff();

         tx_WriteBuffer(0x00, data_N, &tx_data[0]);
         tx_ReadBuffer(0x00,data_N, &rx_data[0]);

         uart_receiver_mode();


         tx_module_reset();
         SX1276LoRaInit();
         tx_vSpiWrite(REG_LR_PACONFIG, 0xff);
         while(1)
         {
             if(sciReceiveByte(scilinREG) == 0xeb){
                 if(sciReceiveByte(scilinREG) == 0x90){
                     sciReceive(scilinREG, 2,  (uint8 *)&tx_data_length);
                     sciReceive(scilinREG, tx_data_length,  &tx_data[0]);

                     SX1276LoRaSetPayloadLength(tx_data_length);

                     pa_poweron();

                     SX1276LR.RegFifoTxBaseAddr = 0x00; // Full buffer used for Tx

                     tx_vSpiWrite( REG_LR_FIFOTXBASEADDR, SX1276LR.RegFifoTxBaseAddr );
                     SX1276LR.RegFifoAddrPtr = SX1276LR.RegFifoTxBaseAddr;
                     tx_vSpiWrite( REG_LR_FIFOADDRPTR, SX1276LR.RegFifoAddrPtr );
                     tx_WriteBuffer(REG_LR_FIFO, tx_data_length, &tx_data[0]);
                     SX1276LoRaSetOpMode( RFLR_OPMODE_TRANSMITTER );
                     while( (tx_bSpiRead(0x12) & 0x08) != 0x08 );
                     tx_vSpiWrite( 0x12, 0x08 );
                     SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );
                     pa_poweroff();
                     i = 100000;
                     while(i--);
                     asm(" nop");
                 }
             }
         }
}

*/
