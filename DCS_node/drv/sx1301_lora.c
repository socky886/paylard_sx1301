/*
 * sx1301_lora.c
 *
 *  Created on: 2021年7月6日
 *      Author: admin
 */

#include "sx1301_lora.h"
#include "sys_config.h"
#include "rti.h"
#include "pin_config.h"
#include "interrupt.h"
#include "sx1255.h"
#include "sx1301_config.h"
#include "print.h"
#include "glib.h"
#include "math.h"
#include "glib.h"

#define TX_METADATA_NB      16
#define RX_METADATA_NB      16

#define DEFAULT_RSSI_OFFSET 0.0
#define DEFAULT_NOTCH_FREQ  129000U

#define LGW_MIN_NOTCH_FREQ      126000U /* 126 KHz */
#define LGW_MAX_NOTCH_FREQ      250000U /* 250 KHz */
#define LGW_DEFAULT_NOTCH_FREQ  129000U /* 129 KHz */

#define RSSI_MULTI_BIAS     -35 /* difference between "multi" modem RSSI offset and "stand-alone" modem RSSI offset */
#define RSSI_FSK_POLY_0     60 /* polynomiam coefficients to linearize FSK RSSI */
#define RSSI_FSK_POLY_1     1.5351
#define RSSI_FSK_POLY_2     0.003

#define TX_START_DELAY_DEFAULT  1497 /* Calibrated value for 500KHz BW and notch filter disabled */

static int lgw_regpage = -1; /*! keep the value of the register page selected */


//uint8 reg[20];

static uint8_t rf_clkout = 1;//0 is Radio A;1 is Radio B;

static bool rf_enable[2];
static uint32_t rf_rx_freq[2]; /* absolute, in Hz */
static float rf_rssi_offset[2];
static bool rf_tx_enable[2];
static enum lgw_radio_type_e rf_radio_type[2];

static bool if_enable[10];
static bool if_rf_chain[10]; /* for each IF, 0 -> radio A, 1 -> radio B */
static int32_t if_freq[10]; /* relative to radio frequency, +/- in Hz */

static uint8_t lora_rx_bw; /* bandwidth setting for LoRa standalone modem */
static uint8_t lora_rx_sf; /* spreading factor setting for LoRa standalone modem */
static bool lora_rx_ppm_offset;
static uint8_t lora_multi_sfmask[8]; /* enables SF for LoRa 'multi' modems */

static uint8_t fsk_rx_bw; /* bandwidth setting of FSK modem */
static uint32_t fsk_rx_dr; /* FSK modem datarate in bauds */
static uint8_t fsk_sync_word_size = 1; /* default number of bytes for FSK sync word */
static uint64_t fsk_sync_word= 0x34; /* default FSK sync word (ALIGNED RIGHT, MSbit first) */
/* constant arrays defining hardware capability */

static bool lorawan_public = TRUE;

const uint8_t ifmod_config[10] = LGW_IFMODEM_CONFIG;

struct lgw_conf_rxrf_s rfconf;
struct lgw_conf_rxif_s ifconf;

uint8 lgw_is_started = FALSE;


extern const uint16_t cal_firmware[MCU_AGC_FW_BYTE];
extern const uint16_t arb_firmware[MCU_ARB_FW_BYTE] ;
extern const uint16_t agc_firmware[MCU_AGC_FW_BYTE];

struct lgw_pkt_tx_s txpkt; /* configuration and metadata for an outbound packet */

/* TX gain LUT table */
static struct lgw_tx_gain_lut_s txgain_lut = {
                                              .size = 5,
                                              .lut[0] = {
                                                  .dig_gain = 0,
                                                  .pa_gain = 0,
                                                  .dac_gain = 3,
                                                  .mix_gain = 12,
                                                  .rf_power = 0
                                              },
                                              .lut[1] = {
                                                  .dig_gain = 0,
                                                  .pa_gain = 1,
                                                  .dac_gain = 3,
                                                  .mix_gain = 12,
                                                  .rf_power = 10
                                              },
                                              .lut[2] = {
                                                  .dig_gain = 0,
                                                  .pa_gain = 2,
                                                  .dac_gain = 3,
                                                  .mix_gain = 10,
                                                  .rf_power = 14
                                              },
                                              .lut[3] = {
                                                  .dig_gain = 0,
                                                  .pa_gain = 3,
                                                  .dac_gain = 3,
                                                  .mix_gain = 9,
                                                  .rf_power = 20
                                              },
                                              .lut[4] = {
                                                  .dig_gain = 0,
                                                  .pa_gain = 3,
                                                  .dac_gain = 3,
                                                  .mix_gain = 14,
                                                  .rf_power = 27
                                              }};
/* TX I/Q imbalance coefficients for mixer gain = 8 to 15 */
static int8_t cal_offset_a_i[8]; /* TX I offset for radio A */
static int8_t cal_offset_a_q[8]; /* TX Q offset for radio A */
static int8_t cal_offset_b_i[8]; /* TX I offset for radio B */
static int8_t cal_offset_b_q[8]; /* TX Q offset for radio B */

const static RF_SPI_PIN_CONFIG_TYPE rf_pin = {
    spiREG4, spiPORT4, SPI_PIN_CS0,  gioPORTA,  5
};


static void _delay_us(uint32 us)
{
    volatile uint32 time = us;
    while(time--) __nop();
}

static void wait_ms(uint32 ms)
{
    while(ms > 50){
        FeedWatchDog();
        _delay_us(50 * 1000);
        ms-=50;

    }
    _delay_us(ms * 1000);

}

static void sxResetPin(uint8 level) {
    gioSetBit(rf_pin.rstport, rf_pin.reset, level);
}

/*********************************初始化配置***************************************/

static int32_t lgw_bw_getval(int x) {
    switch (x) {
        case BW_500KHZ: return 500000;
        case BW_250KHZ: return 250000;
        case BW_125KHZ: return 125000;
        case BW_62K5HZ: return 62500;
        case BW_31K2HZ: return 31200;
        case BW_15K6HZ: return 15600;
        case BW_7K8HZ : return 7800;
        default: return -1;
    }
}
//to set the configuration of the IF+modem channels
static uint8 lgw_rxif_setconf(uint8_t if_chain, struct lgw_conf_rxif_s conf) {
    int32_t bw_hz;
    uint32_t rf_rx_bandwidth;

    /* check if the concentrator is running */
    if (lgw_is_started == true) {
        print("ERROR: CONCENTRATOR IS RUNNING, STOP IT BEFORE TOUCHING CONFIGURATION\n");
        return FALSE;
    }

    /* check input range (segfault prevention) */
    if (if_chain >= 10) {
        print("ERROR: %d NOT A VALID IF_CHAIN NUMBER\n", if_chain);
        return FALSE;
    }

    /* if chain is disabled, don't care about most parameters */
    if (conf.enable == false) {
        if_enable[if_chain] = false;
        if_freq[if_chain] = 0;
        print("Note: if_chain %d disabled\n", if_chain);
        return TRUE;
    }

    /* check 'general' parameters */
    if (ifmod_config[if_chain] == IF_UNDEFINED) {
        print("ERROR: IF CHAIN %d NOT CONFIGURABLE\n", if_chain);
    }
    if (conf.rf_chain >= 2) {
        print("ERROR: INVALID RF_CHAIN TO ASSOCIATE WITH A LORA_STD IF CHAIN\n");
        return FALSE;
    }
    /* check if IF frequency is optimal based on channel and radio bandwidths */
    switch (conf.bandwidth) {
        case BW_250KHZ:
            rf_rx_bandwidth = LGW_RF_RX_BANDWIDTH_250KHZ; /* radio bandwidth */
            break;
        case BW_500KHZ:
            rf_rx_bandwidth = LGW_RF_RX_BANDWIDTH_500KHZ; /* radio bandwidth */
            break;
        default:
            /* For 125KHz and below */
            rf_rx_bandwidth = LGW_RF_RX_BANDWIDTH_125KHZ; /* radio bandwidth */
            break;
    }
    bw_hz = lgw_bw_getval(conf.bandwidth); /* channel bandwidth */
    if ((conf.freq_hz + ((bw_hz==-1)?LGW_REF_BW:bw_hz)/2) > ((int32_t)rf_rx_bandwidth/2)) {
        print("ERROR: IF FREQUENCY %d TOO HIGH\n", conf.freq_hz);
        return FALSE;
    } else if ((conf.freq_hz - ((bw_hz==-1)?LGW_REF_BW:bw_hz)/2) < -((int32_t)rf_rx_bandwidth/2)) {
        print("ERROR: IF FREQUENCY %d TOO LOW\n", conf.freq_hz);
        return FALSE;
    }

    /* check parameters according to the type of IF chain + modem,
    fill default if necessary, and commit configuration if everything is OK */
    switch (ifmod_config[if_chain]) {
        case IF_LORA_STD:
            /* fill default parameters if needed */
            if (conf.bandwidth == BW_UNDEFINED) {
                conf.bandwidth = BW_250KHZ;
            }
            if (conf.datarate == DR_UNDEFINED) {
                conf.datarate = DR_LORA_SF9;
            }
            /* check BW & DR */
            if (!IS_LORA_BW(conf.bandwidth)) {
                print("ERROR: BANDWIDTH NOT SUPPORTED BY LORA_STD IF CHAIN\n");
                return FALSE;
            }
            if (!IS_LORA_STD_DR(conf.datarate)) {
                print("ERROR: DATARATE NOT SUPPORTED BY LORA_STD IF CHAIN\n");
                return FALSE;
            }
            /* set internal configuration  */
            if_enable[if_chain] = conf.enable;
            if_rf_chain[if_chain] = conf.rf_chain;
            if_freq[if_chain] = conf.freq_hz;
            lora_rx_bw = conf.bandwidth;
            lora_rx_sf = (uint8_t)(DR_LORA_MULTI & conf.datarate); /* filter SF out of the 7-12 range */
            if (SET_PPM_ON(conf.bandwidth, conf.datarate)) {
                lora_rx_ppm_offset = true;
            } else {
                lora_rx_ppm_offset = false;
            }

//            print("Note: LoRa 'std' if_chain %d configuration; en:%d freq:%d bw:%d dr:%d\n", if_chain, if_enable[if_chain], if_freq[if_chain], lora_rx_bw, lora_rx_sf);
            break;

        case IF_LORA_MULTI:
            /* fill default parameters if needed */
            if (conf.bandwidth == BW_UNDEFINED) {
                conf.bandwidth = BW_125KHZ;
            }
            if (conf.datarate == DR_UNDEFINED) {
                conf.datarate = DR_LORA_MULTI;
            }
            /* check BW & DR */
            if (conf.bandwidth != BW_125KHZ) {
                print("ERROR: BANDWIDTH NOT SUPPORTED BY LORA_MULTI IF CHAIN\n");
                return FALSE;
            }
            if (!IS_LORA_MULTI_DR(conf.datarate)) {
                print("ERROR: DATARATE(S) NOT SUPPORTED BY LORA_MULTI IF CHAIN\n");
                return FALSE;
            }
            /* set internal configuration  */
            if_enable[if_chain] = conf.enable;
            if_rf_chain[if_chain] = conf.rf_chain;
            if_freq[if_chain] = conf.freq_hz;
            lora_multi_sfmask[if_chain] = (uint8_t)(DR_LORA_MULTI & conf.datarate); /* filter SF out of the 7-12 range */

//            print("Note: LoRa 'multi' if_chain %d configuration; en:%d freq:%d SF_mask:0x%02x\n", if_chain, if_enable[if_chain], if_freq[if_chain], lora_multi_sfmask[if_chain]);
            break;

        case IF_FSK_STD:
            /* fill default parameters if needed */
            if (conf.bandwidth == BW_UNDEFINED) {
                conf.bandwidth = BW_250KHZ;
            }
            if (conf.datarate == DR_UNDEFINED) {
                conf.datarate = 64000; /* default datarate */
            }
            /* check BW & DR */
            if(!IS_FSK_BW(conf.bandwidth)) {
                print("ERROR: BANDWIDTH NOT SUPPORTED BY FSK IF CHAIN\n");
                return FALSE;
            }
            if(!IS_FSK_DR(conf.datarate)) {
                print("ERROR: DATARATE NOT SUPPORTED BY FSK IF CHAIN\n");
                return FALSE;
            }
            /* set internal configuration  */
            if_enable[if_chain] = conf.enable;
            if_rf_chain[if_chain] = conf.rf_chain;
            if_freq[if_chain] = conf.freq_hz;
            fsk_rx_bw = conf.bandwidth;
            fsk_rx_dr = conf.datarate;
            if (conf.sync_word > 0) {
                fsk_sync_word_size = conf.sync_word_size;
                fsk_sync_word = conf.sync_word;
            }
//            print("Note: FSK if_chain %d configuration; en:%d freq:%d bw:%d dr:%d (%d real dr) sync:0x%0*llX\n", if_chain, if_enable[if_chain], if_freq[if_chain], fsk_rx_bw, fsk_rx_dr, LGW_XTAL_FREQU/(LGW_XTAL_FREQU/fsk_rx_dr), 2*fsk_sync_word_size, fsk_sync_word);
            break;

        default:
            print("ERROR: IF CHAIN %d TYPE NOT SUPPORTED\n", if_chain);
            return FALSE;
    }

    return TRUE;
}

