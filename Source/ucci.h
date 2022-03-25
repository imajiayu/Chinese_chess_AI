#include "parameters.h"
#include "tools.h"
#ifndef UCCI_H
#define UCCI_H

const int UCCI_MAX_DEPTH = 32; // UCCI����˼���ļ������
const int MAX_INPUT = 8192;
const int MAX_MOVE_NUM = 1024;

enum UcciCommEnum // UCCIָ������
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
	const char* Fen;  // FEN��
	int nMoveNum;	  // �����ŷ���
	uint32_t* lMoves; // �����ŷ�
	int nTime;		  // ʱ��go_time
	int nRound;		  // ��ǰ�غ���
	//UcciCommClass();

	/**
	 * @brief 		��ȡ���룬��������������������
	 *
	 * @return int 	��ȡ����ָ�����ͣ���Ӧ��UcciCommEnum
	 */
	int read();
	//~UcciCommClass();
private:
	bool ParsePos(char* temp);
};

// ��������ָ�ֻ����"ucci"
UcciCommEnum BootLine();                                    
//UcciCommEnum IdleLine(UcciCommClass& UcciComm, bool bDebug); // �������ʱ����ָ��

#endif

