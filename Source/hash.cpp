#include "hash.h"

void HashClass::Clear()
{
	memset(Table, 0, sizeof(Table));
}

int HashClass::Load(const PositionClass& pos, int vlAlpha, int vlBeta, int nDepth, int& mv, bool bUseNull)
{
	bool bMate; // 杀棋标志：如果是杀棋，那么不需要满足深度条件
	HashStruct hsh;

	hsh = Table[pos.zobr.dwKey & (HASH_SIZE - 1)];
	if (hsh.dwKey != pos.zobr.dwKey) {
		mv = 0;
		return -MATE_VALUE;
	}
	mv = hsh.mv;
	if (!bUseNull && mv == 0)return -MATE_VALUE;
	bMate = false;
	if (hsh.value > WIN_VALUE) {
		hsh.value -= pos.nDistance;
		bMate = true;
	}
	else if (hsh.value < -WIN_VALUE) {
		hsh.value += pos.nDistance;
		bMate = true;
	}
	if (hsh.nDepth >= nDepth || bMate) {
		if (hsh.nFlag == HASH_BETA) {
			return (hsh.value >= vlBeta ? hsh.value : -MATE_VALUE);
		}
		else if (hsh.nFlag == HASH_ALPHA) {
			return (hsh.value <= vlAlpha ? hsh.value : -MATE_VALUE);
		}
		return hsh.value;
	}
	return -MATE_VALUE;
}

bool HashClass::Save(const PositionClass& pos, int nFlag, int vl, int nDepth, int mv)
{
	HashStruct hsh;
	hsh = Table[pos.zobr.dwKey & (HASH_SIZE - 1)];
	if (hsh.nDepth > nDepth) {
		return false;
	}
	hsh.nFlag = nFlag;
	hsh.nDepth = nDepth;
	if (vl > WIN_VALUE) {
		hsh.value = vl + pos.nDistance;
	}
	else if (vl < -WIN_VALUE) {
		hsh.value = vl - pos.nDistance;
	}
	else {
		hsh.value = vl;
	}
	hsh.mv = mv;
	hsh.dwKey = pos.zobr.dwKey;
	Table[pos.zobr.dwKey & (HASH_SIZE - 1)] = hsh;
	return true;
}