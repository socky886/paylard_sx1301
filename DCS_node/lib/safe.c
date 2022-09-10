#include "safe.h"
#include "types_def.h"
#include <string.h>
#include "errorlog.h"

void *
wmemset (void *s, unsigned int c, int n)
{
  unsigned int *sb = s;
  while (n--)
    *sb++ = c;

  return s;
}

bool_t
memcpy_s (void *dest, size_t n1, const void *src, size_t n2)
{
  bool_t r = TRUE;
  if (n1 < n2)
  {
    r = FALSE;
    ErrorLog(__LINE__, (uint32)memcpy_s, n1);
  }
  else
  {
    memcpy (dest, src, n2);
  }
  return r;
}

bool_t
memmove_s (void *dest, size_t n1, const void *src, size_t n2)
{
  bool_t r = TRUE;
  if (n1 < n2)
  {
	ErrorLog(__LINE__, (uint32)memmove_s, n1);
    r = FALSE;
  }
  else
  {
    memmove (dest, src, n2);
  }
  return r;
}

void
SetBit (unsigned char *val, int p, int s)
{
  if (0 == s)
  {
    *val = *val & (~(1 << p));
  }
  else
  {
    *val = *val | (1 << p);
  }
}