//to set the configuration of the radio channels
static uint8 lgw_rxrf_setconf(uint8_t rf_chain, struct lgw_conf_rxrf_s conf)
{
    /* check if the concentrator is running */
    if (lgw_is_started == true) {
        print("ERROR: CONCENTRATOR IS RUNNING, STOP IT BEFORE TOUCHING CONFIGURATION\n");
        return FALSE;
    }
    /* check input range (segfault prevention) */
    if (rf_chain >= 2) {
        print("ERROR: NOT A VALID RF_CHAIN NUMBER\n");
        return FALSE;
    }
    /* check if radio type is supported */
    if ((conf.type != LGW_RADIO_TYPE_SX1255) && (conf.type != LGW_RADIO_TYPE_SX1257)) {
        print("ERROR: NOT A VALID RADIO TYPE\n");
        return FALSE;
    }
    /* check if TX notch filter frequency is supported */
    if ((conf.tx_enable == true) && ((conf.tx_notch_freq < LGW_MIN_NOTCH_FREQ) || (conf.tx_notch_freq > LGW_MAX_NOTCH_FREQ))) {
        print("WARNING: NOT A VALID TX NOTCH FILTER FREQUENCY [%u..%u]Hz\n", LGW_MIN_NOTCH_FREQ, LGW_MAX_NOTCH_FREQ);
        conf.tx_notch_freq = 0;
    }
    /* set internal config according to parameters */
    rf_enable[rf_chain] = conf.enable;
    rf_rx_freq[rf_chain] = conf.freq_hz;
    rf_rssi_offset[rf_chain] = conf.rssi_offset;
    rf_radio_type[rf_chain] = conf.type;
    rf_tx_enable[rf_chain] = conf.tx_enable;

//    print("Note: rf_chain %d configuration; en:%d freq:%d rssi_offset:%.3f radio_type:%d tx_enable:%d tx_notch_freq:%u\n", rf_chain, rf_enable[rf_chain], rf_rx_freq[rf_chain], rf_rssi_offset[rf_chain], rf_radio_type[rf_chain], rf_tx_enable[rf_chain], rf_tx_notch_freq[rf_chain]);

    return TRUE;
}

/* set configuration for RF chains */
static void rxrf_init(void)
{
    //TODO 频点初始化

#if !DCS_CPU
    uint32_t fa = 385000000;//Radio A RX frequency,rf_chain 0
    uint32_t fb = 384400000;//Radio B RX frequency,rf_chain 1
#else
    uint32_t fa = 503600000;//Radio A RX frequency,rf_chain 0
    uint32_t fb = 503000000;//Radio B RX frequency,rf_chain 1
#endif

    memset(&rfconf, 0, sizeof(rfconf));

    rfconf.enable = true;
    rfconf.freq_hz = fa;
    rfconf.rssi_offset = DEFAULT_RSSI_OFFSET;
    rfconf.type = LGW_RADIO_TYPE_SX1255;
    rfconf.tx_enable = true;
    rfconf.tx_notch_freq = DEFAULT_NOTCH_FREQ;
    lgw_rxrf_setconf(0, rfconf); /* radio A, f0 */

    rfconf.enable = true;
    rfconf.freq_hz = fb;
    rfconf.rssi_offset = DEFAULT_RSSI_OFFSET;
    rfconf.type = LGW_RADIO_TYPE_SX1255;
    rfconf.tx_enable = false;
    lgw_rxrf_setconf(1, rfconf); /* radio B, f1 */

}

