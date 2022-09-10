/*
 * sx1255.c
 *
 *  Created on: 2021年7月13日
 *      Author: admin
 */

#include "sx1255.h"
#include "sys_config.h"
#include "pin_config.h"
#include "print.h"
#include "sx1301_config.h"
#include "sx1301_lora.h"
#include "glib.h"

uint8_t sx125x_read(uint8_t channel, uint8_t addr) {
    int reg_add, reg_dat, reg_cs, reg_rb;
    int32_t read_value;

    /* checking input parameters */
    if (channel >= 2) {
        print("ERROR: INVALID RF_CHAIN\n");
        return 0;
    }
    if (addr >= 0x7F) {
        print("ERROR: ADDRESS OUT OF RANGE\n");
        return 0;
    }

    /* selecting the target radio */
    switch (channel) {
        case 0:
            reg_add = LGW_SPI_RADIO_A__ADDR;
            reg_dat = LGW_SPI_RADIO_A__DATA;
            reg_cs  = LGW_SPI_RADIO_A__CS;
            reg_rb  = LGW_SPI_RADIO_A__DATA_READBACK;
            break;

        case 1:
            reg_add = LGW_SPI_RADIO_B__ADDR;
            reg_dat = LGW_SPI_RADIO_B__DATA;
            reg_cs  = LGW_SPI_RADIO_B__CS;
            reg_rb  = LGW_SPI_RADIO_B__DATA_READBACK;
            break;

        default:
            print("ERROR: UNEXPECTED VALUE %d IN SWITCH STATEMENT\n", channel);
            return 0;
    }

    /* SPI master data read procedure */
    lgw_reg_w(reg_cs, 0);
    lgw_reg_w(reg_add, addr); /* MSB at 0 for read operation */
    lgw_reg_w(reg_dat, 0);
    lgw_reg_w(reg_cs, 1);
    lgw_reg_w(reg_cs, 0);
    lgw_reg_r(reg_rb, &read_value);

    return (uint8_t)read_value;
}
void sx125x_write(uint8_t channel, uint8_t addr, uint8_t data) {
    int reg_add, reg_dat, reg_cs;

    /* checking input parameters */
    if (channel >= 2) {
        print("ERROR: INVALID RF_CHAIN\n");
        return;
    }
    if (addr >= 0x7F) {
        print("ERROR: ADDRESS OUT OF RANGE\n");
        return;
    }

    /* selecting the target radio */
    switch (channel) {
        case 0:
            reg_add = LGW_SPI_RADIO_A__ADDR;
            reg_dat = LGW_SPI_RADIO_A__DATA;
            reg_cs  = LGW_SPI_RADIO_A__CS;
            break;

        case 1:
            reg_add = LGW_SPI_RADIO_B__ADDR;
            reg_dat = LGW_SPI_RADIO_B__DATA;
            reg_cs  = LGW_SPI_RADIO_B__CS;
            break;

        default:
            print("ERROR: UNEXPECTED VALUE %d IN SWITCH STATEMENT\n", channel);
            return;
    }

    /* SPI master data write procedure */
    lgw_reg_w(reg_cs, 0);
    lgw_reg_w(reg_add, 0x80 | addr); /* MSB at 1 for write operation */
    lgw_reg_w(reg_dat, data);
    lgw_reg_w(reg_cs, 1);
    lgw_reg_w(reg_cs, 0);

    return;
}

