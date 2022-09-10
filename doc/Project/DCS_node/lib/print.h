#ifndef PRINT_H_
#define PRINT_H_

#include <stdio.h>
#include "sci.h"
#include "spi.h"
#include "types_def.h"


#define ALLOW_PRINT_INFORMATION
//#undef ALLOW_PRINT_INFORMATION

extern void print_logo(void);

extern int print (const char *format, ...);

extern void print_mem (const unsigned char *mem, size_t len, size_t line_length);
#endif //PRINT_H_
