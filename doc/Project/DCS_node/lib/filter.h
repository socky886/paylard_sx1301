/*
 * filter.h
 *
 *  Created on: 2018年4月20日
 *      Author: ases-jack
 */

#ifndef LIB_FILTER_H_
#define LIB_FILTER_H_

#include "types_def.h"

#define ALL_DATA_COUNT   20
#define REMOVE_DATA_COUNT  5    /* Highest 5, lowest 5 */

typedef struct {
	uint16 ad_value[ALL_DATA_COUNT];
	uint32 sum;
	uint16 pos;
}AD_FILTER_INFO;

extern void InitFilter(void);

extern void AddNewAdData(uint16 chn, uint16 data);

extern uint16 GetFilteredData(uint16 chn);

#endif /* LIB_FILTER_H_ */