/* set configuration for LoRa multi-SF channels (bandwidth cannot be set) */
static void rxif_init(void)
{
    memset(&ifconf, 0, sizeof(ifconf));
    ifconf.enable = true;
    ifconf.rf_chain = 1;
    ifconf.freq_hz = -400000;
    ifconf.datarate = DR_LORA_MULTI;
    lgw_rxif_setconf(0, ifconf); /* chain 0: LoRa 125kHz, all SF, on f1 - 0.4 MHz */


    ifconf.enable = true;
    ifconf.rf_chain = 1;
    ifconf.freq_hz = -200000;
    ifconf.datarate = DR_LORA_MULTI;
    lgw_rxif_setconf(1, ifconf); /* chain 1: LoRa 125kHz, all SF, on f1 - 0.2 MHz */

    ifconf.enable = true;
    ifconf.rf_chain = 1;
    ifconf.freq_hz = 0;
    ifconf.datarate = DR_LORA_MULTI;
    lgw_rxif_setconf(2, ifconf); /* chain 2: LoRa 125kHz, all SF, on f1 - 0.0 MHz */

    ifconf.enable = true;
    ifconf.rf_chain = 0;
    ifconf.freq_hz = -400000;
    ifconf.datarate = DR_LORA_MULTI;
    lgw_rxif_setconf(3, ifconf); /* chain 3: LoRa 125kHz, all SF, on f0 - 0.4 MHz */

    ifconf.enable = true;
    ifconf.rf_chain = 0;
    ifconf.freq_hz = -200000;
    ifconf.datarate = DR_LORA_MULTI;
    lgw_rxif_setconf(4, ifconf); /* chain 4: LoRa 125kHz, all SF, on f0 - 0.2 MHz */

    ifconf.enable = true;
    ifconf.rf_chain = 0;
    ifconf.freq_hz = 0;
    ifconf.datarate = DR_LORA_MULTI;
    lgw_rxif_setconf(5, ifconf); /* chain 5: LoRa 125kHz, all SF, on f0 + 0.0 MHz */

    ifconf.enable = true;
    ifconf.rf_chain = 0;
    ifconf.freq_hz = 200000;
    ifconf.datarate = DR_LORA_MULTI;
    lgw_rxif_setconf(6, ifconf); /* chain 6: LoRa 125kHz, all SF, on f0 + 0.2 MHz */

    ifconf.enable = true;
    ifconf.rf_chain = 0;
    ifconf.freq_hz = 400000;
    ifconf.datarate = DR_LORA_MULTI;
    lgw_rxif_setconf(7, ifconf); /* chain 7: LoRa 125kHz, all SF, on f0 + 0.4 MHz */

    /* set configuration for LoRa 'stand alone' channel */
    memset(&ifconf, 0, sizeof(ifconf));
    ifconf.enable = true;
    ifconf.rf_chain = 0;
    ifconf.freq_hz = 0;
    ifconf.bandwidth = BW_250KHZ;
    ifconf.datarate = DR_LORA_SF12;
    lgw_rxif_setconf(8, ifconf); /* chain 8: LoRa 250kHz, SF12, on f1 MHz */

    /* set configuration for FSK channel */
   /* memset(&ifconf, 0, sizeof(ifconf));
    ifconf.enable = true;
    ifconf.rf_chain = 0;
    ifconf.freq_hz = 0;
    ifconf.bandwidth = BW_250KHZ;
    ifconf.datarate = 1200;
    lgw_rxif_setconf(9, ifconf); *//* chain 9: FSK 64kbps, on f1 MHz */
}
/* set configuration for TX packet */
static void tx_init(void)
{
//    int i = 0;
    //TODO
    uint32_t ft = 399000000;//Radio TX frequency

    memset(&txpkt, 0, sizeof(txpkt));
    txpkt.freq_hz = ft;
    txpkt.tx_mode = IMMEDIATE;
    txpkt.rf_power = 27;
    txpkt.modulation = MOD_LORA;
    txpkt.bandwidth = BW_125KHZ;
    txpkt.datarate = DR_LORA_SF12;
    txpkt.coderate = CR_LORA_4_5;
    txpkt.invert_pol = 0;

    txpkt.no_crc = FALSE;
    txpkt.no_header = FALSE;
    txpkt.size = 20;
    txpkt.preamble = 8;
    txpkt.rf_chain = 0;

}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void lgw_constant_adjust(void) {

    /* I/Q path setup */

    //lgw_reg_w(LGW_RX_INVERT_IQ,0); /* default 0 */
    //lgw_reg_w(LGW_MODEM_INVERT_IQ,1); /* default 1 */
    //lgw_reg_w(LGW_CHIRP_INVERT_RX,1); /* default 1 */
    //lgw_reg_w(LGW_RX_EDGE_SELECT,0); /* default 0 */
    //lgw_reg_w(LGW_MBWSSF_MODEM_INVERT_IQ,0); /* default 0 */
    //lgw_reg_w(LGW_DC_NOTCH_EN,1); /* default 1 */
    lgw_reg_w(LGW_RSSI_BB_FILTER_ALPHA,6); /* default 7 */
    lgw_reg_w(LGW_RSSI_DEC_FILTER_ALPHA,7); /* default 5 */
    lgw_reg_w(LGW_RSSI_CHANN_FILTER_ALPHA,7); /* default 8 */
    lgw_reg_w(LGW_RSSI_BB_DEFAULT_VALUE,23); /* default 32 */
    lgw_reg_w(LGW_RSSI_CHANN_DEFAULT_VALUE,85); /* default 100 */
    lgw_reg_w(LGW_RSSI_DEC_DEFAULT_VALUE,66); /* default 100 */
    lgw_reg_w(LGW_DEC_GAIN_OFFSET,7); /* default 8 */
    lgw_reg_w(LGW_CHAN_GAIN_OFFSET,6); /* default 7 */

    /* Correlator setup */
    // lgw_reg_w(LGW_CORR_DETECT_EN,126); /* default 126 */
    // lgw_reg_w(LGW_CORR_NUM_SAME_PEAK,4); /* default 4 */
    // lgw_reg_w(LGW_CORR_MAC_GAIN,5); /* default 5 */
    // lgw_reg_w(LGW_CORR_SAME_PEAKS_OPTION_SF6,0); /* default 0 */
    // lgw_reg_w(LGW_CORR_SAME_PEAKS_OPTION_SF7,1); /* default 1 */
    // lgw_reg_w(LGW_CORR_SAME_PEAKS_OPTION_SF8,1); /* default 1 */
    // lgw_reg_w(LGW_CORR_SAME_PEAKS_OPTION_SF9,1); /* default 1 */
    // lgw_reg_w(LGW_CORR_SAME_PEAKS_OPTION_SF10,1); /* default 1 */
    // lgw_reg_w(LGW_CORR_SAME_PEAKS_OPTION_SF11,1); /* default 1 */
    // lgw_reg_w(LGW_CORR_SAME_PEAKS_OPTION_SF12,1); /* default 1 */
    // lgw_reg_w(LGW_CORR_SIG_NOISE_RATIO_SF6,4); /* default 4 */
    // lgw_reg_w(LGW_CORR_SIG_NOISE_RATIO_SF7,4); /* default 4 */
    // lgw_reg_w(LGW_CORR_SIG_NOISE_RATIO_SF8,4); /* default 4 */
    // lgw_reg_w(LGW_CORR_SIG_NOISE_RATIO_SF9,4); /* default 4 */
    // lgw_reg_w(LGW_CORR_SIG_NOISE_RATIO_SF10,4); /* default 4 */
    // lgw_reg_w(LGW_CORR_SIG_NOISE_RATIO_SF11,4); /* default 4 */
    // lgw_reg_w(LGW_CORR_SIG_NOISE_RATIO_SF12,4); /* default 4 */

    /* LoRa 'multi' demodulators setup */
    // lgw_reg_w(LGW_PREAMBLE_SYMB1_NB,10); /* default 10 */
    // lgw_reg_w(LGW_FREQ_TO_TIME_INVERT,29); /* default 29 */
    // lgw_reg_w(LGW_FRAME_SYNCH_GAIN,1); /* default 1 */
    // lgw_reg_w(LGW_SYNCH_DETECT_TH,1); /* default 1 */
    // lgw_reg_w(LGW_ZERO_PAD,0); /* default 0 */
    lgw_reg_w(LGW_SNR_AVG_CST,3); /* default 2 */
    if (lorawan_public) { /* LoRa network */
        lgw_reg_w(LGW_FRAME_SYNCH_PEAK1_POS,3); /* default 1 */
        lgw_reg_w(LGW_FRAME_SYNCH_PEAK2_POS,4); /* default 2 */
    } else { /* private network */
        lgw_reg_w(LGW_FRAME_SYNCH_PEAK1_POS,1); /* default 1 */
        lgw_reg_w(LGW_FRAME_SYNCH_PEAK2_POS,2); /* default 2 */
    }

    // lgw_reg_w(LGW_PREAMBLE_FINE_TIMING_GAIN,1); /* default 1 */
    // lgw_reg_w(LGW_ONLY_CRC_EN,1); /* default 1 */
    // lgw_reg_w(LGW_PAYLOAD_FINE_TIMING_GAIN,2); /* default 2 */
    // lgw_reg_w(LGW_TRACKING_INTEGRAL,0); /* default 0 */
    // lgw_reg_w(LGW_ADJUST_MODEM_START_OFFSET_RDX8,0); /* default 0 */
    // lgw_reg_w(LGW_ADJUST_MODEM_START_OFFSET_SF12_RDX4,4092); /* default 4092 */
    // lgw_reg_w(LGW_MAX_PAYLOAD_LEN,255); /* default 255 */

    /* LoRa standalone 'MBWSSF' demodulator setup */
    // lgw_reg_w(LGW_MBWSSF_PREAMBLE_SYMB1_NB,10); /* default 10 */
    // lgw_reg_w(LGW_MBWSSF_FREQ_TO_TIME_INVERT,29); /* default 29 */
    // lgw_reg_w(LGW_MBWSSF_FRAME_SYNCH_GAIN,1); /* default 1 */
    // lgw_reg_w(LGW_MBWSSF_SYNCH_DETECT_TH,1); /* default 1 */
    // lgw_reg_w(LGW_MBWSSF_ZERO_PAD,0); /* default 0 */
    if (lorawan_public) { /* LoRa network */
        lgw_reg_w(LGW_MBWSSF_FRAME_SYNCH_PEAK1_POS,3); /* default 1 */
        lgw_reg_w(LGW_MBWSSF_FRAME_SYNCH_PEAK2_POS,4); /* default 2 */
    } else {
        lgw_reg_w(LGW_MBWSSF_FRAME_SYNCH_PEAK1_POS,1); /* default 1 */
        lgw_reg_w(LGW_MBWSSF_FRAME_SYNCH_PEAK2_POS,2); /* default 2 */
    }
    // lgw_reg_w(LGW_MBWSSF_ONLY_CRC_EN,1); /* default 1 */
    // lgw_reg_w(LGW_MBWSSF_PAYLOAD_FINE_TIMING_GAIN,2); /* default 2 */
    // lgw_reg_w(LGW_MBWSSF_PREAMBLE_FINE_TIMING_GAIN,1); /* default 1 */
    // lgw_reg_w(LGW_MBWSSF_TRACKING_INTEGRAL,0); /* default 0 */
    // lgw_reg_w(LGW_MBWSSF_AGC_FREEZE_ON_DETECT,1); /* default 1 */

    /* Improvement of reference clock frequency error tolerance */
    lgw_reg_w(LGW_ADJUST_MODEM_START_OFFSET_RDX4, 1); /* default 0 */
    lgw_reg_w(LGW_ADJUST_MODEM_START_OFFSET_SF12_RDX4, 4094); /* default 4092 */
    lgw_reg_w(LGW_CORR_MAC_GAIN, 7); /* default 5 */

    /* FSK datapath setup */
    lgw_reg_w(LGW_FSK_RX_INVERT,1); /* default 0 */
    lgw_reg_w(LGW_FSK_MODEM_INVERT_IQ,1); /* default 0 */

    /* FSK demodulator setup */
    lgw_reg_w(LGW_FSK_RSSI_LENGTH,4); /* default 0 */
    lgw_reg_w(LGW_FSK_PKT_MODE,1); /* variable length, default 0 */
    lgw_reg_w(LGW_FSK_CRC_EN,1); /* default 0 */
    lgw_reg_w(LGW_FSK_DCFREE_ENC,2); /* default 0 */
    // lgw_reg_w(LGW_FSK_CRC_IBM,0); /* default 0 */
    lgw_reg_w(LGW_FSK_ERROR_OSR_TOL,10); /* default 0 */
    lgw_reg_w(LGW_FSK_PKT_LENGTH,255); /* max packet length in variable length mode */
    // lgw_reg_w(LGW_FSK_NODE_ADRS,0); /* default 0 */
    // lgw_reg_w(LGW_FSK_BROADCAST,0); /* default 0 */
    // lgw_reg_w(LGW_FSK_AUTO_AFC_ON,0); /* default 0 */
    lgw_reg_w(LGW_FSK_PATTERN_TIMEOUT_CFG,128); /* sync timeout (allow 8 bytes preamble + 8 bytes sync word, default 0 */

    /* TX general parameters */
    lgw_reg_w(LGW_TX_START_DELAY, TX_START_DELAY_DEFAULT); /* default 0 */

    /* TX LoRa */
    //lgw_reg_w(LGW_TX_MODE,0); /* default 0,single;1, continue*/
    lgw_reg_w(LGW_TX_SWAP_IQ,1); /* "normal" polarity; default 0 */
    if (lorawan_public) { /* LoRa network */
        lgw_reg_w(LGW_TX_FRAME_SYNCH_PEAK1_POS,3); /* default 1 */
        lgw_reg_w(LGW_TX_FRAME_SYNCH_PEAK2_POS,4); /* default 2 */
    } else { /* Private network */
        lgw_reg_w(LGW_TX_FRAME_SYNCH_PEAK1_POS,1); /* default 1 */
        lgw_reg_w(LGW_TX_FRAME_SYNCH_PEAK2_POS,2); /* default 2 */
    }

    /* TX FSK */
    // lgw_reg_w(LGW_FSK_TX_GAUSSIAN_EN,1); /* default 1 */
    lgw_reg_w(LGW_FSK_TX_GAUSSIAN_SELECT_BT,2); /* Gaussian filter always on TX, default 0 */
    // lgw_reg_w(LGW_FSK_TX_PATTERN_EN,1); /* default 1 */
    // lgw_reg_w(LGW_FSK_TX_PREAMBLE_SEQ,0); /* default 0 */
    return;
}
/*********************************读写操作***************************************/
static uint8 vSpiRead(uint8 reg)
{
    spiDAT1_t dataconfig1_t;
    uint16 spi_data;
    uint16 data = 0;

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL = TRUE;
    dataconfig1_t.DFSEL = SX_DATA_FORMAT;    /* PHASE is 1, POLARITY is 0, charlen is 16bit */
    dataconfig1_t.CSNR = (~rf_pin.cs) & 0xFF;   /* Chip select */

    spi_data = (reg << 8);
    gioSetBit(rf_pin.csport, rf_pin.cs, 0);
    spiTransmitAndReceiveData(rf_pin.reg, &dataconfig1_t, 1, &spi_data, &data);
    gioSetBit(rf_pin.csport, rf_pin.cs, 1);
    return (data & 0xFF);
}

static void vSpiWrite(uint8 reg, uint8 data)
{
    spiDAT1_t dataconfig1_t;
    uint16 spi_data;

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL = TRUE;
    dataconfig1_t.DFSEL = SX_DATA_FORMAT;     /* PHASE is 1, POLARITY is 0, charlen is 16bit */
    dataconfig1_t.CSNR = (~rf_pin.cs) & 0xFF;   /* Chip select */

    spi_data = ((reg << 8) | 0x8000) | data;
    gioSetBit(rf_pin.csport, rf_pin.cs, 0);
    spiTransmitData(rf_pin.reg, &dataconfig1_t, 1, &spi_data);
    gioSetBit(rf_pin.csport, rf_pin.cs, 1);
}

static void vSpiBurstRead(uint8 reg, uint8* msg, uint32 len)
{
    int i;
    if(msg == NULL){
        return ;
    }
    spiDAT1_t dataconfig1_t;
    uint16 addr;
    uint16 spi_burst_data;

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL = TRUE;
    dataconfig1_t.DFSEL = SPI_FMT_1;    /* PHASE is 1, POLARITY is 0, charlen is 8bit */
    dataconfig1_t.CSNR = (~rf_pin.cs) & 0xFF;   /* Chip select */

    addr = reg;
    gioSetBit(rf_pin.csport, rf_pin.cs, 0);
    spiTransmitAndReceiveData(rf_pin.reg, &dataconfig1_t, 1, &addr, &spi_burst_data);
    for(i = 0; i < len; i++){
        spiTransmitAndReceiveData(rf_pin.reg, &dataconfig1_t, 1, &addr, &spi_burst_data);
        msg[i] = spi_burst_data ;
    }
    gioSetBit(rf_pin.csport, rf_pin.cs, 1);

}

