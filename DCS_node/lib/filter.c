/*
 * filter.c
 *
 *  Created on: 2018年4月20日
 *      Author: ases-jack
 */

#include "filter.h"
#include "FreeRTOS.h"
#include "os_task.h"
#include "sys_config.h"
#include "string.h"

static AD_FILTER_INFO ad_filter[MAX_AD_CHN];

static uint16 data_sort_buf[ALL_DATA_COUNT];

void InitFilter(void)
{
	memset(ad_filter, 0, sizeof(ad_filter));
}

static void adjustHeap(int param1, int j, uint16 inNums[])
{
	int i;
    uint16 temp = inNums[param1];

    for (i = param1 * 2 + 1 ; i < j ; i = i * 2 + 1)
    {
        //如果右边值大于左边值，指向右边
        if (((i + 1) < j) && (inNums[i]< inNums[i + 1]))
        {
            i++;
        }
        //如果子节点大于父节点，将子节点值赋给父节点,并以新的子节点作为父节点（不用进行交换）
        if (inNums[i] > temp)
        {
            inNums[param1] = inNums[i];
            param1 = i;
        }
        else
            break;
    }
    //put the value in the final position
    inNums[param1] = temp;
}
/* 从小到大排序 */
static void HeapSort(uint16 chn)
{
	int i;
	uint16 temp;

	memcpy(data_sort_buf, ad_filter[chn].ad_value, sizeof(data_sort_buf));
    //1.构建大顶堆
    for (i = ALL_DATA_COUNT / 2 - 1 ; i >= 0 ; i--)
    {
        //put the value in the final position
        adjustHeap(i, ALL_DATA_COUNT, data_sort_buf);
    }
    //2.调整堆结构+交换堆顶元素与末尾元素
    for (i = ALL_DATA_COUNT-1 ; i > 0 ; i--)
    {
        //堆顶元素和末尾元素进行交换
        temp = data_sort_buf[0];
        data_sort_buf[0] = data_sort_buf[i];
        data_sort_buf[i] = temp;

        adjustHeap(0, i, data_sort_buf);//重新对堆进行调整
    }

    ad_filter[chn].sum = 0;
	for(i = REMOVE_DATA_COUNT; i < (ALL_DATA_COUNT - REMOVE_DATA_COUNT); i++) {
		ad_filter[chn].sum += data_sort_buf[i];
	}
}

void AddNewAdData(uint16 chn, uint16 data)
{
	//taskENTER_CRITICAL();
	ad_filter[chn].ad_value[ad_filter[chn].pos] = data;
	ad_filter[chn].pos = (ad_filter[chn].pos + 1) % ALL_DATA_COUNT;
	HeapSort(chn);
	//taskEXIT_CRITICAL();
}

uint16 GetFilteredData(uint16 chn)
{
	return (ad_filter[chn].sum / (ALL_DATA_COUNT - REMOVE_DATA_COUNT * 2));
}
