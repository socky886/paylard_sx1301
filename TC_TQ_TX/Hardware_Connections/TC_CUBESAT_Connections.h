/*
 * MCU_CONNECTIONS.h
 *
 *  Created on: 2019Äê2ÔÂ11ÈÕ
 *      Author: houzh
 */

#ifndef HARDWARE_CONNECTIONS_TC_CUBESAT_CONNECTIONS_H_
#define HARDWARE_CONNECTIONS_TC_CUBESAT_CONNECTIONS_H_

#include "reg_gio.h"
#include "gio.h"
#include "reg_het.h"
#include "het.h"
#include "reg_spi.h"
#include "spi.h"
//
//SWITCHIO 0~7
#define  switchio_port      gioPORTA
#define  switch_io_0            0                         //output
#define  switch_io_1            1                         //output
#define  switch_io_2            2                         //output
#define  switch_io_3            3                         //output
#define  switch_io_4            4                         //output
#define  switch_io_5            5                         //output
#define  switch_io_6            6                         //output
#define  switch_io_7            7                         //output
//SWITCHIO 8,9,lna_power_pin, pa_power_pin
//rx2_reset_pin, rx2_init_pin
//tx1_reset_pin, tx2_init_pin
#define  general_port        hetPORT1
#define  switch_io_8            0                         //output
#define  switch_io_9            2                         //output
#define  tx_reset_pin           4                         //output
#define  tx_init_pin            6                         //input
#define  rx_init_pin            8                         //input
#define  mram_write_p_pin       10                        //output
#define  mram_cs_pin            12                        //output
#define  rs_de_pin              14                        //output
#define  rs_re_pin              16                        //output
#define  bb_power_pin           17                        //output

#define  cpu_buringing_io2      18                        //output
#define  Heart_Beat             19
#define  reset_output           22
#define  rx_reset_pin           24                        //output
#define  cpu_buringing_io1      26

#define  pa_power_pin           28                        //output

#define  reset_io2              30
//spi port
#define  rx_port              spiREG1
#define  rx_cs_port           spiPORT1
#define  rx_cs_pin              0                        //

#define  fram_port             spiREG2

#define  tx_port              spiREG3
#define  tx_cs_port           spiPORT3
#define  tx_cs_pin              0                        //


// frame instructions
#define  WREN                   0x06
#define  WRDI                   0x04
#define  RSDR                   0x05
#define  WRSR                   0x01
#define  READ                   0x03

#define  WRITE                  0x02
#define  SLEEP                  0xB9
#define  WAKE                   0xAB

//function definitions
//
void fram_enable(void);
void fram_disable(void);
void fram_hardware_wp_enable(void);
void fram_hardware_wp_disable(void);
void fram_write_enable(void);
uint8 fram_read_status(void);

//void fram_status_read(void);
//void fram_id_read(void);
//void fram_status_read(void);
//void fram_data_read(uint32 addr, uint32 length, uint8 *data);
//void fram_data_write(uint32 addr, uint32 length, uint8 *data);




//
void  switch0_on(void);
void  switch0_off(void);
void  switch1_on(void);
void  switch1_off(void);
void  switch2_on(void);
void  switch2_off(void);
void  switch3_on(void);
void  switch3_off(void);
void  switch4_on(void);
void  switch4_off(void);
void  switch5_on(void);
void  switch5_off(void);
void  switch6_on(void);
void  switch6_off(void);
void  switch7_on(void);
void  switch7_off(void);
void  switch7_on(void);
void  switch7_off(void);
void  switch8_on(void);
void  switch8_off(void);
void  switch9_on(void);
void  switch9_off(void);
void  switch_all_on(void);
void  switch_all_off(void);


void  burning_wire1_on(void);
void  burning_wire1_off(void);
void  burning_wire2_on(void);
void  burning_wire2_off(void);

void  bb_poweron(void);
void  bb_poweroff(void);

void  pa_poweron(void);
void  pa_poweroff(void);

void  uart_shutdonw_mode(void);
void  uart_transceiver_mode(void);
void  uart_transmitter_mode();
void  uart_receiver_mode();

void  rx_module_reset(void);
void  rx_vSpiWrite(uint8 reg, uint8 data);
uint8 rx_bSpiRead(uint8 reg);
void  rx_ReadBuffer(uint8 reg,uint8 length, uint8_t *data);
void  rx_WriteBuffer(uint8 reg,uint8 length, uint8_t *data);

void  tx_module_reset(void);
void  tx_vSpiWrite(uint8 reg, uint8 data);
uint8 tx_bSpiRead(uint8 reg);
void  tx_ReadBuffer(uint8 reg,uint8 length, uint8_t *data);
void  tx_WriteBuffer(uint8 reg,uint8 length, uint8_t *data);





#endif /* HARDWARE_CONNECTIONS_TC_CUBESAT_CONNECTIONS_H_ */
