#ifndef  MYLIB_H_
#define  MYLIB_H_

#include "types_def.h"

/**
* 按照4字节的倍数memset.
* @param s 内存指针.
* @param c 数据内容
* @param n 次数
*/
extern void *wmemset (void *s, unsigned int c, int n);

/**
* 安全的memcpy.
* @param dest  目标内存指针.
* @param n1    目标内存最大长度
* @param src   源内存指针
* @param n2    源内存长度
*/
extern bool_t memcpy_s (void *dest, size_t n1, const void *src, size_t n2);

/**
* 安全的memmove.
* @param dest 目标内存指针.
* @param n1   目标内存最大长度
* @param src  源内存指针
* @param n2   源内存长度
*/
extern bool_t memmove_s (void *dest, size_t n1, const void *src, size_t n2);


/**
 * 设指定位为0或1
 * @param val 待修改值指针
 * @param p 待修改位置
 * @param s 修改内容
 */
extern void SetBit (unsigned char *val, int p, int s);


#endif //MYLIB_H_
