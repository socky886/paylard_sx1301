#ifndef APP_CRC_H_
#define APP_CRC_H_

#include "types_def.h"

#define CRC_INITIAL_VALUE   0xFFFF


/*
 * Crc
 */
#define CRC_TYPE_CCITT   0
#define CRC_TYPE_IBM     1

#define POLYNOMIAL_CCITT      0x1021
#define POLYNOMIAL_IBM        0x8005

#define CRC_IBM_SEED          0xFFFF
#define CRC_CCITT_SEED        0x1D0F

/***********************************************************************
*
* FUNCTION:
* CalcCrc
*
* INPUTS:
* seed - (uint16) initial value of check bits.
* buf - (unsigned char *) pointer to the buffer of
* data over which you wish to generate check
* bits.
* len - (int) number to bytes of data in the buffer.
*
* OUTPUTS:
*
* RETURNS:
* - (uint16) the checkbits.
*
* EXTERNALLY READ:
* sdlc_table - (uint16)[256] the lookup table for the CCITT SDLC
* generator polynomial (local to this module).
*
* EXTERNALLY MODIFIED:
*
* DESCRIPTION:
* This function implements CRC generation with the CCITT SDLC error
* polynomial (X16 + X12 + X5 + 1). You must provide it with an
* initial seed value, a pointer to a data buffer, and the byte length
* of the data buffer. It will return the unsigned 16-bit CRC.
*
* You may use this function to generate a CRC over data in scattered
* storage by making multiple calls to it. Just make sure that you
* pass a seed of 0xFFFF on the first call. On subsequent calls, pass
* a seed containing the return value of the previous call.
*
*/
extern uint16 CalcCrc (uint16 seed, const unsigned char *buf, int len);

extern uint16 PacketComputerCRC(uint8* buffer, uint8 bufferlength, uint8 crcType);

extern uint16 CheckCrc(uint8* data, uint16 len);
#endif //APP_CRC_H_
