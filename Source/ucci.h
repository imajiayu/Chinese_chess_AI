#include "parameters.h"
#include "tools.h"
#ifndef UCCI_H
#define UCCI_H

const int UCCI_MAX_DEPTH = 32; // UCCI引擎思考的极限深度
const int MAX_INPUT = 8192;
const int MAX_MOVE_NUM = 1024;

enum UcciCommEnum // UCCI指令类型
{
	UCCI_COMM_UNKNOWN,
	UCCI_COMM_UCCI,
	UCCI_COMM_ISREADY,
	UCCI_COMM_POSITION,
	UCCI_COMM_GO_TIME,
	UCCI_COMM_QUIT
};

class UcciCommClass
{
public:
	const char* Fen;  // FEN串
	int nMoveNum;	  // 后续着法数
	uint32_t* lMoves; // 后续着法
	int nTime;		  // 时间go_time
	int nRound;		  // 当前回合数
	//UcciCommClass();

	/**
	 * @brief 		读取输入，并将结果存在自身变量中
	 *
	 * @return int 	读取到的指令类型，对应于UcciCommEnum
	 */
	int read();
	//~UcciCommClass();
private:
	bool ParsePos(char* temp);
};

// 引擎启动指令，只接收"ucci"
UcciCommEnum BootLine();                                    
//UcciCommEnum IdleLine(UcciCommClass& UcciComm, bool bDebug); // 引擎空闲时接收指令

#endif

