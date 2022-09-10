/*
 * SN65LVDS31PW.c
 *
 *  Created on: 2020年4月12日
 *      Author: 86159
 */
#include "SN65LVDS31PW.h"

#include "pin_config.h"

void LVDS_SpiWrite(uint8 *data, uint32 data_len)
{
    int i;
    uint16 write_buff;
    spiDAT1_t dataconfig1_t;

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL = TRUE;                      /* Wdelay according VCLK1 */
    dataconfig1_t.DFSEL = LVDS_DATA_FORMAT;         /* PHASE is 1, POLARITY is 0, charlen is 8bit */
    dataconfig1_t.CSNR = (~LVDS_CS_PIN) & 0xFF;     /* Chip select */

    LVDS_CS(0);
    for(i = 0; i < data_len; i++) {
        write_buff = data[i];
        spiTransmitData(LVDS_SPI_REG, &dataconfig1_t, 1, &write_buff);

    }
    LVDS_CS(1) ;
}
