#include "bookdata.h"
#include "bookdata_bigbook.h"
#include <algorithm>

int BookDataClass::BOOK_POS_CMP(const BookStruct& bk, const ZobristStruct& pos_zo)
{
	return bk.ZobristLock < pos_zo.dwKey ? -1 :
		bk.ZobristLock > pos_zo.dwKey ? 1 : 0;
}

int BookDataClass::GetBookMoves(const PositionClass& pos)
{
	BookStruct bk;
	ZobristStruct pos_zo = pos.zobr;
	int buffer_len = sizeof(bookBuffer) / sizeof(bookBuffer[0]);
	int nScan, nLow, nHigh, nPtr;
	int i, j, nMoves;
	nMoves = 0;
	for (nScan = 0; nScan < 2; nScan++) {
		nPtr = nLow = 0;
		nHigh = buffer_len - 1;
		while (nLow <= nHigh) {
			nPtr = (nLow + nHigh) / 2;
			bk = bookBuffer[nPtr];
			if (BOOK_POS_CMP(bk, pos_zo) < 0) {
				nLow = nPtr + 1;
			}
			else if (BOOK_POS_CMP(bk, pos_zo) > 0) {
				nHigh = nPtr - 1;
			}
			else {
				break;
			}
		}
		if (nLow > nHigh) {
			continue;
		}
		// 3. 如果找到局面，则向前查找第一个着法；
		for (nPtr--; nPtr >= 0; nPtr--) {
			bk = bookBuffer[nPtr];
			if (BOOK_POS_CMP(bk, pos_zo) < 0) {
				break;
			}
		}
		// 4. 向后依次读入属于该局面的每个着法；
		for (nPtr++; nPtr < buffer_len; nPtr++) {
			bk = bookBuffer[nPtr];
			if (BOOK_POS_CMP(bk, pos_zo) > 0) {
				break;
			}
			// 如果局面是第二趟搜索到的，则着法必须做镜像
			bk.mv = MOVE(DST(bk.mv), SRC(bk.mv));		//兵河五四MOVE格式转码
			bk.mv = nScan == 0 ? bk.mv : MOVE_MIRROR(bk.mv);
			if (pos.LegalMove(bk.mv)) {
				Books[nMoves].mv = bk.mv;
				Books[nMoves].value = bk.value;
				nMoves++;
				if (nMoves == MAX_GEN_MOVES) {
					break;
				}
			}
		}
		// 原局面和镜像局面各搜索一趟
		if(nScan == 0) pos_zo = pos.MirrorZobrist();
	}
	if (nMoves == 0)return 0;
	// 5. 对着法按分值排序
	for (i = 0; i < nMoves - 1; i++) {
		for (j = nMoves - 1; j > i; j--) {
			if (Books[j - 1].value < Books[j].value) {
				std::swap(Books[j - 1], Books[j]);
			}
		}
	}
	return nMoves;
}