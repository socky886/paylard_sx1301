#include "queue.h"
#include "string.h"

/**
 * @brief 队列是否为空
 * @param 队列信息
 * @return 如果队列为空则返回True，否则返回False
 */
bool_t
EmptyQueue (const TypeQueue * q)
{
  return (0 == q->len);
}

/**
 * @brief 队列是否为满
 * @param 队列信息
 * @return 如果队列为满则返回True，否则返回False
 *
 */
bool_t
FullQueue (const TypeQueue * q)
{
  return (q->max == q->len);
}

/**
 * @brief 取队列首元素
 * @param 队列信息
 * @return 返回队列首元素指针
 *
 */
TypeQueueElem *
FrontQueue (const TypeQueue * q)
{
  return &((uint8 *) q->data)[q->head * q->esize];
}

/**
 * @brief 删除队列开头一个元素
 * @param 队列信息
 * @return 无
 *
 */
void
PopQueue (TypeQueue * q)
{
  q->head = (q->head + 1) % q->max;
  q->len--;
}

/**
 * @brief 压入队列一个元素
 * @param 需要压入的元素指针
 * @param 队列信息
 * @return 无
 *
 */
void
PushQueue (const TypeQueueElem * elem, TypeQueue * q)
{
  uint32 pos = (q->head + q->len) % q->max;
  memcpy (&((uint8 *) q->data)[pos * q->esize], elem, q->esize);
  q->len++;
}

/**
 * @brief 弹出队列最早的元素
 * @param 队列信息
 * @param 元素指针
 * @return 如果弹出成功则返回True，否则返回False
 *
 */
bool_t
PopQueueEx (TypeQueue * q, TypeQueueElem * elem)
{
  bool_t r = FALSE;
  if (FALSE == EmptyQueue (q))
  {
    TypeQueueElem *t = FrontQueue (q);
    memcpy (elem, t, q->esize);
    PopQueue (q);
    r = TRUE;
  }
  return r;
}

/**
 * @brief 压入队列一个元素
 * @param 元素指针
 * @param 队列信息
 * @return 如果压入队列成功则返回True，否则返回False
 *
 */
bool_t
PushQueueEx (const TypeQueueElem * elem, TypeQueue * q)
{
  bool_t r = FALSE;
  if (FALSE == FullQueue (q))
  {
    PushQueue (elem, q);
    r = TRUE;
  }
  return r;
}

/**
 * @brief 压入一个元素，并删除最早的元素
 * @param 元素指针
 * @param 队列信息
 * @return 无
 *
 */
void
PushQueueOverWriteEx (const TypeQueueElem * elem, TypeQueue * q)
{
  if (TRUE == FullQueue (q))
  {
    PopQueue (q);
  }
  PushQueue (elem, q);
}

/**
 * @fn void InitQueue (TypeQueue * q, TypeQueueElem * elems, size_t esize, uint32 max)
 * @brief 初始化FIFO队列
 *
 * @param q 队列信息结构体
 * @param elems 存储队列数据的缓冲区
 * @param esize 队列中单个元素的长度，单位字节
 * @param max 队列中最大元素个数
 *
 * @return 无
 */
void
InitQueue (TypeQueue * q, TypeQueueElem * elems, size_t esize, uint32 max)
{
  memset(elems, 0, esize * max);
  q->head = 0;
  q->len = 0;
  q->max = max;
  q->data = elems;
  q->esize = esize;
}
