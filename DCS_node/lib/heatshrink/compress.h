/*
 * compress.h
 *
 *  Created on: 2018年4月9日
 *      Author: ases-jack
 */

#ifndef LIB_HEATSHRINK_COMPRESS_H_
#define LIB_HEATSHRINK_COMPRESS_H_

#include "types_def.h"
#include "heatshrink_encoder.h"
#include "heatshrink_decoder.h"

#define DEF_DECODER_INPUT_BUFFER_SIZE 220

#define DATA_IS_COMPRESSED  0x55
#define DATA_IS_ORIGINAL    0xAA


extern uint32_t compress(uint8_t *input, uint32_t input_size, uint8_t *output, uint32_t output_size);

extern uint32_t decompress(uint8_t *input, uint32_t input_size, uint8_t *output, uint32_t output_size);


#endif /* LIB_HEATSHRINK_COMPRESS_H_ */