static void vSpiBurstWrite(uint8 reg, uint8* msg, uint32 len)
{
    spiDAT1_t dataconfig1_t;
    uint16 addr;
    uint16 spi_burst_data;

    int i;
    if(msg == NULL){
        return ;
    }

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL = TRUE;
    dataconfig1_t.DFSEL = SPI_FMT_1;     /* PHASE is 1, POLARITY is 0, charlen is 8bit */
    dataconfig1_t.CSNR = (~rf_pin.cs) & 0xFF;   /* Chip select */

    addr = reg | 0x80;
    gioSetBit(rf_pin.csport, rf_pin.cs, 0);
    spiTransmitData(rf_pin.reg, &dataconfig1_t, 1, &addr);

    for(i = 0; i < len; i++){
        spi_burst_data = msg[i];
        spiTransmitData(rf_pin.reg, &dataconfig1_t, 1, &spi_burst_data);
    }
    gioSetBit(rf_pin.csport, rf_pin.cs, 1);

}
static void vSpiBurstWriteFw(uint8 reg, uint16* msg, uint32 len)
{
    spiDAT1_t dataconfig1_t;
    uint16 addr;

    if(msg == NULL){
        return ;
    }

    dataconfig1_t.CS_HOLD = FALSE;
    dataconfig1_t.WDEL = TRUE;
    dataconfig1_t.DFSEL = SPI_FMT_1;     /* PHASE is 1, POLARITY is 0, charlen is 8bit */
    dataconfig1_t.CSNR = (~rf_pin.cs) & 0xFF;   /* Chip select */

    addr = reg | 0x80;
    gioSetBit(rf_pin.csport, rf_pin.cs, 0);
    spiTransmitData(rf_pin.reg, &dataconfig1_t, 1, &addr);
    spiTransmitData(rf_pin.reg, &dataconfig1_t, len, msg);
    gioSetBit(rf_pin.csport, rf_pin.cs, 1);

}
static int reg_r_align32(struct lgw_reg_s r, int32_t *reg_value) {
    uint8_t bufu[4] = "\x00\x00\x00\x00";
    int8_t *bufs = (int8_t *)bufu;
    int i, size_byte;
    uint32_t u = 0;

    if ((r.offs + r.leng) <= 8) {
        /* read one byte, then shift and mask bits to get reg value with sign extension if needed */
        bufu[0] = vSpiRead(r.addr);
        bufu[1] = bufu[0] << (8 - r.leng - r.offs); /* left-align the data */
        if (r.sign == true) {
            bufs[2] = bufs[1] >> (8 - r.leng); /* right align the data with sign extension (ARITHMETIC right shift) */
            *reg_value = (int32_t)bufs[2]; /* signed pointer -> 32b sign extension */
        } else {
            bufu[2] = bufu[1] >> (8 - r.leng); /* right align the data, no sign extension */
            *reg_value = (int32_t)bufu[2]; /* unsigned pointer -> no sign extension */
        }
    } else if ((r.offs == 0) && (r.leng > 0) && (r.leng <= 32)) {
        size_byte = (r.leng + 7) / 8; /* add a byte if it's not an exact multiple of 8 */
        vSpiBurstRead(r.addr, bufu, size_byte);
        u = 0;
        for (i=(size_byte-1); i>=0; --i) {
            u = (uint32_t)bufu[i] + (u << 8); /* transform a 4-byte array into a 32 bit word */
        }
//        for (i=0; i < size_byte; i++) {
//             u = (uint32_t)bufu[i] + (u << 8); /* transform a 4-byte array into a 32 bit word */
//         }
        if (r.sign == true) {
            u = u << (32 - r.leng); /* left-align the data */
            *reg_value = (int32_t)u >> (32 - r.leng); /* right-align the data with sign extension (ARITHMETIC right shift) */
        } else {
            *reg_value = (int32_t)u; /* unsigned value -> return 'as is' */
        }
    } else {
        /* register spanning multiple memory bytes but with an offset */
        print("ERROR: REGISTER SIZE AND OFFSET ARE NOT SUPPORTED\n");
        return FALSE;
    }

    return TRUE;
}
static void reg_w_align32(struct lgw_reg_s r, int32_t reg_value) {
    int i, size_byte;
    uint8_t buf[4] = "\x00\x00\x00\x00";

    if ((r.leng == 8) && (r.offs == 0)) {
        /* direct write */
        vSpiWrite(r.addr, (uint8_t)reg_value);
    } else if ((r.offs + r.leng) <= 8) {
        /* single-byte read-modify-write, offs:[0-7], leng:[1-7] */
        buf[0] = vSpiRead(r.addr);
        buf[1] = ((1 << r.leng) - 1) << r.offs; /* bit mask */
        buf[2] = ((uint8_t)reg_value) << r.offs; /* new data offsetted */
        buf[3] = (~buf[1] & buf[0]) | (buf[1] & buf[2]); /* mixing old & new data */
        vSpiWrite(r.addr, buf[3]);
    } else if ((r.offs == 0) && (r.leng > 0) && (r.leng <= 32)) {
        /* multi-byte direct write routine */
        size_byte = (r.leng + 7) / 8; /* add a byte if it's not an exact multiple of 8 */
        for (i=0; i<size_byte; ++i) {
            /* big endian register file for a file on N bytes
            Least significant byte is stored in buf[0], most one in buf[N-1] */
            buf[i] = (uint8_t)(0x000000FF & reg_value);
            reg_value = (reg_value >> 8);
        }
//        for (i=0; i<size_byte; ++i) {
//            buf[size_byte - i - 1] = (uint8_t)(0x000000FF & reg_value);
//            reg_value = (reg_value >> 8);
//        }
        vSpiBurstWrite(r.addr, buf, size_byte); /* write the register in one burst */
    } else {
        /* register spanning multiple memory bytes but with an offset */
        print("ERROR: REGISTER SIZE AND OFFSET ARE NOT SUPPORTED\n");
        return ;
    }
}


/*********************************PRIVATE FUNCTIONS***************************************/
//select the page of registers : valid range 0 to 2
static void page_switch(uint8_t target)
{
    lgw_regpage = PAGE_MASK & target;
    vSpiWrite(PAGE_ADDR, lgw_regpage);
}
/*
 * soft-reset function
 * to reset the whole hardware by resetting the register array
 *  */
static uint8 lgw_soft_reset(void) {
    /* check if SPI is initialised */
    if (lgw_regpage < 0) {
        print("ERROR: CONCENTRATOR UNCONNECTED\n");
        return FALSE;
    }
    vSpiWrite( 0, 0x80); /* 1 -> SOFT_RESET bit */

    lgw_regpage = 0; /* reset the paging static variable */
    return TRUE;
}

//to initialise and check the connection with the hardware
/* Concentrator connect */
static uint8 lgw_connect(void)
{
    uint8 u = 0;

    /* check SX1301 version */
    u = vSpiRead(loregs[LGW_VERSION].addr);
    if (u != loregs[LGW_VERSION].dflt) {
        print("ERROR: NOT EXPECTED CHIP VERSION (v%u)\n", u);
        return FALSE;
    }
    /* write 0 to the page/reset register */
    vSpiWrite(loregs[LGW_PAGE_REG].addr, 0);
    lgw_regpage = 0; /* reset the paging static variable */
    return TRUE;
}
/*********************************PUBLIC FUNCTIONS***************************************/

/* Read to a register addressed by name */
uint8 lgw_reg_r(uint16_t register_id, int32_t *reg_value) {
    struct lgw_reg_s r;

    /* check input parameters */
    if (register_id >= LGW_TOTALREGS) {
        print("ERROR: REGISTER NUMBER OUT OF DEFINED RANGE\n");
        return FALSE;
    }

    /* get register struct from the struct array */
    r = loregs[register_id];

    /* select proper register page if needed */
    if ((r.page != -1) && (r.page != lgw_regpage)) {
        page_switch(r.page);
    }

    reg_r_align32(r, reg_value);
    return TRUE;


}
/* Point to a register by name and do a burst read */
uint8 lgw_reg_rb(uint16_t register_id, uint8_t *data, uint16_t size)
{
    struct lgw_reg_s r;

    /* check input parameters */
    if (register_id >= LGW_TOTALREGS) {
        print("ERROR: REGISTER NUMBER OUT OF DEFINED RANGE\n");
        return FALSE;
    }

    /* get register struct from the struct array */
    r = loregs[register_id];

    /* select proper register page if needed */
    if ((r.page != -1) && (r.page != lgw_regpage)) {
        page_switch(r.page);
    }


    /* do the burst read */
    vSpiBurstRead( r.addr, data, size);

    return TRUE;


}

/* Write to a register addressed by name */
uint8 lgw_reg_w(uint16_t register_id, int32_t reg_value)
{
    struct lgw_reg_s r;

    /* check input parameters */
    if (register_id >= LGW_TOTALREGS) {
        print("ERROR: REGISTER NUMBER OUT OF DEFINED RANGE\n");
        return FALSE;
    }
    /* check if SPI is initialised */
    if (lgw_regpage < 0) {
        print("ERROR: CONCENTRATOR UNCONNECTED\n");
        return FALSE;
    }
    /* intercept direct access to PAGE_REG & SOFT_RESET */
    if (register_id == LGW_PAGE_REG) {
        page_switch(reg_value);
        return TRUE;
    } else if (register_id == LGW_SOFT_RESET) {
        /* only reset if lsb is 1 */
        if ((reg_value & 0x01) != 0)
            lgw_soft_reset();
        return TRUE;
    }
    /* get register struct from the struct array */
    r = loregs[register_id];

    /* reject write to read-only registers */
    if (r.rdon == 1){
        print("ERROR: TRYING TO WRITE A READ-ONLY REGISTER\n");
        return FALSE;
    }

    /* select proper register page if needed */
    if ((r.page != -1) && (r.page != lgw_regpage)) {
        page_switch(r.page);
    }

    reg_w_align32(r, reg_value);

    return TRUE;
}

int lgw_reg_wb(uint16_t register_id, uint8_t *data, uint16_t size) {
    struct lgw_reg_s r;

    /* check input parameters */
    if (size == 0) {
        print("ERROR: BURST OF NULL LENGTH\n");
        return FALSE;
    }
    if (register_id >= LGW_TOTALREGS) {
        print("ERROR: REGISTER NUMBER OUT OF DEFINED RANGE\n");
        return FALSE;
    }

    /* check if SPI is initialised */
    if (lgw_regpage < 0){
        print("ERROR: CONCENTRATOR UNCONNECTED\n");
        return FALSE;
    }

    /* get register struct from the struct array */
    r = loregs[register_id];

    /* reject write to read-only registers */
    if (r.rdon == 1){
        print("ERROR: TRYING TO BURST WRITE A READ-ONLY REGISTER\n");
        return FALSE;
    }

    /* select proper register page if needed */
    if ((r.page != -1) && (r.page != lgw_regpage)) {
        page_switch(r.page);
    }

    /* do the burst write */
    vSpiBurstWrite(r.addr, data, size);

    return TRUE;

}

int lgw_reg_wb_fw(uint16_t register_id, uint16_t *data, uint16_t size) {
    struct lgw_reg_s r;

    /* get register struct from the struct array */
    r = loregs[register_id];

    /* select proper register page if needed */
    if ((r.page != -1) && (r.page != lgw_regpage)) {
        page_switch(r.page);
    }

    /* do the burst write */
    vSpiBurstWriteFw(r.addr, data, size);

    return TRUE;

}
//uint8_t fw_check[8192];

/* size is the firmware size in bytes (not 14b words) */
int load_firmware(uint8_t target, const uint16_t *firmware, uint16_t size) {
    int reg_rst;
    int reg_sel;
//    int32_t dummy;

    /* check parameters */
    if (target == MCU_ARB) {
        if (size != MCU_ARB_FW_BYTE) {
            print("ERROR: NOT A VALID SIZE FOR MCU ARG FIRMWARE\n");
            return -1;
        }
        reg_rst = LGW_MCU_RST_0;
        reg_sel = LGW_MCU_SELECT_MUX_0;
    }else if (target == MCU_AGC) {
        if (size != MCU_AGC_FW_BYTE) {
            print("ERROR: NOT A VALID SIZE FOR MCU AGC FIRMWARE\n");
            return -1;
        }
        reg_rst = LGW_MCU_RST_1;
        reg_sel = LGW_MCU_SELECT_MUX_1;
    } else {
        print("ERROR: NOT A VALID TARGET FOR LOADING FIRMWARE\n");
        return -1;
    }

    /* reset the targeted MCU */
    lgw_reg_w(reg_rst, 1);

    /* set mux to access MCU program RAM and set address to 0 */
    lgw_reg_w(reg_sel, 0);
    lgw_reg_w(LGW_MCU_PROM_ADDR, 0);

    /* write the program in one burst */
    lgw_reg_wb_fw(LGW_MCU_PROM_DATA, (uint16 *)firmware, size);

//    /* Read back firmware code for check */
//    lgw_reg_r( LGW_MCU_PROM_DATA, &dummy ); /* bug workaround */
//    lgw_reg_rb( LGW_MCU_PROM_DATA, fw_check, size );
//    if (memcmp(firmware, fw_check, size) != 0) {
//        printf ("ERROR: Failed to load fw %d\n", (int)target);
//        return -1;
//    }

    /* give back control of the MCU program ram to the MCU */
    lgw_reg_w(reg_sel, 1);

    return 0;
}

