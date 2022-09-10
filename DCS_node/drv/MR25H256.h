/*
 * FM25V20.h
 *
 *  Created on: 2017��11��24��
 *      Author: ases-jack
 */

#ifndef DRV_FM25V20_H_
#define DRV_FM25V20_H_

#include "pin_config.h"

#define MR_SPI_REG       MRAM_SPI_REG
#define MR_SPI_PORT      MRAM_PORT

#define MR_CHIP_SELECT   MRAM_CS_PIN
#define MR_CS_PIN        MRAM_CS_PIN


#define MR_DATA_FORMAT   MRAM_DATA_FORMAT

#define MAX_ADDRESS       0x3FFFF
#define MIN_ADDRESS       0x00000

#define WHOLE_SIZE        0x40000


/*
 * Op Code
 */
#define WREN    0x06    /* Set Write Enable Latch */
#define WRDI    0x04    /* Write Disable */
#define RDSR    0x05    /* Read Status Register */
#define WRSR    0x01    /* Write Status Register */
#define READ    0x03    /* Read Memory Data */
#define FSTRD   0x0B    /* Fast Read Memory Data */
#define WRITE   0x02    /* Write Memory Data */
#define SLEEP   0xB9    /* Enter Sleep Mode */
#define WAKE    0xAB    /* Read Device ID */


#define WPEN_MASK     0x80
#define BP_MASK       0x0C
#define WEL_MASK      0x02


/*
 * Function
 */
extern void MR_Write_Enable(void);
extern void MR_Write_Disable(void);
extern uint8 MR_Read_Status(void);
extern void MR_Write_Status(uint8 status);
extern void MR_Write_Data(uint32 addr, uint8* data, uint32 len);
extern void MR_Read_Data(uint32 addr, uint8* data, uint32 len);
extern void MR_Clear_All(void);
extern void MR_Clear_Piece(uint32 base, uint32 len);
#endif /* DRV_FM25V20_H_ */
