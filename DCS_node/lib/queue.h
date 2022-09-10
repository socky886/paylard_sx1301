#ifndef QUEUE_H_
#define QUEUE_H_

#include "types_def.h"
#include "hal_stdtypes.h"


typedef struct
{
  uint32 in;
  uint32 out;
  uint32 err;
} TypeQueueLog;

typedef void TypeQueueElem;

typedef struct
{
  uint32 head;
  uint32 len;
  uint32 max;
  TypeQueueElem *data;
  uint32 esize;
} TypeQueue;

extern void InitQueue (TypeQueue * q, TypeQueueElem * elems, size_t esize, uint32 max);

extern bool_t EmptyQueue (const TypeQueue * q);

extern bool_t FullQueue (const TypeQueue * q);

extern TypeQueueElem *FrontQueue (const TypeQueue * q);

extern void PopQueue (TypeQueue * q);

extern void PushQueue (const TypeQueueElem * elem, TypeQueue * q);

extern bool_t PopQueueEx (TypeQueue * q, TypeQueueElem * elem);

extern bool_t PushQueueEx (const TypeQueueElem * elem, TypeQueue * q);

extern void PushQueueOverWriteEx (const TypeQueueElem * elem, TypeQueue * q);

#endif //QUEUE_H_
