/*
 * MCU_CONNECTIONS.c
 *
 *  Created on: 2019Äê2ÔÂ11ÈÕ
 *      Author: houzh
 */

#include "TC_CUBESAT_Connections.h"

//
void delay_ms(uint32_t time)
{
    uint32_t scale;
    scale =time*2000;    //
    while( scale--);
}
//
void delay_us(uint32 us)
{
    volatile uint32 time = us;
    while(time--);
}
//
/*
void fram_enable(void)
{
    gioSetBit(general_port, fram_cs_pin, 0);
}
void fram_disable(void)
{
    gioSetBit(general_port, fram_cs_pin, 1);
}
void fram_hardware_wp_enable(void)
{
    gioSetBit(general_port, fram_write_p_pin, 0);
}
void fram_hardware_wp_disable(void)
{
    gioSetBit(general_port, fram_write_p_pin, 1);
}
void fram_write_enable(void)
{
    spiDAT1_t dataconfig1_t;
    uint16 cmd;

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL = TRUE;
    dataconfig1_t.DFSEL = SPI_FMT_0;
    dataconfig1_t.CSNR = 0;

    cmd = WREN;
    fram_enable();
    fram_hardware_wp_disable();
    delay_us(2);
    spiTransmitData(fram_port, &dataconfig1_t, 1, &cmd);
    delay_us(2);
    fram_hardware_wp_enable();
    fram_disable();
}

void fram_write_disable(void)
{
    spiDAT1_t dataconfig1_t;
    uint16 cmd;

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL = TRUE;
    dataconfig1_t.DFSEL = SPI_FMT_0;
    dataconfig1_t.CSNR = 0;

    cmd = WRDI;
   // fram_enable();
   // fram_hardware_wp_disable();
    delay_us(2);
    spiTransmitData(fram_port, &dataconfig1_t, 1, &cmd);
    delay_us(2);
  //  fram_hardware_wp_enable();
  //  fram_disable();
}

uint8 fram_read_status(void)
{
    spiDAT1_t dataconfig1_t;
    uint16 cmd;

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL = TRUE;
    dataconfig1_t.DFSEL = SPI_FMT_0;
    dataconfig1_t.CSNR = 0;

    cmd = RSDR ;
    fram_enable();
    fram_hardware_wp_disable();
    delay_us(2);
    spiTransmitData(fram_port, &dataconfig1_t, 1, &cmd);
    spiReceiveData(fram_port, &dataconfig1_t, 1, &cmd);
    delay_us(2);
    fram_hardware_wp_enable();
    fram_disable();
    return cmd;

}
//

void gps_power_on(void)
{
    gioSetBit(spiPORT2, 0, 1);
}
void gps_power_off(void)
{
    gioSetBit(spiPORT2, 0, 0);
}
*/
//

void switch0_on(void)
{
    gioSetBit(switchio_port, switch_io_0, 1);
}
void switch0_off(void)
{
    gioSetBit(switchio_port, switch_io_0, 0);
}

void switch1_on(void)
{
    gioSetBit(switchio_port, switch_io_1, 1);
}
void switch1_off(void)
{
    gioSetBit(switchio_port, switch_io_1, 0);
}

void switch2_on(void)
{
    gioSetBit(switchio_port, switch_io_2, 1);
}
void switch2_off(void)
{
    gioSetBit(switchio_port, switch_io_2, 0);
}

void switch3_on(void)
{
    gioSetBit(switchio_port, switch_io_3, 1);
}
void switch3_off(void)
{
    gioSetBit(switchio_port, switch_io_3, 0);
}

void switch4_on(void)
{
    gioSetBit(switchio_port, switch_io_4, 1);
}
void switch4_off(void)
{
    gioSetBit(switchio_port, switch_io_4, 0);
}

void switch5_on(void)
{
    gioSetBit(switchio_port, switch_io_5, 1);
}
void switch5_off(void)
{
    gioSetBit(switchio_port, switch_io_5, 0);
}

void switch6_on(void)
{
    gioSetBit(switchio_port, switch_io_6, 1);
}
void switch6_off(void)
{
    gioSetBit(switchio_port, switch_io_6, 0);
}

void switch7_on(void)
{
    gioSetBit(switchio_port, switch_io_7, 1);
}
void switch7_off(void)
{
    gioSetBit(switchio_port, switch_io_7, 0);
}

void switch8_on(void)
{
    gioSetBit(general_port, switch_io_8, 1);
}
void switch8_off(void)
{
    gioSetBit(general_port, switch_io_8, 0);
}

