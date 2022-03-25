#include <cstring>
#include "parameters.h"
#include "position.h"

#ifndef HASH_H
#define HASH_H

// 置换表结构，置换表信息夹在两个Zobrist校验锁中间，可以防止存取冲突
class HashClass
{
	struct HashStruct
	{
		uint8_t nDepth, nFlag;	   // 深度，标志位
		int16_t value;			   // 分值
		uint16_t mv, xReserved;	   // 最佳着法，内存填充
		uint64_t dwKey;			   // Zobrist校验锁
	} Table[HASH_SIZE];

public:
	//HashClass();

	void Clear(); // 清空置换表

	/**
	 * @brief 			即ProbeHash()
	 * 
	 * @param pos     	目前棋盘
	 * @param vlAlpha 	当前alpha
	 * @param vlBeta  	当前beta
	 * @param nDepth  	当前深度
	 * @param mv 	  	获取到的最优动作
	 * @return int 		获取到的分值，如果查不到或不符合则返回-MATE_VALUE
	 */

	int Load(const PositionClass &pos, int vlAlpha, int vlBeta, int nDepth, int &mv, bool bUseNull = false); // 提取置换表项
	/**
	 * @brief 			即RecordHash()
	 * 
	 * @param pos 		目前棋盘
	 * @param nFlag 	当前节点状态，参照parameters.h
	 * @param vl 		当前分值
	 * @param nDepth	当前深度
	 * @param mv 		最优动作
	 */
	bool Save(const PositionClass &pos, int nFlag, int vl, int nDepth, int mv); // 保存置换表项
	//~HashClass();
};

#endif