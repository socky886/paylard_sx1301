/*
 * compress.c
 *
 *  Created on: 2018年4月9日
 *      Author: ases-jack
 */
#include "compress.h"
#include "print.h"
#include "errorlog.h"
#include "string.h"

static heatshrink_encoder hse;
static heatshrink_decoder hsd;


/*
uint8 compress(uint8_t *data, size_t data_sz, uint8* out_buf, size_t out_sz, size_t* compressed_sz)
{
    size_t sink_sz = 0;
    size_t count = 0;
    HSE_sink_res sres;
    HSE_poll_res pres;
    HSE_finish_res fres;

    heatshrink_encoder_reset(&hse);

	sres = heatshrink_encoder_sink(&hse, data, data_sz, &sink_sz);
	if (sres < 0) {
		ErrorLog(__LINE__, (uint32)compress, (uint32)sres);
		print("sink");
	}

	do {
		pres = heatshrink_encoder_poll(&hse, &out_buf[*compressed_sz], out_sz - *compressed_sz, &count);
		if (pres < 0) {
			ErrorLog(__LINE__, (uint32)compress, (uint32)pres);
			print("poll");
		}
		*compressed_sz += count;
	} while (pres == HSER_POLL_MORE);

	fres = heatshrink_encoder_finish(&hse);
	if (fres < 0) {
		ErrorLog(__LINE__, (uint32)compress, (uint32)fres);
		print("finish");
	}
	if (fres == HSER_FINISH_DONE) {
		return TRUE;
	}

    return FALSE;
}

uint8 decompress(uint8_t *data, size_t data_sz, uint8* out_buf, size_t out_sz, size_t* decompressed_sz)
{
	size_t sink_sz = 0;
	size_t count = 0;
    HSD_sink_res sres;
    HSD_poll_res pres;
    HSD_finish_res fres;

	heatshrink_decoder_reset(&hsd);

	sres = heatshrink_decoder_sink(&hsd, data, data_sz, &sink_sz);
	if (sres < 0) {
		ErrorLog(__LINE__, (uint32)decompress, (uint32)sres);
		print("sink");
	}

	do {
		pres = heatshrink_decoder_poll(&hsd, &out_buf[*decompressed_sz], out_sz - *decompressed_sz, &count);
		if (pres < 0) {
			ErrorLog(__LINE__, (uint32)decompress, (uint32)pres);
			print("poll");
		}
		*decompressed_sz += count;
	}while (pres == HSDR_POLL_MORE);

	fres = heatshrink_decoder_finish(&hsd);
	if (fres < 0) {
		ErrorLog(__LINE__, (uint32)decompress, (uint32)fres);
		print("finish");
	}
	if (fres == HSER_FINISH_DONE) {
		return TRUE;
	}
	return FALSE;
}
*/


/*
 * size_t comp_sz = input_size + (input_size/2) + 4;
 */
uint32_t compress(uint8_t *input, uint32_t input_size, uint8_t *output, uint32_t output_size)
{
	uint32_t sunk = 0;
	uint32_t polled = 0;
	size_t count = 0;

	size_t comp_sz = output_size;
	HSE_sink_res sres;
	HSE_poll_res pres;
	HSE_finish_res fres;

	heatshrink_encoder_reset(&hse);

	while (sunk < input_size) {
		sres = heatshrink_encoder_sink(&hse, &input[sunk], input_size - sunk, &count);
		if(sres < 0){
			ErrorLog(__LINE__, (uint32)compress, (uint32)sres);
		}
		sunk += count;
		if (sunk == input_size) {
			fres = heatshrink_encoder_finish(&hse);
			if(fres != HSER_FINISH_MORE) {
				ErrorLog(__LINE__, (uint32)compress, (uint32)fres);
			}
		}

		do {                    /* "turn the crank" */
			pres = heatshrink_encoder_poll(&hse, &output[polled], comp_sz - polled, &count);
			if(pres < 0){
				ErrorLog(__LINE__, (uint32)compress, (uint32)pres);
			}
			polled += count;
		} while (pres == HSER_POLL_MORE);
		if(pres != HSER_POLL_EMPTY) {
			ErrorLog(__LINE__, (uint32)compress, (uint32)pres);
		}
		if (polled >= comp_sz) {
			ErrorLog(__LINE__, (uint32)compress, (uint32)polled);
		}
		if (sunk == input_size) {
			fres = heatshrink_encoder_finish(&hse);
			if(fres != HSER_FINISH_DONE) {
				ErrorLog(__LINE__, (uint32)compress, (uint32)fres);
			}
		}
	}
	return polled;
}
/*
 * size_t decomp_sz = input_size + (input_size/2) + 4;
 */
