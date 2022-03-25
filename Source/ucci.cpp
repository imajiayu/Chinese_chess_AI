#define _CRT_SECURE_NO_WARNINGS
#include "ucci.h"  
#include <cstdio>
#include <iostream>
#include <string.h>
#pragma comment(lib, "Shlwapi.lib")
using namespace std;

//#define min(a,b)            (((a) < (b)) ? (a) : (b))

/*"BootLine()"接收引擎启动后的第一行指令
 * 输入"ucci"时就返回"UCCI_COMM_UCCI"，否则一律返回"UCCI_COMM_UNKNOWN"
 * 前两个解释器都等待是否有输入，如果没有输入则执行待机指令"Idle()"
 * 而第三个解释器("BusyLine()"，只用在引擎思考时)则在没有输入时直接返回"UCCI_COMM_UNKNOWN"
 */

static char szFen[MAX_INPUT];
static uint32_t dwCoordList[MAX_MOVE_NUM];
//char* strchr(const char* str, char c)
bool UcciCommClass::ParsePos(char* temp)
{
	int i;
	int szNum;
	//temp字符串格式为{fen <FEN串> | startpos} [moves <后续着法列表>]
	//输入一个字符串temp，判断是否指定了FEN串
	if (StrEqvSkip(temp, "fen "))
	{
		char* lpsz, * lptemp;
		lpsz = temp;
		lpsz = strcasestr(temp, " - - ");
		szNum = lpsz - temp;
		lpsz += 5;//strlen(" - - ") = 5
		//StrScanSkip(lpsz, " - - ");
		if (lpsz != NULL)
		{
			StrScanSkip(lpsz, " ");
			nRound = 0;
			while (*lpsz != 0 && *lpsz != ' ')
			{
				nRound *= 10;
				nRound += *(lpsz) - '0';
				lpsz += 1;
			}	
		}
		strncpy(szFen, temp, szNum);
		szFen[szNum] = '\0';
		Fen = szFen;
	}
	//判断是否是startpos，开始局面
	else if (StrEqv(temp, "startpos"))
	{
		Fen = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w";
	}
	else
	{
		return false;
	}

	// 判断是否有"moves"
	nMoveNum = 0;   //nMoveNum为后续着法数目
	if (StrScanSkip(temp, " moves "))
	{
		//略过"moves"
		*(temp - strlen(" moves ")) = '\0';
		nMoveNum = min((int)(strlen(temp) + 1) / 5, MAX_MOVE_NUM);		// "moves"后面的每个着法都是1个空格和4个字符
		for (i = 0; i < nMoveNum; i++) {
			dwCoordList[i] = *(uint32_t*)temp; // 4个字符可转换为一个"uint32_t"，存储和处理起来方便
			temp += 5;//后移四个字符和一个空格
		}
		lMoves = dwCoordList;
	}
	return true;
}

UcciCommEnum BootLine()
{
	//只判断是否为ucci，是则返回UCCI_COMM_UCCI
	char szLineStr[MAX_INPUT];
	while (!cin.getline(szLineStr, MAX_INPUT))
	{
		Idle();
	}
	if (StrEqv(szLineStr, "ucci"))
	{
		return UCCI_COMM_UCCI;
	}
	else
	{
		return UCCI_COMM_UNKNOWN;
	}
}


/*
* @brief 		读取输入，并将结果存在自身变量中
*
* @return int 	读取到的指令类型，对应于UcciCommEnum
*/
int UcciCommClass::read()
{

	char idLineStr[MAX_INPUT];
	char* temp;
	bool bGoTime;

	while (!cin.getline(idLineStr, MAX_INPUT))
	{
		Idle();
	}
	temp = idLineStr;
	if (false)//debug
	{
		printf("info idleline [%s]\n", temp);
		fflush(stdout);//清空输入缓冲区
	}
	if (false) {
		//空着
	}
	// 1. "isready"指令
	else if (StrEqv(temp, "isready"))
	{
		return UCCI_COMM_ISREADY;
	}
	// 2. "position {<special_position> | fen <fen_string>} [moves <move_list>]"指令
	//如果读到position指令，调用ParsePos指令，着法列表保存在UcciComm里
	else if (StrEqvSkip(temp, "position "))
	{
		//ParsePos是复杂的解释器.........
		return ParsePos(temp) ? UCCI_COMM_POSITION : UCCI_COMM_UNKNOWN;
	}
	else if (StrEqvSkip(temp, "go time "))
	{
		bGoTime = true;
		nTime = StrTime(temp, 0, 2000000000);
		return UCCI_COMM_GO_TIME;//然后开始思考，时间为nTime
	}
	else if (StrEqv(temp, "quit"))
	{
		return UCCI_COMM_QUIT;
	}
	else
	{
		return UCCI_COMM_UNKNOWN;
	}
}