uint8 lgw_start(void)
{
    int i,err;
    unsigned x;

    int32_t read_val;
    uint8_t cal_cmd, cal_status, fw_version, radio_select, load_val;
    uint16_t cal_time;
    uint64_t fsk_sync_word_reg;



    if (lgw_is_started == TRUE) {
        print("Note: LoRa concentrator already started, restarting it now\n");
    }
    lgw_connect();
    /* reset the registers (also shuts the radios down) */
    lgw_soft_reset();
    /* gate clocks */
    lgw_reg_w(LGW_GLOBAL_EN, 0);
    lgw_reg_w(LGW_CLK32M_EN, 0);

    /* switch on and reset the radios (also starts the 32 MHz XTAL) */
    lgw_reg_w(LGW_RADIO_A_EN,1);//未连接，没有啥用，用来控制参考设计的射频单刀双掷开关
    lgw_reg_w(LGW_RADIO_B_EN,0);
    FeedWatchDog();

    wait_ms(5000+20); //TODO 500ms
    FeedWatchDog();

    lgw_reg_w(LGW_RADIO_RST,1);
    wait_ms(50+2); //5ms
    lgw_reg_w(LGW_RADIO_RST,0);

    /* setup the radios */
    err = lgw_setup_sx125x(0, rf_clkout, rf_enable[0], rf_radio_type[0], rf_rx_freq[0]);
    if (err != 0) {
        print("ERROR: Failed to setup sx125x radio for RF chain 0\n");
        return FALSE;
    }
    err = lgw_setup_sx125x(1, rf_clkout, rf_enable[1], rf_radio_type[1], rf_rx_freq[1]);
    if (err != 0) {
        print("ERROR: Failed to setup sx125x radio for RF chain 1\n");
        return FALSE;
    }

    /* gives AGC control of GPIOs to enable Tx external digital filter */
    lgw_reg_w(LGW_GPIO_MODE,31); /* Set all GPIOs as output */
    lgw_reg_w(LGW_GPIO_SELECT_OUTPUT,10);

    /*TODO  Configure LBT ,Not used*/

    /* Enable clocks */
    lgw_reg_w(LGW_GLOBAL_EN, 1);
    lgw_reg_w(LGW_CLK32M_EN, 1);

    /* GPIOs table :
    DGPIO0 -> tx_on
    DGPIO1 -> fsk/bh_pkt
    DGPIO2 -> gps_hpps
    DGPIO3 -> sensor_pkt
    DGPIO4 -> rx_buffer_not_empty
    */


    /* select calibration command */
    cal_cmd = 0;
    cal_cmd |= rf_enable[0] ? 0x01 : 0x00; /* Bit 0: Calibrate Rx IQ mismatch compensation on radio A */
    cal_cmd |= rf_enable[1] ? 0x02 : 0x00; /* Bit 1: Calibrate Rx IQ mismatch compensation on radio B */
    cal_cmd |= (rf_enable[0] && rf_tx_enable[0]) ? 0x04 : 0x00; /* Bit 2: Calibrate Tx DC offset on radio A */
    cal_cmd |= (rf_enable[1] && rf_tx_enable[1]) ? 0x08 : 0x00; /* Bit 3: Calibrate Tx DC offset on radio B */
    cal_cmd |= 0x10; /* Bit 4: 0: calibrate with DAC gain=2, 1: with DAC gain=3 (use 3) */
    switch (rf_radio_type[0]) { /* we assume that there is only one radio type on the board */
        case LGW_RADIO_TYPE_SX1255:
            cal_cmd |= 0x20; /* Bit 5: 0: SX1257, 1: SX1255 */
            break;
        case LGW_RADIO_TYPE_SX1257:
            cal_cmd |= 0x00; /* Bit 5: 0: SX1257, 1: SX1255 */
            break;
        default:
            print("ERROR: UNEXPECTED VALUE %d FOR RADIO TYPE\n", rf_radio_type[0]);
            break;
    }

    cal_cmd |= 0x00; /* Bit 6-7: Board type 0: ref, 1: FPGA, 3: board X */
    cal_time = 2300; /* measured between 2.1 and 2.2 sec, because 1 TX only */
    /* Load the calibration firmware  */
    load_firmware(MCU_AGC, cal_firmware, MCU_AGC_FW_BYTE);
    lgw_reg_w(LGW_FORCE_HOST_RADIO_CTRL, 0); /* gives to AGC MCU the control of the radios */
    lgw_reg_w(LGW_RADIO_SELECT, cal_cmd); /* send calibration configuration word */
    lgw_reg_w(LGW_MCU_RST_1, 0);

    /* Check firmware version */
    lgw_reg_w(LGW_DBG_AGC_MCU_RAM_ADDR, FW_VERSION_ADDR);
    lgw_reg_r(LGW_DBG_AGC_MCU_RAM_DATA, &read_val);
    fw_version = (uint8_t)read_val;
    if (fw_version != FW_VERSION_CAL) {
        print("ERROR: Version of calibration firmware not expected\n");
        return FALSE;
    }

    lgw_reg_w(LGW_PAGE_REG, 3); /* Calibration will start on this condition as soon as MCU can talk to concentrator registers */
    lgw_reg_w(LGW_EMERGENCY_FORCE_HOST_CTRL, 0); /* Give control of concentrator registers to MCU */

    /* Wait for calibration to end */
    wait_ms(cal_time * 12);/* Wait for end of calibration */
    lgw_reg_w(LGW_EMERGENCY_FORCE_HOST_CTRL, 1); /* Take back control */

    /* Get calibration status */
    lgw_reg_r(LGW_MCU_AGC_STATUS, &read_val);
    cal_status = (uint8_t)read_val;
    /*
        bit 7: calibration finished
        bit 0: could access SX1301 registers
        bit 1: could access radio A registers
        bit 2: could access radio B registers
        bit 3: radio A RX image rejection successful
        bit 4: radio B RX image rejection successful
        bit 5: radio A TX DC Offset correction successful
        bit 6: radio B TX DC Offset correction successful
    */
    if ((cal_status & 0x81) != 0x81) {
        print("ERROR: CALIBRATION FAILURE (STATUS = %d)\n", cal_status);
        return FALSE;
    } else {
//        print("Note: calibration finished (status = %d)\n", cal_status);
    }
    if (rf_enable[0] && ((cal_status & 0x02) == 0)) {
        print("WARNING: calibration could not access radio A\n");
    }
    if (rf_enable[1] && ((cal_status & 0x04) == 0)) {
        print(" WARNING: calibration could not access radio B\n");
    }
    if (rf_enable[0] && ((cal_status & 0x08) == 0)) {
        print("WARNING: problem in calibration of radio A for image rejection\n");
    }
    if (rf_enable[1] && ((cal_status & 0x10) == 0)) {
        print("WARNING: problem in calibration of radio B for image rejection\n");
    }
    if (rf_enable[0] && rf_tx_enable[0] && ((cal_status & 0x20) == 0)) {
        print("WARNING: problem in calibration of radio A for TX DC offset\n");
    }
    if (rf_enable[1] && rf_tx_enable[1] && ((cal_status & 0x40) == 0)) {
        print(" WARNING: problem in calibration of radio B for TX DC offset\n");
    }

    /* Get TX DC offset values */
    for(i=0; i<=7; ++i) {
        lgw_reg_w(LGW_DBG_AGC_MCU_RAM_ADDR, 0xA0+i);
        lgw_reg_r(LGW_DBG_AGC_MCU_RAM_DATA, &read_val);
        cal_offset_a_i[i] = (int8_t)read_val;
        lgw_reg_w(LGW_DBG_AGC_MCU_RAM_ADDR, 0xA8+i);
        lgw_reg_r(LGW_DBG_AGC_MCU_RAM_DATA, &read_val);
        cal_offset_a_q[i] = (int8_t)read_val;
        lgw_reg_w(LGW_DBG_AGC_MCU_RAM_ADDR, 0xB0+i);
        lgw_reg_r(LGW_DBG_AGC_MCU_RAM_DATA, &read_val);
        cal_offset_b_i[i] = (int8_t)read_val;
        lgw_reg_w(LGW_DBG_AGC_MCU_RAM_ADDR, 0xB8+i);
        lgw_reg_r(LGW_DBG_AGC_MCU_RAM_DATA, &read_val);
        cal_offset_b_q[i] = (int8_t)read_val;
    }

    /* load adjusted parameters */
    lgw_constant_adjust();

    /* Sanity check for RX frequency */
    if (rf_rx_freq[0] == 0) {
        print("ERROR: wrong configuration, rf_rx_freq[0] is not set\n");
        return FALSE;
    }
    /* Freq-to-time-drift calculation */
    x = 4096000000 / (rf_rx_freq[0] >> 1); /* dividend: (4*2048*1000000) >> 1, rescaled to avoid 32b overflow */
    x = ( x > 63 ) ? 63 : x; /* saturation */
    lgw_reg_w(LGW_FREQ_TO_TIME_DRIFT, x); /* default 9 */

    x = 4096000000 / (rf_rx_freq[0] >> 3); /* dividend: (16*2048*1000000) >> 3, rescaled to avoid 32b overflow */
    x = ( x > 63 ) ? 63 : x; /* saturation */
    lgw_reg_w(LGW_MBWSSF_FREQ_TO_TIME_DRIFT, x); /* default 36 */

    /* configure LoRa 'multi' demodulators aka. LoRa 'sensor' channels (IF0-3) */
    radio_select = 0; /* IF mapping to radio A/B (per bit, 0=A, 1=B) */
    for(i=0; i<LGW_MULTI_NB; ++i) {
        radio_select += (if_rf_chain[i] == 1 ? 1 << i : 0); /* transform bool array into binary word */
    }
    /*
    lgw_reg_w(LGW_RADIO_SELECT, radio_select);

    LGW_RADIO_SELECT is used for communication with the firmware, "radio_select"
    will be loaded in LGW_RADIO_SELECT at the end of start procedure.
    */

    lgw_reg_w(LGW_IF_FREQ_0, IF_HZ_TO_REG(if_freq[0])); /* default -384 */
    lgw_reg_w(LGW_IF_FREQ_1, IF_HZ_TO_REG(if_freq[1])); /* default -128 */
    lgw_reg_w(LGW_IF_FREQ_2, IF_HZ_TO_REG(if_freq[2])); /* default 128 */
    lgw_reg_w(LGW_IF_FREQ_3, IF_HZ_TO_REG(if_freq[3])); /* default 384 */
    lgw_reg_w(LGW_IF_FREQ_4, IF_HZ_TO_REG(if_freq[4])); /* default -384 */
    lgw_reg_w(LGW_IF_FREQ_5, IF_HZ_TO_REG(if_freq[5])); /* default -128 */
    lgw_reg_w(LGW_IF_FREQ_6, IF_HZ_TO_REG(if_freq[6])); /* default 128 */
    lgw_reg_w(LGW_IF_FREQ_7, IF_HZ_TO_REG(if_freq[7])); /* default 384 */

    lgw_reg_w(LGW_CORR0_DETECT_EN, (if_enable[0] == true) ? lora_multi_sfmask[0] : 0); /* default 0 */
    lgw_reg_w(LGW_CORR1_DETECT_EN, (if_enable[1] == true) ? lora_multi_sfmask[1] : 0); /* default 0 */
    lgw_reg_w(LGW_CORR2_DETECT_EN, (if_enable[2] == true) ? lora_multi_sfmask[2] : 0); /* default 0 */
    lgw_reg_w(LGW_CORR3_DETECT_EN, (if_enable[3] == true) ? lora_multi_sfmask[3] : 0); /* default 0 */
    lgw_reg_w(LGW_CORR4_DETECT_EN, (if_enable[4] == true) ? lora_multi_sfmask[4] : 0); /* default 0 */
    lgw_reg_w(LGW_CORR5_DETECT_EN, (if_enable[5] == true) ? lora_multi_sfmask[5] : 0); /* default 0 */
    lgw_reg_w(LGW_CORR6_DETECT_EN, (if_enable[6] == true) ? lora_multi_sfmask[6] : 0); /* default 0 */
    lgw_reg_w(LGW_CORR7_DETECT_EN, (if_enable[7] == true) ? lora_multi_sfmask[7] : 0); /* default 0 */

    lgw_reg_w(LGW_PPM_OFFSET, 0x60); /* as the threshold is 16ms, use 0x60 to enable ppm_offset for SF12 and SF11 @125kHz*/

    lgw_reg_w(LGW_CONCENTRATOR_MODEM_ENABLE, 1); /* default 0 */


    /* configure LoRa 'stand-alone' modem (IF8) */
    lgw_reg_w(LGW_IF_FREQ_8, IF_HZ_TO_REG(if_freq[8])); /* MBWSSF modem (default 0) */
    if (if_enable[8] == true) {
        lgw_reg_w(LGW_MBWSSF_RADIO_SELECT, if_rf_chain[8]);
        switch(lora_rx_bw) {
            case BW_125KHZ: lgw_reg_w(LGW_MBWSSF_MODEM_BW, 0); break;
            case BW_250KHZ: lgw_reg_w(LGW_MBWSSF_MODEM_BW, 1); break;
            case BW_500KHZ: lgw_reg_w(LGW_MBWSSF_MODEM_BW, 2); break;
            default:
                print("ERROR: UNEXPECTED VALUE %d IN SWITCH STATEMENT\n", lora_rx_bw);
                return FALSE;
        }
        switch(lora_rx_sf) {
            case DR_LORA_SF7: lgw_reg_w(LGW_MBWSSF_RATE_SF, 7); break;
            case DR_LORA_SF8: lgw_reg_w(LGW_MBWSSF_RATE_SF, 8); break;
            case DR_LORA_SF9: lgw_reg_w(LGW_MBWSSF_RATE_SF, 9); break;
            case DR_LORA_SF10: lgw_reg_w(LGW_MBWSSF_RATE_SF, 10); break;
            case DR_LORA_SF11: lgw_reg_w(LGW_MBWSSF_RATE_SF, 11); break;
            case DR_LORA_SF12: lgw_reg_w(LGW_MBWSSF_RATE_SF, 12); break;
            default:
                print("ERROR: UNEXPECTED VALUE %d IN SWITCH STATEMENT\n", lora_rx_sf);
                return FALSE;
        }
        lgw_reg_w(LGW_MBWSSF_PPM_OFFSET, lora_rx_ppm_offset); /* default 0 */
        lgw_reg_w(LGW_MBWSSF_MODEM_ENABLE, 1); /* default 0 */
    } else {
        lgw_reg_w(LGW_MBWSSF_MODEM_ENABLE, 0);
    }

    /* configure FSK modem (IF9) */
        lgw_reg_w(LGW_IF_FREQ_9, IF_HZ_TO_REG(if_freq[9])); /* FSK modem, default 0 */
        lgw_reg_w(LGW_FSK_PSIZE, fsk_sync_word_size-1);
        lgw_reg_w(LGW_FSK_TX_PSIZE, fsk_sync_word_size-1);
        fsk_sync_word_reg = fsk_sync_word << (8 * (8 - fsk_sync_word_size));
        lgw_reg_w(LGW_FSK_REF_PATTERN_LSB, (uint32_t)(0xFFFFFFFF & fsk_sync_word_reg));
        lgw_reg_w(LGW_FSK_REF_PATTERN_MSB, (uint32_t)(0xFFFFFFFF & (fsk_sync_word_reg >> 32)));
        if (if_enable[9] == true) {
            lgw_reg_w(LGW_FSK_RADIO_SELECT, if_rf_chain[9]);
            lgw_reg_w(LGW_FSK_BR_RATIO, LGW_XTAL_FREQU/fsk_rx_dr); /* setting the dividing ratio for datarate */
            lgw_reg_w(LGW_FSK_CH_BW_EXPO, fsk_rx_bw);
            lgw_reg_w(LGW_FSK_MODEM_ENABLE, 1); /* default 0 */
        } else {
            lgw_reg_w(LGW_FSK_MODEM_ENABLE, 0);
        }

    /* Load firmware */
    load_firmware(MCU_ARB, arb_firmware, MCU_ARB_FW_BYTE);
    load_firmware(MCU_AGC, agc_firmware, MCU_AGC_FW_BYTE);


    /* gives the AGC MCU control over radio, RF front-end and filter gain */
    lgw_reg_w(LGW_FORCE_HOST_RADIO_CTRL, 0);
    lgw_reg_w(LGW_FORCE_HOST_FE_CTRL, 0);
    lgw_reg_w(LGW_FORCE_DEC_FILTER_GAIN, 0);


    /* Get MCUs out of reset */
    lgw_reg_w(LGW_RADIO_SELECT, 0); /* MUST not be = to 1 or 2 at firmware init */
    lgw_reg_w(LGW_MCU_RST_0, 0);
    lgw_reg_w(LGW_MCU_RST_1, 0);


    /* Check firmware version */
    lgw_reg_w(LGW_DBG_AGC_MCU_RAM_ADDR, FW_VERSION_ADDR);
    lgw_reg_r(LGW_DBG_AGC_MCU_RAM_DATA, &read_val);
    fw_version = (uint8_t)read_val;
    if (fw_version != FW_VERSION_AGC) {
        print("ERROR: Version of AGC firmware not expected, actual:%d expected:%d\n", fw_version, FW_VERSION_AGC);
        return FALSE;
    }
    lgw_reg_w(LGW_DBG_ARB_MCU_RAM_ADDR, FW_VERSION_ADDR);
    lgw_reg_r(LGW_DBG_ARB_MCU_RAM_DATA, &read_val);
    fw_version = (uint8_t)read_val;
    if (fw_version != FW_VERSION_ARB) {
        print("ERROR: Version of arbiter firmware not expected, actual:%d expected:%d\n", fw_version, FW_VERSION_ARB);
        return FALSE;
    }

    print("Info: Initialising AGC firmware...\n");
    wait_ms(10+2);

    lgw_reg_r(LGW_MCU_AGC_STATUS, &read_val);
    if (read_val != 0x10) {
        print("ERROR: AGC FIRMWARE INITIALIZATION FAILURE, STATUS 0x%02X\n", (uint8_t)read_val);
        return FALSE;
    }


    /* Update Tx gain LUT and start AGC */
    for (i = 0; i < txgain_lut.size; ++i) {
        lgw_reg_w(LGW_RADIO_SELECT, AGC_CMD_WAIT); /* start a transaction */
        wait_ms(10+2);
        load_val = txgain_lut.lut[i].mix_gain + (16 * txgain_lut.lut[i].dac_gain) + (64 * txgain_lut.lut[i].pa_gain);
        lgw_reg_w(LGW_RADIO_SELECT, load_val);
        wait_ms(10+2);
        lgw_reg_r(LGW_MCU_AGC_STATUS, &read_val);
        if (read_val != (0x30 + i)) {
            print("ERROR: AGC FIRMWARE INITIALIZATION FAILURE, STATUS 0x%02X\n", (uint8_t)read_val);
            return FALSE;
        }
    }

    /* As the AGC fw is waiting for 16 entries, we need to abort the transaction if we get less entries */
    if (txgain_lut.size < 16) {
        lgw_reg_w(LGW_RADIO_SELECT, AGC_CMD_WAIT);
        wait_ms(10+2);
        load_val = AGC_CMD_ABORT;
        lgw_reg_w(LGW_RADIO_SELECT, load_val);
        wait_ms(10+2);
        lgw_reg_r(LGW_MCU_AGC_STATUS, &read_val);
        if (read_val != 0x30) {
            print("ERROR: AGC FIRMWARE INITIALIZATION FAILURE, STATUS 0x%02X\n", (uint8_t)read_val);
            return FALSE;
        }
    }


    /* Load Tx freq MSBs (always 3 if f > 768 for SX1257 or f > 384 for SX1255 */
    lgw_reg_w(LGW_RADIO_SELECT, AGC_CMD_WAIT);
    wait_ms(10+2);
    lgw_reg_w(LGW_RADIO_SELECT, 3);
    wait_ms(10+2);
    lgw_reg_r(LGW_MCU_AGC_STATUS, &read_val);
    if (read_val != 0x33) {
        print("ERROR: AGC FIRMWARE INITIALIZATION FAILURE, STATUS 0x%02X\n", (uint8_t)read_val);
        return FALSE;
    }

    /* Load chan_select firmware option */
    lgw_reg_w(LGW_RADIO_SELECT, AGC_CMD_WAIT);
    wait_ms(10+2);
    lgw_reg_w(LGW_RADIO_SELECT, 0);
    wait_ms(10+2);
    lgw_reg_r(LGW_MCU_AGC_STATUS, &read_val);
    if (read_val != 0x30) {
        print("ERROR: AGC FIRMWARE INITIALIZATION FAILURE, STATUS 0x%02X\n", (uint8_t)read_val);
        return FALSE;
    }

    /* End AGC firmware init and check status */
    lgw_reg_w(LGW_RADIO_SELECT, AGC_CMD_WAIT);
    wait_ms(10+2);
    lgw_reg_w(LGW_RADIO_SELECT, radio_select); /* Load intended value of RADIO_SELECT */
    wait_ms(10+2);
    print("Info: putting back original RADIO_SELECT value\n");
    lgw_reg_r(LGW_MCU_AGC_STATUS, &read_val);
    if (read_val != 0x40) {
        print("ERROR: AGC FIRMWARE INITIALIZATION FAILURE, STATUS 0x%02X\n", (uint8_t)read_val);
        return FALSE;
    }

    /* enable GPS event capture */
    lgw_reg_w(LGW_GPS_EN, 0);

    lgw_is_started = TRUE;
    return TRUE;
}

