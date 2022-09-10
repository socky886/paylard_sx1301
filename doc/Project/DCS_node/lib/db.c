/*
 * db.c
 *
 *  Created on: 2014年1月1日
 *      Author: 805user
 */

#include "db.h"
#include "safe.h"
/*
 * 根据关键字查找索引代号
 * @param uint16 key 关键字
 * @param TypeDb * db 数据库
 * @return 数据库索引表ID
 */
uint8 FindFromDb (uint16 key, TypeDb * db)
{
  uint8 i = 0;
  uint8 r = 0xFF;

  for (i = 0; i < db->ilen; i++)
  {
    if (key == db->index[i].key)
    {
      r = i;
      break;
    }
  }
  return (r);
}

/*
 * 根据key获取包在数据库中存储地址
 * @param int index 数据库索引位置
 * @param TypeDb * db数据库
 * @return TypeDbIndex* 数据库索引单元
 */
TypeDbIndex *
GetDb (int index, TypeDb * db)
{
  return &db->index[index];
}

/*
 * 数据库索引长度获取
 * @param TypeDb * db 数据库
 * @return 索引表长度
 */
int
GetILenDb (TypeDb * db)
{
  return db->ilen;
}

/*
 * 根据关键字查找索引代号
 * @param int index 数据库索引代号
 * @param TypeDb * db 数据库
 * @return 关键字
 */
uint16
GetIndexKeyDb (int index, TypeDb * db)
{
  return db->index[index].key;
}

/*
 * 数据库中分配空间
 * @param uint16 key 用于索引的关键字
 * @param TypeDb * db 数据库
 * @param uint32 len 长度
 * @return 分配的位置
 */
static uint8 allocFromDb (uint16 key, TypeDb * db, uint32 len)
{
	uint8 result = 0xFF;

	db->ilen += 1;
	//填充数据库信息块
	db->index[db->ilen - 1].ptr = db->mem + db->mlen;
	db->index[db->ilen - 1].key = key;
	db->index[db->ilen - 1].len = len;
	//更新数据库已有数据长度
	db->mlen = db->mlen + len;
	result = db->ilen - 1;

    //判断是否超过数据库最大空间
    if (db->mlen > db->mmax)
    {
    	result = 0xFF;
    }
  return result;
}

/**
* 数据写入到数据库
* @param uint16 key 遥测APID
* @param *p 遥测包指针
* @param uint32 len 包长
* @param int index 写入索引表位置
* @param TypeDb 数据库
* @return TRUE /FALSE
*/
uint8 WriteDb (uint16 key, void *p, uint32 len, TypeDb * db)
{
  uint8 index = 0xFF;

  //查找数据库索引
  index = FindFromDb (key, db);

  //索引是否有效
  if (0xFF == index)
  {
	  //索引无效则重新数据库申请空间
	  index = allocFromDb (key, db, len);
  }

  if (0xFF != index)
  {
	  //数据库申请成功
      TypeDbIndex *tp = &db->index[index];
      memcpy_s (tp->ptr, db->mmax, p, len);
      tp->freshflg = DB_ENABLE;
  }

  return (index);
}

/**
* 数据写入数据库扩展
* @param uint16 key 遥测APID
* @param *p 遥测包指针
* @param uint32 len 包长
* @param int index 写入索引表位置
* @param TypeDb 数据库
* @return TRUE /FALSE
*/
uint8 WriteDbEx (uint16 key, void *p, uint32 len, uint32 alloclen, uint8 index, TypeDb * db)
{
	//索引是否有效
	if (0xFF == index)
	{
		index = allocFromDb(key, db, alloclen);
	}
	//数据库申请成功
	if (0xFF != index)
	{
		TypeDbIndex *tp = &db->index[index];
		//将数据存入数据库
		memcpy_s (tp->ptr, alloclen, p, len);
		tp->freshflg = DB_ENABLE;
		tp->ulen = len;
	}

	return index;
}

/**
* 数据库初始化
* @param *q 数据库指针
* @param *p 内存缓存指向
* @param max 最大长度
* @return NONE
*/
void
InitDb (TypeDb * q, void *elems, uint32 mmax, TypeDbIndex * t, uint32 imax)
{
  q->mlen = 0;//初始数据库已有数据长度
  q->ilen = 0;//索引个数
  q->imax = imax;//最大索引个数
  q->mem = elems;//给数据库用的空间指针
  q->index = t;//存放索引的数据
  q->mmax = mmax;//数据库最大数据个数
}
