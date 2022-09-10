/*
 * SX1276-Fsk.c
 *
 *  Created on: 2018Äê7ÔÂ16ÈÕ
 *      Author: ASES_Hz
 */

#include "sx1276-Fsk.h"
#include "TC_CUBESAT_Connections.h"


tFskSettings FskSettings =
{
    401098850,      // RFFrequency
    1200,           // Bitrate
    5000,          // Fdev
    15,             // Power
    10000,         // RxBw
    150000,         // RxBwAfc
    true,           // CrcOn
    true,           // AfcOn
    255             // PayloadLength (set payload size to the maximum for variable mode, else set the exact payload length)
};
tSX1276 SX1276_REG;       // register var

tSX1276 *SX1276 = &SX1276_REG;

//RF FREQUENCY
void SX1276FskSetRFFrequency(uint32_t freq)
{

	FskSettings.RFFrequency = freq;
    freq = (uint32_t) ((double) freq / (double) FREQ_STEP);
    SX1276->RegFrfMsb = (uint8_t) ((freq >> 16) & 0xFF);
    SX1276->RegFrfMid = (uint8_t) ((freq >> 8) & 0xFF);
    SX1276->RegFrfLsb = (uint8_t) (freq & 0xFF);

    tx_vSpiWrite( REG_FRFMSB, SX1276->RegFrfMsb);
    tx_vSpiWrite( REG_FRFMID, SX1276->RegFrfMid);
    tx_vSpiWrite( REG_FRFLSB, SX1276->RegFrfLsb);

}

//Bitrate
void SX1276FskSetBitrate(uint32_t bitrate)
{
    FskSettings.Bitrate = bitrate;

    bitrate = (uint16_t) ((double) XTAL_FREQ / (double) bitrate);

    SX1276->RegBitrateMsb = (uint8_t) (bitrate >> 8);
    SX1276->RegBitrateLsb = (uint8_t) (bitrate & 0xFF);

    tx_vSpiWrite( REG_BITRATEMSB, SX1276->RegBitrateMsb);
    tx_vSpiWrite( REG_BITRATELSB, SX1276->RegBitrateLsb);

}
//
void SX1276FskSetFdev( uint32_t fdev )
{
    FskSettings.Fdev = fdev;

    SX1276->RegFdevMsb = tx_bSpiRead( REG_FDEVMSB);
    SX1276->RegFdevLsb = tx_bSpiRead( REG_FDEVLSB);

    fdev = ( uint16_t )( ( double )fdev / ( double )FREQ_STEP );
    SX1276->RegFdevMsb    = ( ( SX1276->RegFdevMsb & RF_FDEVMSB_FDEV_MASK ) | ( ( ( uint8_t )( fdev >> 8 ) ) & ~RF_FDEVMSB_FDEV_MASK ) );
    SX1276->RegFdevLsb    = ( uint8_t )( fdev & 0xFF );
    tx_vSpiWrite( REG_FDEVMSB, SX1276->RegFdevMsb );
    tx_vSpiWrite( REG_FDEVLSB, SX1276->RegFdevLsb );

}
// PA PINOUT
void SX1276FskSetPAOutput( uint8_t outputPin )
{

    SX1276->RegPaConfig = ( tx_bSpiRead(REG_PACONFIG)  & RF_PACONFIG_PASELECT_MASK ) | outputPin;
    tx_vSpiWrite( REG_PACONFIG, SX1276->RegPaConfig);

}

// PA Power Set
void SX1276FskSetRFPower(void )
{
    tx_vSpiWrite( REG_PACONFIG, 0xff);

}
//
void SX1276PayloadLength( void )
{
    SX1276->RegPayloadLength = FskSettings.PayloadLength;
    tx_vSpiWrite( REG_PAYLOADLENGTH, SX1276->RegPayloadLength);

}
//
void SX1276FskSetOpMode( uint8_t opMode )
{
	SX1276->RegOpMode = tx_bSpiRead(REG_OPMODE);
	SX1276->RegOpMode = ( SX1276->RegOpMode & RF_OPMODE_MASK ) | opMode;
	tx_vSpiWrite( REG_OPMODE, SX1276->RegOpMode );
}
//
void SX1276FskSetPackageMode( uint8_t PackageMode )
{
	SX1276->RegPacketConfig1 = tx_bSpiRead(REG_PACKETCONFIG1);
	SX1276->RegPacketConfig1 = ( SX1276->RegPacketConfig1 & 0x7F) | PackageMode;
	tx_vSpiWrite( REG_PACKETCONFIG1, SX1276->RegPacketConfig1 );
}
//
void SX1276FskSetPayLoadLength( uint8_t length )
{
	SX1276->RegPayloadLength = length;
	tx_vSpiWrite( REG_PAYLOADLENGTH, SX1276->RegPayloadLength );
}
//
//
void SX1276SetFskMode(void)
{
	SX1276->RegOpMode = tx_bSpiRead(REG_OPMODE);
    SX1276->RegOpMode = SX1276->RegOpMode & 0x7f;
    tx_vSpiWrite( REG_OPMODE, SX1276->RegOpMode );

}
//

void SX1276FskInit( void )
{

	SX1276FskSetOpMode(RF_OPMODE_SLEEP);                                //sleep mode

	SX1276FskSetRFFrequency(FskSettings.RFFrequency);                   //set rf_frequency
    SX1276FskSetBitrate(FskSettings.Bitrate);                           //set bitrate
    SX1276FskSetFdev(FskSettings.Fdev );                                //set fdv
    SX1276FskSetRFPower();                                              //set rf_transmit_power

    SX1276FskSetPackageMode(0x00);                                      //tx package mode
//    SX1276FskSetPayLoadLength(0x1f);                                    //tx payload length
//    vSpiWrite(0x35, 0x09 );
    SX1276FskSetOpMode(RF_OPMODE_STANDBY);                              //standby mode

}
//
