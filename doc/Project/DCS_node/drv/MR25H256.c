/*
 * FM25V20.c
 *
 *  Created on: 2017��11��24��
 *      Author: ases-jack
 */

#include "MR25H256.h"
#include "spi.h"
#include "gio.h"

extern void Delay(volatile int time);

static void MR_CS(uint8 value)
{
	gioSetBit(MR_SPI_PORT, MR_CS_PIN, value);
}


void MR_Write_Enable(void)
{
	spiDAT1_t dataconfig1_t;
	uint16 cmd;

	dataconfig1_t.CS_HOLD = TRUE;
	dataconfig1_t.WDEL = TRUE;
	dataconfig1_t.DFSEL = MR_DATA_FORMAT;   /* PHASE is 1, POLARITY is 0, charlen is 8bit */
	dataconfig1_t.CSNR = MR_CHIP_SELECT;   /* Chip select */

	cmd = WREN;
	MR_CS(FALSE);
	Delay(1);
	spiTransmitData(MR_SPI_REG, &dataconfig1_t, 1, &cmd);
	Delay(1);
	MR_CS(TRUE);
}


void MR_Write_Disable(void)
{
	spiDAT1_t dataconfig1_t;
	uint16 cmd;

	dataconfig1_t.CS_HOLD = TRUE;
	dataconfig1_t.WDEL = TRUE;
	dataconfig1_t.DFSEL = MR_DATA_FORMAT;   /* PHASE is 1, POLARITY is 0, charlen is 8bit */
	dataconfig1_t.CSNR = MR_CHIP_SELECT;   /* Chip select */

	cmd = WRDI;
	MR_CS(FALSE);
	Delay(1);
	spiTransmitData(MR_SPI_REG, &dataconfig1_t, 1, &cmd);
	Delay(1);
	MR_CS(TRUE);
}

uint8 MR_Read_Status(void)
{
	spiDAT1_t dataconfig1_t;
	uint16 cmd;

	dataconfig1_t.CS_HOLD = TRUE;
	dataconfig1_t.WDEL = TRUE;
	dataconfig1_t.DFSEL = MR_DATA_FORMAT;   /* PHASE is 1, POLARITY is 0, charlen is 8bit */
	dataconfig1_t.CSNR = MR_CHIP_SELECT;   /* Chip select */

	cmd = RDSR;
	MR_CS(FALSE);
	Delay(1);
	spiTransmitData(MR_SPI_REG, &dataconfig1_t, 1, &cmd);
	spiReceiveData(MR_SPI_REG, &dataconfig1_t, 1, &cmd);
	Delay(1);
	MR_CS(TRUE);
	return cmd;
}

void MR_Write_Status(uint8 status)
{
	spiDAT1_t dataconfig1_t;
	uint16 cmd[2];

	dataconfig1_t.CS_HOLD = TRUE;
	dataconfig1_t.WDEL = TRUE;
	dataconfig1_t.DFSEL = MR_DATA_FORMAT;   /* PHASE is 1, POLARITY is 0, charlen is 8bit */
	dataconfig1_t.CSNR = MR_CHIP_SELECT;   /* Chip select */

	cmd[0] = WRSR;
	cmd[1] = status;
	MR_CS(FALSE);
	Delay(1);
	spiTransmitData(MR_SPI_REG, &dataconfig1_t, 2, cmd);
	Delay(1);
	MR_CS(TRUE);
}

void MR_Write_Data(uint32 addr, uint8* data, uint32 len)
{
	int i;
	spiDAT1_t dataconfig1_t;
	uint16 cmd[4];
	uint16 fram_data;

	if(data == NULL){
		return ;
	}
	MR_Write_Enable();

	dataconfig1_t.CS_HOLD = TRUE;
	dataconfig1_t.WDEL = TRUE;
	dataconfig1_t.DFSEL = MR_DATA_FORMAT;   /* PHASE is 1, POLARITY is 0, charlen is 8bit */
	dataconfig1_t.CSNR = MR_CHIP_SELECT;   /* Chip select */

	cmd[0] = WRITE;
	cmd[1] = (addr >> 8) & 0xFF;
	cmd[2] = addr & 0xFF;

	MR_CS(FALSE);
	Delay(1);
	spiTransmitData(MR_SPI_REG, &dataconfig1_t, 3, cmd);
	for(i = 0; i < len; i++) {
		fram_data = data[i];
		spiTransmitData(MR_SPI_REG, &dataconfig1_t, 1, &fram_data);
	}
	Delay(1);
	MR_CS(TRUE);

	MR_Write_Disable();
}