void switch9_on(void)
{
    gioSetBit(general_port, switch_io_9, 1);
}
void switch9_off(void)
{
    gioSetBit(general_port, switch_io_9, 0);
}

void switch_all_on(void)
{
    gioSetBit(switchio_port, switch_io_0, 1);
    gioSetBit(switchio_port, switch_io_1, 1);
    gioSetBit(switchio_port, switch_io_2, 1);
    gioSetBit(switchio_port, switch_io_3, 1);
    gioSetBit(switchio_port, switch_io_7, 1);
    gioSetBit(general_port, switch_io_8, 1);
    gioSetBit(general_port, switch_io_9, 1);
}
void switch_all_off(void)
{
    gioSetBit(switchio_port, switch_io_0, 0);
    gioSetBit(switchio_port, switch_io_1, 0);
    gioSetBit(switchio_port, switch_io_2, 0);
    gioSetBit(switchio_port, switch_io_3, 0);
    gioSetBit(switchio_port, switch_io_7, 0);
    gioSetBit(general_port, switch_io_8, 0);
    gioSetBit(general_port, switch_io_9, 0);

}


void  burning_wire1_on(void)
{
    gioSetBit(general_port, cpu_buringing_io1, 1);
}
void  burning_wire1_off(void)
{
    gioSetBit(general_port, cpu_buringing_io1, 0);
}
void  burning_wire2_on(void)
{
    gioSetBit(general_port, cpu_buringing_io2, 1);
}
void  burning_wire2_off(void)
{
    gioSetBit(general_port, cpu_buringing_io2, 0);
}



void bb_poweron(void)
{
    gioSetBit(general_port, bb_power_pin, 1);
}
void bb_poweroff(void)
{
    gioSetBit(general_port, bb_power_pin, 0);
}

void pa_poweron(void)
{
    gioSetBit(general_port, pa_power_pin, 1);
}
void pa_poweroff(void)
{
    gioSetBit(general_port, pa_power_pin, 0);
}

//
void uart_shutdonw_mode(void)
{
    gioSetBit(general_port, rs_de_pin, 0);
    gioSetBit(general_port, rs_re_pin, 1);
}
//
void uart_transceiver_mode(void)
{
    gioSetBit(general_port, rs_de_pin, 1);
    gioSetBit(general_port, rs_re_pin, 0);
}
//
void uart_transmitter_mode()
{
    gioSetBit(general_port, rs_de_pin, 1);
    gioSetBit(general_port, rs_re_pin, 1);
}
void uart_receiver_mode()
{
    gioSetBit(general_port, rs_de_pin, 0);
    gioSetBit(general_port, rs_re_pin, 0);

}


