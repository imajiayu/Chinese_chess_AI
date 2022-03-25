#include <cstdio>
#include "position.h"

#ifndef BOOKDATA_H
#define BOOKDATA_H

// 获取开局库着法
class BookDataClass
{
public:
	struct BookStruct
	{
		uint64_t ZobristLock;//校验码
		uint16_t mv;
		int16_t value;	  //动作和评估值
	} Books[MAX_GEN_MOVES];
	//BookDataClass();

	/**
	 * @brief 		从开局库中提取走法
	 * 
	 * @param pos 	当前棋盘
	 * @return int 	开局数量，提取到的内容存放在Books的变量中
	 */
	int GetBookMoves(const PositionClass &pos);
	//~BookDataClass();
private:
	int BOOK_POS_CMP(const BookStruct& bk, const ZobristStruct& pos_zo);
};



#endif

