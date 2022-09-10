
#include "sys_common.h"

#include <stdio.h>
#include "sci.h"
#include "spi.h"
// #include "types_def.h"

// static char buf[1024];
// int fputc_r(int c)
// {
// 	sciSendByte(sciREG, c);
// 	return 1;
// }

// size_t
// fwrite_r(const void *buf, size_t count)
// {
// 	size_t i = 0;
// 	const char *p = buf;
// 	for (i = 0; i < count; i++)
// 	{
// 		fputc_r(p[i]);
// 		if ('\n' == p[i])
// 		{
// 			fputc_r('\r');
// 		}
// 	}
// 	return i;
// }

// int printf(const char *format, ...)
// {
//   int ans;

//   va_list ap;

//   memset(buf, 0, sizeof(buf));

//   va_start (ap, format);
//   ans = vsprintf(buf, format, ap);
//   va_end (ap);

//   fwrite_r (buf, ans);
//   return ans;

// }

// weijunfeng added 2022-06-13
// alter this function to delay_ms
// the result is ok by test
void Delay(volatile int time)
{
	int temp = time;
	while(time--){
		volatile int mil_time = 10000;
		while(mil_time--);
	}
}

/**
 * main.c
 */
int main(void)
{
	sciInit();
	gioInit();
	spiInit();
	while (1)
	{
		sciSendByte(sciREG, 'w');
		sciSendByte(sciREG, 'j');
		sciSendByte(sciREG, '\n');
		Delay(200);
	}
	
	return 0;
}