int lgw_setup_sx125x(uint8_t rf_chain, uint8_t rf_clkout, bool rf_enable, uint8_t rf_radio_type, uint32_t freq_hz) {
    uint32_t part_int = 0;
    uint32_t part_frac = 0;
    int cpt_attempts = 0;

    if (rf_chain >= 2) {
        print("ERROR: INVALID RF_CHAIN\n");
        return -1;
    }

    /* General radio setup */
    if (rf_clkout == rf_chain) {
        sx125x_write(rf_chain, 0x10, SX125x_TX_DAC_CLK_SEL + 2);
//        print("SX125x %d clock output enabled\n", rf_chain);
    } else {
        sx125x_write(rf_chain, 0x10, SX125x_TX_DAC_CLK_SEL);
//        print("SX125x %d clock output disabled\n", rf_chain);
    }

    switch (rf_radio_type) {
        case LGW_RADIO_TYPE_SX1255:
            sx125x_write(rf_chain, 0x28, SX125x_XOSC_GM_STARTUP + SX125x_XOSC_DISABLE*16);
            break;
        case LGW_RADIO_TYPE_SX1257:
            sx125x_write(rf_chain, 0x26, SX125x_XOSC_GM_STARTUP + SX125x_XOSC_DISABLE*16);
            break;
        default:
            print("ERROR: UNEXPECTED VALUE %d FOR RADIO TYPE\n", rf_radio_type);
            break;
    }

    if (rf_enable == true) {
        /* Tx gain and trim */
        sx125x_write(rf_chain, 0x08, SX125x_TX_MIX_GAIN + SX125x_TX_DAC_GAIN*16);
        sx125x_write(rf_chain, 0x0A, SX125x_TX_ANA_BW + SX125x_TX_PLL_BW*32);
        sx125x_write(rf_chain, 0x0B, SX125x_TX_DAC_BW);

        /* Rx gain and trim */
        sx125x_write(rf_chain, 0x0C, SX125x_LNA_ZIN + SX125x_RX_BB_GAIN*2 + SX125x_RX_LNA_GAIN*32);
        sx125x_write(rf_chain, 0x0D, SX125x_RX_BB_BW + SX125x_RX_ADC_TRIM*4 + SX125x_RX_ADC_BW*32);
        sx125x_write(rf_chain, 0x0E, SX125x_ADC_TEMP + SX125x_RX_PLL_BW*2);

        /* set RX PLL frequency */
        switch (rf_radio_type) {
            case LGW_RADIO_TYPE_SX1255:
                part_int = freq_hz / (SX125x_32MHz_FRAC << 7); /* integer part, gives the MSB */
                part_frac = ((freq_hz % (SX125x_32MHz_FRAC << 7)) << 9) / SX125x_32MHz_FRAC; /* fractional part, gives middle part and LSB */
                break;
            case LGW_RADIO_TYPE_SX1257:
                part_int = freq_hz / (SX125x_32MHz_FRAC << 8); /* integer part, gives the MSB */
                part_frac = ((freq_hz % (SX125x_32MHz_FRAC << 8)) << 8) / SX125x_32MHz_FRAC; /* fractional part, gives middle part and LSB */
                break;
            default:
                print("ERROR: UNEXPECTED VALUE %d FOR RADIO TYPE\n", rf_radio_type);
                break;
        }

        sx125x_write(rf_chain, 0x01,0xFF & part_int); /* Most Significant Byte */
        sx125x_write(rf_chain, 0x02,0xFF & (part_frac >> 8)); /* middle byte */
        sx125x_write(rf_chain, 0x03,0xFF & part_frac); /* Least Significant Byte */

        /* start and PLL lock */
        do {
            if (cpt_attempts >= PLL_LOCK_MAX_ATTEMPTS) {
                print("ERROR: FAIL TO LOCK PLL\n");
                return -1;
            }
            sx125x_write(rf_chain, 0x00, 1); /* enable Xtal oscillator */
            sx125x_write(rf_chain, 0x00, 3); /* Enable RX (PLL+FE) */
            ++cpt_attempts;
//            print("Note: SX125x #%d PLL start (attempt %d)\n", rf_chain, cpt_attempts);
            Delay(1);//TODO 1ms
        } while((sx125x_read(rf_chain, 0x11) & 0x02) == 0);
    } else {
        print("Note: SX125x #%d kept in standby mode\n", rf_chain);
    }

    return 0;
}

void sx1255_read_all(uint8_t rf_chain, uint8 *data){
    int i = 0;
    for( i = 0; i < 20; i++){
        data[i] = sx125x_read(rf_chain, i);
    }
}
