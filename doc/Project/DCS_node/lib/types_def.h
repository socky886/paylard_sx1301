/*
 * types_def.h
 *
 *  Created on: 2018/1/15
 *      Author: ases-jack
 */

#ifndef LIB_TYPES_DEF_H_
#define LIB_TYPES_DEF_H_

#include <stdio.h>
#include "hal_stdtypes.h"
#include "sys_config.h"

#ifndef _BOOL_T
#define _BOOL_T
typedef unsigned char bool_t;
#endif

//#ifndef _SIZE_T
//#define _SIZE_T
//typedef unsigned int size_t;
//#endif

#ifndef addr_t
typedef uint32 addr_t;
#endif

typedef enum {RW_A = 0x55, RW_B = 0xAA}   RW_CHOOSE_TYPE;

#endif /* LIB_TYPES_DEF_H_ */
