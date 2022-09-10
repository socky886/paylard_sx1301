/*
 * pin_config.h
 *
 *  Created on: 2017��11��28��
 *      Author: ases-jack
 */

#ifndef DRV_PIN_CONFIG_H_
#define DRV_PIN_CONFIG_H_
#include "spi.h"
#include "sci.h"
#include "het.h"

#define LIN_RX_PIN   1
#define LIN_TX_PIN   2


/*
 * SPI channel
 */
#define MRAM_SPI_REG      spiREG3
#define LVDS_SPI_REG      spiREG5
/*
 * SPI port
 */
#define MRAM_PORT       spiPORT3
#define LVDS_SPI_PORT   spiPORT5
/*
 * SPI CS pin
 */
#define MRAM_CS_PIN     SPI_PIN_CS0
#define LVDS_CS_PIN     SPI_PIN_CS0
/*
 * Data Format
 */
#define SX_DATA_FORMAT    SPI_FMT_0
#define MRAM_DATA_FORMAT  SPI_FMT_1
#define LVDS_DATA_FORMAT  SPI_FMT_0

#endif /* DRV_PIN_CONFIG_H_ */