void MR_Read_Data(uint32 addr, uint8* data, uint32 len)
{
	int i;
	spiDAT1_t dataconfig1_t;
	uint16 cmd[4];
	uint16 fram_data;

	if(data == NULL){
		return ;
	}

	dataconfig1_t.CS_HOLD = TRUE;
	dataconfig1_t.WDEL = TRUE;
	dataconfig1_t.DFSEL = MR_DATA_FORMAT;   /* PHASE is 1, POLARITY is 0, charlen is 8bit */
	dataconfig1_t.CSNR = MR_CHIP_SELECT;   /* Chip select */

	cmd[0] = READ;
	cmd[1] = (addr >> 8) & 0xFF;
	cmd[2] = addr & 0xFF;

	MR_CS(FALSE);
	Delay(1);
	spiTransmitData(MR_SPI_REG, &dataconfig1_t, 3, cmd);
	for(i = 0; i < len; i++) {
		spiReceiveData(MR_SPI_REG, &dataconfig1_t, 1, &fram_data);
		data[i] = fram_data & 0xFF;
	}
	Delay(1);
	MR_CS(TRUE);
}

void MR_Clear_All(void)
{
	int i;
	spiDAT1_t dataconfig1_t;
	uint16 cmd[4];
	uint16 fram_data = 0;

	MR_Write_Enable();

	dataconfig1_t.CS_HOLD = TRUE;
	dataconfig1_t.WDEL = TRUE;
	dataconfig1_t.DFSEL = MR_DATA_FORMAT;   /* PHASE is 1, POLARITY is 0, charlen is 8bit */
	dataconfig1_t.CSNR = MR_CHIP_SELECT;   /* Chip select */

	cmd[0] = WRITE;
	cmd[1] = 0;
	cmd[2] = 0;
	cmd[3] = 0;

	MR_CS(FALSE);
	Delay(1);
	spiTransmitData(MR_SPI_REG, &dataconfig1_t, 4, cmd);
	for(i = 0; i <= MAX_ADDRESS ; i++){
		spiTransmitData(MR_SPI_REG, &dataconfig1_t, 1, &fram_data);
	}
	Delay(1);
	MR_CS(TRUE);

	MR_Write_Disable();
}

void MR_Clear_Piece(uint32 base, uint32 len)
{
	int i;
	spiDAT1_t dataconfig1_t;
	uint16 cmd[4];
	uint16 fram_data = 0;

	if((base > MAX_ADDRESS) || ((base + len) > MAX_ADDRESS)) {
		return ;
	}

	MR_Write_Enable();

	dataconfig1_t.CS_HOLD = TRUE;
	dataconfig1_t.WDEL = TRUE;
	dataconfig1_t.DFSEL = MR_DATA_FORMAT;   /* PHASE is 1, POLARITY is 0, charlen is 8bit */
	dataconfig1_t.CSNR = MR_CHIP_SELECT;   /* Chip select */

	cmd[0] = WRITE;
	cmd[1] = (base >> 16) & 0xFF;
	cmd[2] = (base >> 8) & 0xFF;
	cmd[3] = base & 0xFF;

	MR_CS(FALSE);
	Delay(1);
	spiTransmitData(MR_SPI_REG, &dataconfig1_t, 4, cmd);
	for(i = 0; i < len ; i++){
		spiTransmitData(MR_SPI_REG, &dataconfig1_t, 1, &fram_data);
	}
	Delay(1);
	MR_CS(TRUE);

	MR_Write_Disable();
}