//
void  rx_module_reset(void)
{
    gioSetBit(general_port, rx_reset_pin, 0);
    delay_ms(2);
    gioSetBit(general_port, rx_reset_pin, 1);
    delay_ms(100);
}
//
void rx_vSpiWrite(uint8 reg, uint8 data)
{
    spiDAT1_t dataconfig1_t;
    uint16 spi_data;

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL = TRUE;
    dataconfig1_t.DFSEL = SPI_FMT_0;
    dataconfig1_t.CSNR = 0;

    spi_data = ((reg << 8) | 0x8000) | data;
    gioSetBit(rx_cs_port, rx_cs_pin,   0);
    spiTransmitData(rx_port, &dataconfig1_t, 1, &spi_data);
    gioSetBit(rx_cs_port, rx_cs_pin,   1);

}
//
uint8 rx_bSpiRead(uint8 reg)
{
    spiDAT1_t dataconfig1_t;
    uint16 spi_data;
    uint16 data;

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL = TRUE;
    dataconfig1_t.DFSEL = SPI_FMT_0;
    dataconfig1_t.CSNR = 0;

    spi_data = (reg << 8);
    gioSetBit(rx_cs_port, rx_cs_pin,   0);
    spiTransmitAndReceiveData(rx_port, &dataconfig1_t, 1, &spi_data, &data);
    gioSetBit(rx_cs_port, rx_cs_pin,   1);
    return (data & 0xFF);
}
//
void rx_ReadBuffer(uint8 reg,uint8 length, uint8_t *data)
{
    spiDAT1_t dataconfig1_t;
    uint16    reg_data;
    uint16    rx_data[255];
    uint8     cnt;

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL    = TRUE;
    dataconfig1_t.DFSEL   = SPI_FMT_1;
    dataconfig1_t.CSNR    = 0;

    reg_data = reg;

    gioSetBit(rx_cs_port, rx_cs_pin, 0);
    spiTransmitData(rx_port, &dataconfig1_t, 1, &reg_data);
    spiReceiveData(rx_port, &dataconfig1_t, length,  &rx_data[0]);
    gioSetBit(rx_cs_port, rx_cs_pin, 1);
    for(cnt=0;cnt<length;cnt++)
    {
        *data = rx_data[cnt];
        data++;
    }

}
//
void rx_WriteBuffer(uint8 reg,uint8 length, uint8_t *data)
{
    spiDAT1_t dataconfig1_t;
    uint16 reg_data;
    uint16 instruction_data[255];
    uint8 cnt;


    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL    = TRUE;
    dataconfig1_t.DFSEL   = SPI_FMT_1;
    dataconfig1_t.CSNR    = 0;

    reg_data = reg | 0x80;
    for(cnt =0;cnt<length;cnt++)
    {
        instruction_data[cnt] = *data;
        data++;
    }

    gioSetBit(rx_cs_port, rx_cs_pin, 0);

    spiTransmitData(rx_port, &dataconfig1_t, 1, &reg_data);
    spiTransmitData(rx_port, &dataconfig1_t, length, &instruction_data[0]);

    gioSetBit(rx_cs_port, rx_cs_pin,   1);


}
//
//
void  tx_module_reset(void)
{
    gioSetBit(general_port, tx_reset_pin, 0);
    delay_ms(2);
    gioSetBit(general_port, tx_reset_pin, 1);
    delay_ms(100);
}
//
void  tx_vSpiWrite(uint8 reg, uint8 data)
{
     spiDAT1_t dataconfig1_t;
     uint16 spi_data;

     dataconfig1_t.CS_HOLD = FALSE;
     dataconfig1_t.WDEL = TRUE;
     dataconfig1_t.DFSEL = SPI_FMT_0;
     dataconfig1_t.CSNR = 0;

     spi_data = ((reg << 8) | 0x8000) | data;
     gioSetBit(tx_cs_port, tx_cs_pin,   0);
     spiTransmitData(tx_port, &dataconfig1_t, 1, &spi_data);
     gioSetBit(tx_cs_port, tx_cs_pin,   1);

}
//
uint8 tx_bSpiRead(uint8 reg)
{
    spiDAT1_t dataconfig1_t;
    uint16 spi_data;
    uint16 data;

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL = TRUE;
    dataconfig1_t.DFSEL = SPI_FMT_0;
    dataconfig1_t.CSNR = 0;

    spi_data = (reg << 8);
    gioSetBit(tx_cs_port, tx_cs_pin,   0);
    spiTransmitAndReceiveData(tx_port, &dataconfig1_t, 1, &spi_data, &data);
    gioSetBit(tx_cs_port, tx_cs_pin,   1);
    return (data & 0xFF);
}
//
void  tx_ReadBuffer(uint8 reg,uint8 length, uint8_t *data)
{
    spiDAT1_t dataconfig1_t;
    uint16    reg_data;
    uint16    rx_data[255];
    uint8     cnt;

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL    = TRUE;
    dataconfig1_t.DFSEL   = SPI_FMT_1;
    dataconfig1_t.CSNR    = 0;

    reg_data = reg;

    gioSetBit(tx_cs_port, tx_cs_pin, 0);
    spiTransmitData(tx_port, &dataconfig1_t, 1, &reg_data);
    spiReceiveData(tx_port, &dataconfig1_t, length,  &rx_data[0]);
    gioSetBit(tx_cs_port, tx_cs_pin, 1);
    for(cnt=0;cnt<length;cnt++)
     {
         *data = rx_data[cnt];
         data++;
     }

}
//
void  tx_WriteBuffer(uint8 reg,uint8 length, uint8_t *data)
{
    spiDAT1_t dataconfig1_t;
    uint16 reg_data;
    uint16 instruction_data[255];
    uint8 cnt;

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL    = TRUE;
    dataconfig1_t.DFSEL   = SPI_FMT_1;
    dataconfig1_t.CSNR    = 0;

    reg_data = reg | 0x80;
    for(cnt =0;cnt<length;cnt++)
    {
        instruction_data[cnt] = *data;
        data++;
    }

    gioSetBit(tx_cs_port, tx_cs_pin, 0);

    spiTransmitData(tx_port, &dataconfig1_t, 1, &reg_data);
    spiTransmitData(tx_port, &dataconfig1_t, length, &instruction_data[0]);

    gioSetBit(tx_cs_port, tx_cs_pin,   1);
}
//
