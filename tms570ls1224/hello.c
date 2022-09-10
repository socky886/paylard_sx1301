#include <stdio.h>

#include "sci.h"
#include "gio.h"
#include "spi.h"
#include "het.h"
#include "sci.h"

#include <string.h>

/**
 * hello.c
 */
int main(void)
{
	char xx[]="serial print test\n";
    sciInit();
    gioInit();
    spiInit();
    hetInit();
	while (1)
	{
		sciSendByte(sciREG,'w');
		//sciSend(sciREG,strlen(xx),(uint8_t *)xx);

	}
	
	// printf("Hello World!\n");
	
	return 0;
}
