#define _CRT_SECURE_NO_WARNINGS
#include "ucci.h"  
#include <cstdio>
#include <iostream>
#include <string.h>
#pragma comment(lib, "Shlwapi.lib")
using namespace std;

//#define min(a,b)            (((a) < (b)) ? (a) : (b))

/*"BootLine()"��������������ĵ�һ��ָ��
 * ����"ucci"ʱ�ͷ���"UCCI_COMM_UCCI"������һ�ɷ���"UCCI_COMM_UNKNOWN"
 * ǰ�������������ȴ��Ƿ������룬���û��������ִ�д���ָ��"Idle()"
 * ��������������("BusyLine()"��ֻ��������˼��ʱ)����û������ʱֱ�ӷ���"UCCI_COMM_UNKNOWN"
 */

static char szFen[MAX_INPUT];
static uint32_t dwCoordList[MAX_MOVE_NUM];
//char* strchr(const char* str, char c)
bool UcciCommClass::ParsePos(char* temp)
{
	int i;
	int szNum;
	//temp�ַ�����ʽΪ{fen <FEN��> | startpos} [moves <�����ŷ��б�>]
	//����һ���ַ���temp���ж��Ƿ�ָ����FEN��
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
	//�ж��Ƿ���startpos����ʼ����
	else if (StrEqv(temp, "startpos"))
	{
		Fen = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w";
	}
	else
	{
		return false;
	}

	// �ж��Ƿ���"moves"
	nMoveNum = 0;   //nMoveNumΪ�����ŷ���Ŀ
	if (StrScanSkip(temp, " moves "))
	{
		//�Թ�"moves"
		*(temp - strlen(" moves ")) = '\0';
		nMoveNum = min((int)(strlen(temp) + 1) / 5, MAX_MOVE_NUM);		// "moves"�����ÿ���ŷ�����1���ո��4���ַ�
		for (i = 0; i < nMoveNum; i++) {
			dwCoordList[i] = *(uint32_t*)temp; // 4���ַ���ת��Ϊһ��"uint32_t"���洢�ʹ�����������
			temp += 5;//�����ĸ��ַ���һ���ո�
		}
		lMoves = dwCoordList;
	}
	return true;
}

UcciCommEnum BootLine()
{
	//ֻ�ж��Ƿ�Ϊucci�����򷵻�UCCI_COMM_UCCI
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
* @brief 		��ȡ���룬��������������������
*
* @return int 	��ȡ����ָ�����ͣ���Ӧ��UcciCommEnum
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
		fflush(stdout);//������뻺����
	}
	if (false) {
		//����
	}
	// 1. "isready"ָ��
	else if (StrEqv(temp, "isready"))
	{
		return UCCI_COMM_ISREADY;
	}
	// 2. "position {<special_position> | fen <fen_string>} [moves <move_list>]"ָ��
	//�������positionָ�����ParsePosָ��ŷ��б�����UcciComm��
	else if (StrEqvSkip(temp, "position "))
	{
		//ParsePos�Ǹ��ӵĽ�����.........
		return ParsePos(temp) ? UCCI_COMM_POSITION : UCCI_COMM_UNKNOWN;
	}
	else if (StrEqvSkip(temp, "go time "))
	{
		bGoTime = true;
		nTime = StrTime(temp, 0, 2000000000);
		return UCCI_COMM_GO_TIME;//Ȼ��ʼ˼����ʱ��ΪnTime
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