//to stop the hardware
uint8 lgw_stop(void)
{
    lgw_soft_reset();
    lgw_is_started = false;
    return TRUE;
}

//to check when a packet has effectively been sent
uint8 lgw_status(uint8_t select, uint8_t *code) {
    int32_t read_value;

    if (select == TX_STATUS) {
        lgw_reg_r(LGW_TX_STATUS, &read_value);
        if (lgw_is_started == false) {
            *code = TX_OFF;
        } else if ((read_value & 0x10) == 0) { /* bit 4 @1: TX programmed */
            *code = TX_FREE;
        } else if ((read_value & 0x60) != 0) { /* bit 5 or 6 @1: TX sequence */
            *code = TX_EMITTING;
        } else {
            *code = TX_SCHEDULED;
        }
        return TRUE;

    } else if (select == RX_STATUS) {
        *code = RX_STATUS_UNKNOWN; /* todo */
        return TRUE;

    } else {
        print("ERROR: SELECTION INVALID, NO STATUS TO RETURN\n");
        return FALSE;
    }

}
uint8_t buff[256+TX_METADATA_NB]; /* buffer to prepare the packet to send + metadata before SPI write burst */

uint8 lgw_send(struct lgw_pkt_tx_s pkt_data)
{
    uint32_t part_int = 0; /* integer part for PLL register value calculation */
    uint32_t part_frac = 0; /* fractional part for PLL register value calculation */
    int transfer_size = 0; /* data to transfer from host to TX databuffer */
    int payload_offset = 0; /* start of the payload content in the databuffer */
    uint8_t pow_index = 0; /* 4-bit value to set the firmware TX power */
    uint8_t target_mix_gain = 0; /* used to select the proper I/Q offset correction */
    uint32_t count_trig = 0; /* timestamp value in trigger mode corrected for TX start delay */
    uint16_t fsk_dr_div; /* divider to configure for target datarate */
    bool tx_notch_enable = false;

    uint16_t tx_start_delay = TX_START_DELAY_DEFAULT;

    memset(buff, 0, 256+TX_METADATA_NB);

    /* check if the concentrator is running */
    if (lgw_is_started == FALSE) {
        print("ERROR: CONCENTRATOR IS NOT RUNNING, START IT BEFORE SENDING\n");
        return FALSE;
    }
    /* check input range (segfault prevention) */
    if(pkt_data.rf_chain >= 2){
        return FALSE;
    }
    /* check input variables */
    if (rf_tx_enable[pkt_data.rf_chain] == false) {
        print("ERROR: SELECTED RF_CHAIN IS DISABLED FOR TX ON SELECTED BOARD\n");
        return FALSE;
    }
    if (rf_enable[pkt_data.rf_chain] == false) {
        print("ERROR: SELECTED RF_CHAIN IS DISABLED\n");
        return FALSE;
    }
    if (!IS_TX_MODE(pkt_data.tx_mode)) {
        print("ERROR: TX_MODE NOT SUPPORTED\n");
        return FALSE;
    }
    if (pkt_data.modulation == MOD_LORA) {
        if (!IS_LORA_BW(pkt_data.bandwidth)) {
            print("ERROR: BANDWIDTH NOT SUPPORTED BY LORA TX\n");
            return FALSE;
        }
        if (!IS_LORA_STD_DR(pkt_data.datarate)) {
            print("ERROR: DATARATE NOT SUPPORTED BY LORA TX\n");
            return FALSE;
        }
        if (!IS_LORA_CR(pkt_data.coderate)) {
            print("ERROR: CODERATE NOT SUPPORTED BY LORA TX\n");
            return FALSE;
        }
        if (pkt_data.size > 255) {
            print("ERROR: PAYLOAD LENGTH TOO BIG FOR LORA TX\n");
            return FALSE;
        }
    }else if (pkt_data.modulation == MOD_FSK) {
        if((pkt_data.f_dev < 1) || (pkt_data.f_dev > 200)) {
            print("ERROR: TX FREQUENCY DEVIATION OUT OF ACCEPTABLE RANGE\n");
            return FALSE;
        }
        if(!IS_FSK_DR(pkt_data.datarate)) {
            print("ERROR: DATARATE NOT SUPPORTED BY FSK IF CHAIN\n");
            return FALSE;
        }
        if (pkt_data.size > 255) {
            print("ERROR: PAYLOAD LENGTH TOO BIG FOR FSK TX\n");
            return FALSE;
        }
    } else{
        return FALSE;
    }

    /* Enable notch filter for LoRa 125kHz */
    if ((pkt_data.modulation == MOD_LORA) && (pkt_data.bandwidth == BW_125KHZ)) {
        tx_notch_enable = true;
    }

    /* interpretation of TX power */
    for (pow_index = txgain_lut.size-1; pow_index > 0; pow_index--) {
        if (txgain_lut.lut[pow_index].rf_power <= pkt_data.rf_power) {
            break;
        }
    }
    /* loading TX imbalance correction */
    target_mix_gain = txgain_lut.lut[pow_index].mix_gain;
    if (pkt_data.rf_chain == 0) { /* use radio A calibration table */
        lgw_reg_w(LGW_TX_OFFSET_I, cal_offset_a_i[target_mix_gain]);
        lgw_reg_w(LGW_TX_OFFSET_Q, cal_offset_a_q[target_mix_gain]);
    } else { /* use radio B calibration table */
        lgw_reg_w(LGW_TX_OFFSET_I, cal_offset_b_i[target_mix_gain]);
        lgw_reg_w(LGW_TX_OFFSET_Q, cal_offset_b_q[target_mix_gain]);
    }

    /* Set digital gain from LUT */
    lgw_reg_w(LGW_TX_GAIN, txgain_lut.lut[pow_index].dig_gain);

    /* fixed metadata, useful payload and misc metadata compositing */
    transfer_size = TX_METADATA_NB + pkt_data.size; /*  */
    payload_offset = TX_METADATA_NB; /* start the payload just after the metadata */

    /* metadata 0 to 2, TX PLL frequency */
    switch (rf_radio_type[0]) { /* we assume that there is only one radio type on the board */
        case LGW_RADIO_TYPE_SX1255:
            part_int = pkt_data.freq_hz / (SX125x_32MHz_FRAC << 7); /* integer part, gives the MSB */
            part_frac = ((pkt_data.freq_hz % (SX125x_32MHz_FRAC << 7)) << 9) / SX125x_32MHz_FRAC; /* fractional part, gives middle part and LSB */
            break;
        case LGW_RADIO_TYPE_SX1257:
            part_int = pkt_data.freq_hz / (SX125x_32MHz_FRAC << 8); /* integer part, gives the MSB */
            part_frac = ((pkt_data.freq_hz % (SX125x_32MHz_FRAC << 8)) << 8) / SX125x_32MHz_FRAC; /* fractional part, gives middle part and LSB */
            break;
        default:
            print("ERROR: UNEXPECTED VALUE %d FOR RADIO TYPE\n", rf_radio_type[0]);
            break;
    }

    buff[0] = 0xFF & part_int; /* Most Significant Byte */
    buff[1] = 0xFF & (part_frac >> 8); /* middle byte */
    buff[2] = 0xFF & part_frac; /* Least Significant Byte */

    /* metadata 3 to 6, timestamp trigger value */
    /* TX state machine must be triggered at T0 - TX_START_DELAY for packet to start being emitted at T0 */
    if (pkt_data.tx_mode == TIMESTAMPED) {
        count_trig = pkt_data.count_us - TX_START_DELAY ;
        buff[3] = 0xFF & (count_trig >> 24);
        buff[4] = 0xFF & (count_trig >> 16);
        buff[5] = 0xFF & (count_trig >> 8);
        buff[6] = 0xFF & count_trig;
    }

    if (pkt_data.modulation == MOD_LORA) {
        /* metadata 7, modulation type, radio chain selection and TX power */
        buff[7] = (0x20 & (pkt_data.rf_chain << 5)) | (0x0F & pow_index); /* bit 4 is 0 -> LoRa modulation */
        buff[8] = 0; /* metadata 8, not used */
        /* metadata 9, CRC, LoRa CR & SF */
        switch (pkt_data.datarate) {
            case DR_LORA_SF7: buff[9] = 7; break;
            case DR_LORA_SF8: buff[9] = 8; break;
            case DR_LORA_SF9: buff[9] = 9; break;
            case DR_LORA_SF10: buff[9] = 10; break;
            case DR_LORA_SF11: buff[9] = 11; break;
            case DR_LORA_SF12: buff[9] = 12; break;
            default: print("ERROR: UNEXPECTED VALUE %d IN SWITCH STATEMENT\n", pkt_data.datarate);
        }
        switch (pkt_data.coderate) {
            case CR_LORA_4_5: buff[9] |= 1 << 4; break;
            case CR_LORA_4_6: buff[9] |= 2 << 4; break;
            case CR_LORA_4_7: buff[9] |= 3 << 4; break;
            case CR_LORA_4_8: buff[9] |= 4 << 4; break;
            default: print("ERROR: UNEXPECTED VALUE %d IN SWITCH STATEMENT\n", pkt_data.coderate);
        }
        if (pkt_data.no_crc == false) {
            buff[9] |= 0x80; /* set 'CRC enable' bit */
        } else {
            print("Info: packet will be sent without CRC\n");
        }

        /* metadata 10, payload size */
        buff[10] = pkt_data.size;

        /* metadata 11, implicit header, modulation bandwidth, PPM offset & polarity */
        switch (pkt_data.bandwidth) {
            case BW_125KHZ: buff[11] = 0; break;
            case BW_250KHZ: buff[11] = 1; break;
            case BW_500KHZ: buff[11] = 2; break;
            default: print("ERROR: UNEXPECTED VALUE %d IN SWITCH STATEMENT\n", pkt_data.bandwidth);
        }
        if (pkt_data.no_header == true) {
            buff[11] |= 0x04; /* set 'implicit header' bit */
        }
        if (SET_PPM_ON(pkt_data.bandwidth,pkt_data.datarate)) {
            buff[11] |= 0x08; /* set 'PPM offset' bit at 1 */
        }
        if (pkt_data.invert_pol == true) {
            buff[11] |= 0x10; /* set 'TX polarity' bit at 1 */
        }

        /* metadata 12 & 13, LoRa preamble size */
       if (pkt_data.preamble == 0) { /* if not explicit, use recommended LoRa preamble size */
           pkt_data.preamble = STD_LORA_PREAMBLE;
       } else if (pkt_data.preamble < MIN_LORA_PREAMBLE) { /* enforce minimum preamble size */
           pkt_data.preamble = MIN_LORA_PREAMBLE;
           print("Note: preamble length adjusted to respect minimum LoRa preamble size\n");
       }
       buff[12] = 0xFF & (pkt_data.preamble >> 8);
       buff[13] = 0xFF & pkt_data.preamble;

       /* metadata 14 & 15, not used */
       buff[14] = 0;
       buff[15] = 0;

       /* MSB of RF frequency is now used in AGC firmware to implement large/narrow filtering in SX1257/55 */
       buff[0] &= 0x3F; /* Unset 2 MSBs of frequency code */
       if (pkt_data.bandwidth == BW_500KHZ) {
           buff[0] |= 0x80; /* Set MSB bit to enlarge analog filter for 500kHz BW */
       }


    } else if (pkt_data.modulation == MOD_FSK) {
        /* metadata 7, modulation type, radio chain selection and TX power */
        buff[7] = (0x20 & (pkt_data.rf_chain << 5)) | 0x10 | (0x0F & pow_index); /* bit 4 is 1 -> FSK modulation */

        buff[8] = 0; /* metadata 8, not used */

        /* metadata 9, frequency deviation */
        buff[9] = pkt_data.f_dev;

        /* metadata 10, payload size */
        buff[10] = pkt_data.size;
        /* TODO: how to handle 255 bytes packets ?!? */

        /* metadata 11, packet mode, CRC, encoding */
        buff[11] = 0x01 | (pkt_data.no_crc?0:0x02) | (0x00 << 2); /* always in variable length packet mode, whitening, and CCITT CRC if CRC is not disabled  */

        /* metadata 12 & 13, FSK preamble size */
        if (pkt_data.preamble == 0) { /* if not explicit, use LoRa MAC preamble size */
            pkt_data.preamble = STD_FSK_PREAMBLE;
        } else if (pkt_data.preamble < MIN_FSK_PREAMBLE) { /* enforce minimum preamble size */
            pkt_data.preamble = MIN_FSK_PREAMBLE;
            print("Note: preamble length adjusted to respect minimum FSK preamble size\n");
        }
        buff[12] = 0xFF & (pkt_data.preamble >> 8);
        buff[13] = 0xFF & pkt_data.preamble;

        /* metadata 14 & 15, FSK baudrate */
        fsk_dr_div = (uint16_t)((uint32_t)LGW_XTAL_FREQU / pkt_data.datarate); /* Ok for datarate between 500bps and 250kbps */
        buff[14] = 0xFF & (fsk_dr_div >> 8);
        buff[15] = 0xFF & fsk_dr_div;

        /* insert payload size in the packet for variable mode */
        buff[16] = pkt_data.size;
        ++transfer_size; /* one more byte to transfer to the TX modem */
        ++payload_offset; /* start the payload with one more byte of offset */

        /* MSB of RF frequency is now used in AGC firmware to implement large/narrow filtering in SX1257/55 */
        buff[0] &= 0x7F; /* Always use narrow band for FSK (force MSB to 0) */

    } else{
        return FALSE;
    }

    /* Configure TX start delay based on TX notch filter */
    lgw_reg_w(LGW_TX_START_DELAY, tx_start_delay);

    /* copy payload from user struct to buffer containing metadata */
    memcpy((void *)(buff + payload_offset), (void *)(pkt_data.payload), pkt_data.size);

    /* reset TX command flags */
    lgw_reg_w(LGW_TX_TRIG_ALL, 0);

    /* put metadata + payload in the TX data buffer */
    lgw_reg_w(LGW_TX_DATA_BUF_ADDR, 0);
    lgw_reg_wb(LGW_TX_DATA_BUF_DATA, buff, transfer_size);


    switch(pkt_data.tx_mode) {
        case IMMEDIATE:
            lgw_reg_w(LGW_TX_TRIG_IMMEDIATE, 1);
            break;

        case TIMESTAMPED:
            lgw_reg_w(LGW_TX_TRIG_DELAYED, 1);
            break;

        case ON_GPS:
            lgw_reg_w(LGW_TX_TRIG_GPS, 1);
            break;

        default:
            print("ERROR: UNEXPECTED VALUE %d IN SWITCH STATEMENT\n", pkt_data.tx_mode);
            return FALSE;
    }

    return TRUE;

}