uint32_t decompress(uint8_t *input, uint32_t input_size, uint8_t *output, uint32_t output_size)
{
	uint32_t sunk = 0;
	uint32_t polled = 0;
    size_t count = 0;

    size_t decomp_sz = output_size;
	HSD_sink_res sres;
	HSD_poll_res pres;
	HSD_finish_res fres;

	heatshrink_decoder_reset(&hsd);

	while (sunk < input_size) {
		sres = heatshrink_decoder_sink(&hsd, &input[sunk], input_size - sunk, &count);
		if(sres < 0) {
			ErrorLog(__LINE__, (uint32)decompress, (uint32)sres);
		}
		sunk += count;
		if (sunk == input_size) {
			fres = heatshrink_decoder_finish(&hsd);
			if(fres != HSDR_FINISH_MORE) {
				ErrorLog(__LINE__, (uint32)decompress, (uint32)fres);
			}
		}

		do {
			pres = heatshrink_decoder_poll(&hsd, &output[polled], decomp_sz - polled, &count);
			if(pres < 0){
				ErrorLog(__LINE__, (uint32)decompress, (uint32)pres);
			}
			polled += count;
		} while (pres == HSDR_POLL_MORE);
		if(pres != HSDR_POLL_EMPTY) {
			ErrorLog(__LINE__, (uint32)decompress, (uint32)pres);
		}
		if (sunk == input_size) {
			HSD_finish_res fres = heatshrink_decoder_finish(&hsd);
			if(fres != HSDR_FINISH_DONE) {
				ErrorLog(__LINE__, (uint32)decompress, (uint32)fres);
			}
		}

		if (polled > output_size) {
			ErrorLog(__LINE__, (uint32)decompress, (uint32)polled);
		}
	}
	return polled;
}

/*
int compress_and_expand_and_check(uint8_t *input, uint32_t input_size, int log_lvl) {
    heatshrink_encoder_reset(&hse);
    heatshrink_decoder_reset(&hsd);
    size_t comp_sz = input_size + (input_size/2) + 4;
    size_t decomp_sz = input_size + (input_size/2) + 4;
    uint8_t *comp = malloc(comp_sz);
    uint8_t *decomp = malloc(decomp_sz);
    if (comp == NULL) FAILm("malloc fail");
    if (decomp == NULL) FAILm("malloc fail");
    memset(comp, 0, comp_sz);
    memset(decomp, 0, decomp_sz);

    size_t count = 0;

    if (log_lvl > 1) {
        printf("\n^^ COMPRESSING\n");
        dump_buf("input", input, input_size);
    }

    uint32_t sunk = 0;
    uint32_t polled = 0;
    while (sunk < input_size) {
        ASSERT(heatshrink_encoder_sink(&hse, &input[sunk], input_size - sunk, &count) >= 0);
        sunk += count;
        if (log_lvl > 1) printf("^^ sunk %zd\n", count);
        if (sunk == input_size) {
            ASSERT_EQ(HSER_FINISH_MORE, heatshrink_encoder_finish(&hse));
        }

        HSE_poll_res pres;
        do {
            pres = heatshrink_encoder_poll(&hse, &comp[polled], comp_sz - polled, &count);
            ASSERT(pres >= 0);
            polled += count;
            if (log_lvl > 1) printf("^^ polled %zd\n", count);
        } while (pres == HSER_POLL_MORE);
        ASSERT_EQ(HSER_POLL_EMPTY, pres);
        if (polled >= comp_sz) FAILm("compression should never expand that much");
        if (sunk == input_size) {
            ASSERT_EQ(HSER_FINISH_DONE, heatshrink_encoder_finish(&hse));
        }
    }
    if (log_lvl > 0) printf("in: %u compressed: %u ", input_size, polled);
    uint32_t compressed_size = polled;
    sunk = 0;
    polled = 0;

    if (log_lvl > 1) {
        printf("\n^^ DECOMPRESSING\n");
        dump_buf("comp", comp, compressed_size);
    }
    while (sunk < compressed_size) {
        ASSERT(heatshrink_decoder_sink(&hsd, &comp[sunk], compressed_size - sunk, &count) >= 0);
        sunk += count;
        if (log_lvl > 1) printf("^^ sunk %zd\n", count);
        if (sunk == compressed_size) {
            ASSERT_EQ(HSDR_FINISH_MORE, heatshrink_decoder_finish(&hsd));
        }

        HSD_poll_res pres;
        do {
            pres = heatshrink_decoder_poll(&hsd, &decomp[polled],
                decomp_sz - polled, &count);
            ASSERT(pres >= 0);
            polled += count;
            if (log_lvl > 1) printf("^^ polled %zd\n", count);
        } while (pres == HSDR_POLL_MORE);
        ASSERT_EQ(HSDR_POLL_EMPTY, pres);
        if (sunk == compressed_size) {
            HSD_finish_res fres = heatshrink_decoder_finish(&hsd);
            ASSERT_EQ(HSDR_FINISH_DONE, fres);
        }

        if (polled > input_size) {
            FAILm("Decompressed data is larger than original input");
        }
    }
    if (log_lvl > 0) printf("decompressed: %u\n", polled);
    if (polled != input_size) {
        FAILm("Decompressed length does not match original input length");
    }

    if (log_lvl > 1) dump_buf("decomp", decomp, polled);
    for (size_t i=0; i<input_size; i++) {
        if (input[i] != decomp[i]) {
            printf("*** mismatch at %zd\n", i);
            if (0) {
                for (size_t j=0; j<= input_size; j++) {
                    printf("in[%zd] == 0x%02x ('%c') => out[%zd] == 0x%02x ('%c')\n",
                        j, input[j], isprint(input[j]) ? input[j] : '.',
                        j, decomp[j], isprint(decomp[j]) ? decomp[j] : '.');
                }
            }
        }
        ASSERT_EQ(input[i], decomp[i]);
    }
    free(comp);
    free(decomp);
    PASS();
}
*/
