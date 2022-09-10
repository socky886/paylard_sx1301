#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include "print.h"
#include "sys_config.h"

extern int vsprintf_r (const char *s, const char *format, va_list ap);
#ifdef ALLOW_PRINT_INFORMATION
static char buf[1024];
#endif

const static char* ases_logo[] = {"\n                 EEEE           EEEEEEEEEEEEE   EEEEEEEEEEEEEEEEEEE  EEEEEEEEEEEEEE\n",
						"                EEEEEE        EEEEEEEEEEEEEEE   EEEEEEEEEEEEEEEEE  EEEEEEEEE   LEEE\n",
						"               EEEEEEE      tEEEEEEE            EEEEEEG           EEEEEEE          \n",
						"              EEEEEEEEE     EEEEEEEE            EEEEEEL          EEEEEEEE          \n",
						"             EEEEEEEEEEE    EEEEEEEE            EEEEEEL           EEEEEEED         \n",
						"            DDDDLDDDDDDD     DDDDDDDDDD         DDDDDDf           DDDDDDDDDDD      \n",
						"           DDDD  DDDDDDDD     DDDDDDDDDDEE      EEEEEED            DDDDDDDDDDDDD   \n",
						"          GGGG    GGGGGGG      EEEEEEEEEEEEE    EEEEEEEEEEEEEEEEE    EEGGGGGGGGGGG \n",
						"         GGGG     GGGGGGEE        EEEEEEEEEEEE  EEEEEEEEEEEEEEEEE       EEEEEGGGGGG\n",
						"        fLLL       GEEEEEEE          EEEEEEEEEE EEEEEEEEEEEEEEEEE          EEEEEEELL\n",
						"       ffff         DDDDDDD            DDDDDDDD DDDDDDG                      DDDDDDD \n",
						"      f/   ;Lffffff DDDDDDDD             DDDDDD DDDDDDG                       DDDDDDD \n",
						"        GLLLLL       DDDDDDDi            DDDDDD DDDDDDG                        DDDDDD \n",
						"      GGGGL          GDDDDDDD            DDDDDD DDDDDDG                       DDDDDD  \n",
						"    :DGGG             GGGGGGGGGD       GGGGGGG  GGGGGGGGGGGGGGGGG GGG       GGGGGGG   \n",
						"   DDDDG               GGGGGGGGGGGGGGGGGGGGGG   GGGGGGGGGGGGGGGGG GGGGGGGGGGGGGGGG    \n",
						"  DDDDD                GGGGGGGGGGGGGGGGGG         GGGGGGGGGGGGGGG GGGGGGGGGGGG        \n"};


void
print_logo(void)
{
	int i;
	for(i = 0; i < 17; i++) {
		print(ases_logo[i]);
	}
}


int
fputc_r (int c)
{
#ifdef ALLOW_PRINT_INFORMATION
  sciSendByte(sciREG, c);
#endif
  return 1;
}

size_t
fwrite_r (const void *buf, size_t count)
{
  size_t i = 0;
  const char *p = buf;
  for (i = 0; i < count; i++)
  {
    fputc_r (p[i]);
    if ('\n' == p[i])
    {
      fputc_r ('\r');
    }
  }
  return i;
}

int
print (const char *format, ...)
{
#ifdef ALLOW_PRINT_INFORMATION
  int ans;

  va_list ap;

  memset(buf, 0, sizeof(buf));

  va_start (ap, format);
  ans = vsprintf(buf, format, ap);
  va_end (ap);

  fwrite_r (buf, ans);
  return ans;
#else
  return 0;
#endif
}


void
print_mem (const unsigned char *mem, size_t len, size_t line_length)
{
#ifdef ALLOW_PRINT_INFORMATION
  size_t i = 0;
  print ("\n");
  for (i = 0; i < len; i++)
  {
    if (0 == i % line_length)
    {
      if (0 != i)
      {
        print ("\n");
      }
      print ("[%08X-%04X]", (uint32)mem + i, i);
    }
    print (" %02X", 0xFF & mem[i]);

  }
  print ("\n");
#endif
}
