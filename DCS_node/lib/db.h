/*
 * db.h
 *
 *  Created on: 2014年11月21日
 *      Author: 805user
 */

#ifndef DB_H_
#define DB_H_

#include "types_def.h"


#define DB_ENABLE  1
#define DB_DISABLE 0

//源包指针向量存储缓存
typedef struct
{
  //刷新标志
  uint8 freshflg;
  //关键字
  uint16 key;
  //申请长度
  size_t len;
  //使用长度
  size_t ulen;
  //对应指针
  void *ptr;
} __attribute__ ((__packed__)) TypeDbIndex;

//遥测缓存
typedef struct
{
  //数据库字节数上限
  int mmax;
  //缓冲区
  uint8 *mem;
  //长度  当前数据库字节数据长度
  int mlen;
  //索引区最大长度
  int imax;
  //索引区
  TypeDbIndex *index;
  //APID数  元素个数
  int ilen;
} TypeDb;

/*
 * 根据关键字查找索引代号
 * @param uint16 key 关键字
 * @param TypeDb * db 数据库
 * @return 数据库索引表ID
 */
extern uint8 FindFromDb (uint16 key, TypeDb * db);

/*
 * 根据关键字查找索引代号
 * @param int index 数据库索引代号
 * @param TypeDb * db 数据库
 * @return 关键字
 */
extern uint16 GetIndexKeyDb (int index, TypeDb * db);

/*
 * 数据库索引长度获取
 * @param TypeDb * db 数据库
 * @return 索引表长度
 */
extern int GetILenDb (TypeDb * db);

/*
 * 根据key获取包在数据库中存储地址
 * @param int index 数据库索引位置
 * @param TypeDb * db数据库
 * @return TypeDbIndex* 数据库索引单元
 */
extern TypeDbIndex *GetDb (int index, TypeDb * db);

/**
* 数据写入到数据库
* @param uint16 key 遥测APID
* @param *p 遥测包指针
* @param uint32 len 包长
* @param TypeDb 数据库
* @return TRUE /FALSE
*/
extern uint8 WriteDb (uint16 key, void *p, uint32 len, TypeDb * db);

/**
* 数据写入到数据库
* @param uint16 key 遥测APID
* @param *p 遥测包指针
* @param uint32 len 包长
* @param int index 写入索引表位置
* @param TypeDb 数据库
* @return TRUE /FALSE
*/
extern uint8 WriteDbEx (uint16 key, void *p, uint32 len, uint32 alloclen, uint8 index, TypeDb * db);

/**
* 数据库初始化
* @param *q 数据库指针
* @param *p 内存缓存指向
* @param max 最大长度
* @return NONE
*/
extern void InitDb (TypeDb * q, void *elems, uint32 mmax, TypeDbIndex * t, uint32 imax);

#endif /* DB_H_ */
