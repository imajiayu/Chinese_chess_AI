#include "ucci.h"
#include "position.h"
#include "hash.h"
#include "bookdata.h"
#include "parameters.h"
#include "tools.h"
#include <algorithm>
#include <random>
#include<time.h>
#include<iostream>
#ifndef SEARCH_H
#define SEARCH_H


//着法排序的阶段
enum SORT_PHASE { PHASE_HASH, PHASE_KILLER_1, PHASE_KILLER_2, PHASE_GEN_SORT, PHASE_GET };

// 着法序列结构
class MoveSortClass {
	int mvHash, mvKiller1, mvKiller2;	//置换表走法和两个杀手走法
	int phase, index, genMoveNum;		//当前阶段，当前着法的序号，着法总数
	int mvlist[MAX_GEN_MOVES];          //着法列表
	int* historyTable;					//历史表
	PositionClass* pPos;				//指向Pos的指针
public:
	//对MoveSortClass的各项进行初始化
	void Init(int mvHash_, PositionClass* pPosition, int* HistoryTable, unsigned short KillerTable[][2]);
	//根据着法排序策略得到下一个着法
	int NextMove(); 
	//将着法表按历史表排序
	void HistorySort(int* mvlist, int* mvlistend){
		std::sort(mvlist, mvlistend, [this](int mv1, int mv2) {return (historyTable[mv1] > historyTable[mv2]); });
	}
	// 对最佳走法的处理
	inline void SetBestMove(int mv, int depth, unsigned short mvKillerTable[2]) {
		historyTable[mv] += depth * depth;
		if (mvKillerTable[0] != mv) {
			mvKillerTable[1] = mvKillerTable[0];
			mvKillerTable[0] = mv;
		}
	}

};


class SearchClass{
	//成员变量
private:
	HashClass HashTable;					//置换表
	BookDataClass BookData;					//开局库
	PositionClass Pos;						//局面						
	bool isStop = false;				    //中止信号
	bool isDebug = false;					//是否调试状态
	bool useHash = true;					//是否使用置换表
	bool useBook = true;					//是否使用开局库
	int maxTime;							//能使用的最大时间
	long long startTime;                    //开始时间
	int finalMove;							//走棋结果
	int unchangedDepth;                     //最佳着法未改变的深度
	int HistoryTable[65536] = { 0 };		//历史表
	unsigned short KillerTable[LIMIT_DEPTH][2] = { 0 };		//杀手着法表
public:
	//公有成员函数，搜索的主要函数
	int SearchMain(PositionClass& Pos_, UcciCommClass& Ucci);
private:
	//从根节点开始搜索
	int SearchRoot(int depth);
	//主要变例搜索
	int SearchPV(int alpha, int beta, int depth);
	//零窗口主要变例搜索
	int SearchPVCut(int beta, int depth, bool useNull);
	//静态搜索
	int SearchQuiesc(int alpha, int beta);
	//清空历史表
	inline void ClearHistory() {
		memset(HistoryTable, 0, 65536 * sizeof(int));
	}
	//清空杀手节点表
	inline void ClearKillerTable() {
		memset(KillerTable, 0, LIMIT_DEPTH * 2 * sizeof(unsigned short));
	}
};

#endif
