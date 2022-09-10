/*
 * SN65LVDS31PW.h
 *
 *  Created on: 2020年4月12日
 *      Author: 86159
 */

#ifndef SRC_DRV_SN65LVDS31PW_H_
#define SRC_DRV_SN65LVDS31PW_H_

#include "gio.h"
#include "pin_config.h"

#define     LVDS_CS(n)          n ? gioSetBit(LVDS_SPI_PORT, LVDS_CS_PIN, 1) : gioSetBit(LVDS_SPI_PORT, LVDS_CS_PIN, 0)

extern void        LVDS_SpiWrite(uint8* data, uint32 data_len);
extern void        LVDS_TEST_SpiWrite(uint8 *data, uint32 data_len);

#endif /* SRC_DRV_SN65LVDS31PW_H_ */