// to fetch packets if any was received
uint8 lgw_receive(uint8_t max_pkt, struct lgw_pkt_rx_s *pkt_data)
{
    int nb_pkt_fetch; /* loop variable and return value */
    struct lgw_pkt_rx_s *p; /* pointer to the current structure in the struct array */
    unsigned sz; /* size of the payload, uses to address metadata */
    int ifmod; /* type of if_chain/modem a packet was received by */
    int stat_fifo; /* the packet status as indicated in the FIFO */
    uint32_t raw_timestamp; /* timestamp when internal 'RX finished' was triggered */
    uint32_t delay_x, delay_y, delay_z; /* temporary variable for timestamp offset calculation */
    uint32_t timestamp_correction; /* correction to account for processing delay */
    uint32_t sf, cr, bw_pow, crc_en, ppm; /* used to calculate timestamp correction */

    memset(buff, 0, 255+RX_METADATA_NB); /* buffer to store the result of SPI read bursts */


    /* check if the concentrator is running */
    if (lgw_is_started == false) {
        print("ERROR: CONCENTRATOR IS NOT RUNNING, START IT BEFORE RECEIVING\n");
        return FALSE;
    }

    /* check input variables */
    if ((max_pkt <= 0) || (max_pkt > LGW_PKT_FIFO_SIZE)) {
        print("ERROR: %d = INVALID MAX NUMBER OF PACKETS TO FETCH\n", max_pkt);
        return FALSE;
    }

    if(pkt_data == NULL){
        return FALSE;
    }

    /* Initialize buffer */
    memset (buff, 0, sizeof buff);

    /* iterate max_pkt times at most */
    for (nb_pkt_fetch = 0; nb_pkt_fetch < max_pkt; ++nb_pkt_fetch) {

        /* point to the proper struct in the struct array */
        p = &pkt_data[nb_pkt_fetch];

        /* fetch all the RX FIFO data */
        lgw_reg_rb(LGW_RX_PACKET_DATA_FIFO_NUM_STORED, buff, 5);
        /* 0:   number of packets available in RX data buffer */
        /* 1,2: start address of the current packet in RX data buffer */
        /* 3:   CRC status of the current packet */
        /* 4:   size of the current packet payload in byte */

        /* how many packets are in the RX buffer ? Break if zero */
        if (buff[0] == 0) {
            break; /* no more packets to fetch, exit out of FOR loop */
        }

        /* sanity check */
        if (buff[0] > LGW_PKT_FIFO_SIZE) {
            print("WARNING: %u = INVALID NUMBER OF PACKETS TO FETCH, ABORTING\n", buff[0]);
            break;
        }

        print("FIFO content: %x %x %x %x %x\n", buff[0], buff[1], buff[2], buff[3], buff[4]);

        p->size = buff[4];
        sz = p->size;
        stat_fifo = buff[3]; /* will be used later, need to save it before overwriting buff */

        /* get payload + metadata */
        lgw_reg_rb(LGW_RX_DATA_BUF_DATA, buff, sz+RX_METADATA_NB);

        /* copy payload to result struct */
        memcpy((void *)p->payload, (void *)buff, sz);

        /* process metadata */
        p->if_chain = buff[sz+0];
        if (p->if_chain >= LGW_IF_CHAIN_NB) {
            print("WARNING: %u NOT A VALID IF_CHAIN NUMBER, ABORTING\n", p->if_chain);
            break;
        }
        ifmod = ifmod_config[p->if_chain];
        print("[%d %d]\n", p->if_chain, ifmod);

        p->rf_chain = (uint8_t)if_rf_chain[p->if_chain];
        p->freq_hz = (uint32_t)((int32_t)rf_rx_freq[p->rf_chain] + if_freq[p->if_chain]);
        p->rssi = (float)buff[sz+5] + rf_rssi_offset[p->rf_chain];

        if ((ifmod == IF_LORA_MULTI) || (ifmod == IF_LORA_STD)) {
            print("Note: LoRa packet\n");
            switch(stat_fifo & 0x07) {
                case 5:
                    p->status = STAT_CRC_OK;
                    crc_en = 1;
                    break;
                case 7:
                    p->status = STAT_CRC_BAD;
                    crc_en = 1;
                    break;
                case 1:
                    p->status = STAT_NO_CRC;
                    crc_en = 0;
                    break;
                default:
                    p->status = STAT_UNDEFINED;
                    crc_en = 0;
            }
            p->modulation = MOD_LORA;
            p->snr = ((float)((int8_t)buff[sz+2]))/4;
            p->snr_min = ((float)((int8_t)buff[sz+3]))/4;
            p->snr_max = ((float)((int8_t)buff[sz+4]))/4;
            if (ifmod == IF_LORA_MULTI) {
                p->bandwidth = BW_125KHZ; /* fixed in hardware */
            } else {
                p->bandwidth = lora_rx_bw; /* get the parameter from the config variable */
            }
            sf = (buff[sz+1] >> 4) & 0x0F;
            switch (sf) {
                case 7: p->datarate = DR_LORA_SF7; break;
                case 8: p->datarate = DR_LORA_SF8; break;
                case 9: p->datarate = DR_LORA_SF9; break;
                case 10: p->datarate = DR_LORA_SF10; break;
                case 11: p->datarate = DR_LORA_SF11; break;
                case 12: p->datarate = DR_LORA_SF12; break;
                default: p->datarate = DR_UNDEFINED;
            }
            cr = (buff[sz+1] >> 1) & 0x07;
            switch (cr) {
                case 1: p->coderate = CR_LORA_4_5; break;
                case 2: p->coderate = CR_LORA_4_6; break;
                case 3: p->coderate = CR_LORA_4_7; break;
                case 4: p->coderate = CR_LORA_4_8; break;
                default: p->coderate = CR_UNDEFINED;
            }

            /* determine if 'PPM mode' is on, needed for timestamp correction */
            if (SET_PPM_ON(p->bandwidth,p->datarate)) {
                ppm = 1;
            } else {
                ppm = 0;
            }

            /* timestamp correction code, base delay */
            if (ifmod == IF_LORA_STD) { /* if packet was received on the stand-alone LoRa modem */
                switch (lora_rx_bw) {
                    case BW_125KHZ:
                        delay_x = 64;
                        bw_pow = 1;
                        break;
                    case BW_250KHZ:
                        delay_x = 32;
                        bw_pow = 2;
                        break;
                    case BW_500KHZ:
                        delay_x = 16;
                        bw_pow = 4;
                        break;
                    default:
                        print("ERROR: UNEXPECTED VALUE %d IN SWITCH STATEMENT\n", p->bandwidth);
                        delay_x = 0;
                        bw_pow = 0;
                }
            } else { /* packet was received on one of the sensor channels = 125kHz */
                delay_x = 114;
                bw_pow = 1;
            }

            /* timestamp correction code, variable delay */
            if ((sf >= 6) && (sf <= 12) && (bw_pow > 0)) {
                if ((2*(sz + 2*crc_en) - (sf-7)) <= 0) { /* payload fits entirely in first 8 symbols */
                    delay_y = ( ((1<<(sf-1)) * (sf+1)) + (3 * (1<<(sf-4))) ) / bw_pow;
                    delay_z = 32 * (2*(sz+2*crc_en) + 5) / bw_pow;
                } else {
                    delay_y = ( ((1<<(sf-1)) * (sf+1)) + ((4 - ppm) * (1<<(sf-4))) ) / bw_pow;
                    delay_z = (16 + 4*cr) * (((2*(sz+2*crc_en)-sf+6) % (sf - 2*ppm)) + 1) / bw_pow;
                }
                timestamp_correction = delay_x + delay_y + delay_z;
            } else {
                timestamp_correction = 0;
                print("WARNING: invalid packet, no timestamp correction\n");
            }

            /* RSSI correction */
            if (ifmod == IF_LORA_MULTI) {
                p->rssi -= RSSI_MULTI_BIAS;
            }

        } else if (ifmod == IF_FSK_STD) {
            print("Note: FSK packet\n");
            switch(stat_fifo & 0x07) {
                case 5:
                    p->status = STAT_CRC_OK;
                    break;
                case 7:
                    p->status = STAT_CRC_BAD;
                    break;
                case 1:
                    p->status = STAT_NO_CRC;
                    break;
                default:
                    p->status = STAT_UNDEFINED;
                    break;
            }
            p->modulation = MOD_FSK;
            p->snr = -128.0;
            p->snr_min = -128.0;
            p->snr_max = -128.0;
            p->bandwidth = fsk_rx_bw;
            p->datarate = fsk_rx_dr;
            p->coderate = CR_UNDEFINED;
            timestamp_correction = ((uint32_t)680000 / fsk_rx_dr) - 20;

            /* RSSI correction */
            p->rssi = RSSI_FSK_POLY_0 + RSSI_FSK_POLY_1 * p->rssi + RSSI_FSK_POLY_2 * pow(p->rssi, 2);
        } else {
            print("ERROR: UNEXPECTED PACKET ORIGIN\n");
            p->status = STAT_UNDEFINED;
            p->modulation = MOD_UNDEFINED;
            p->rssi = -128.0;
            p->snr = -128.0;
            p->snr_min = -128.0;
            p->snr_max = -128.0;
            p->bandwidth = BW_UNDEFINED;
            p->datarate = DR_UNDEFINED;
            p->coderate = CR_UNDEFINED;
            timestamp_correction = 0;
        }

        raw_timestamp = (uint32_t)buff[sz+6] + ((uint32_t)buff[sz+7] << 8) + ((uint32_t)buff[sz+8] << 16) + ((uint32_t)buff[sz+9] << 24);
        p->count_us = raw_timestamp - timestamp_correction;
        p->crc = (uint16_t)buff[sz+10] + ((uint16_t)buff[sz+11] << 8);

        /* advance packet FIFO */
        lgw_reg_w(LGW_RX_PACKET_DATA_FIFO_NUM_STORED, 0);
    }

    return nb_pkt_fetch;

}

void sx1301init(void){

    sxResetPin(TRUE);
    wait_ms(10);
    sxResetPin(FALSE);
    wait_ms(10);


    rxrf_init();
    rxif_init();
    tx_init();


    lgw_start();

    Delay(100);





}


