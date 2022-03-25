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
		// 3. ����ҵ����棬����ǰ���ҵ�һ���ŷ���
		for (nPtr--; nPtr >= 0; nPtr--) {
			bk = bookBuffer[nPtr];
			if (BOOK_POS_CMP(bk, pos_zo) < 0) {
				break;
			}
		}
		// 4. ������ζ������ڸþ����ÿ���ŷ���
		for (nPtr++; nPtr < buffer_len; nPtr++) {
			bk = bookBuffer[nPtr];
			if (BOOK_POS_CMP(bk, pos_zo) > 0) {
				break;
			}
			// ��������ǵڶ����������ģ����ŷ�����������
			bk.mv = MOVE(DST(bk.mv), SRC(bk.mv));		//��������MOVE��ʽת��
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
		// ԭ����;�����������һ��
		if(nScan == 0) pos_zo = pos.MirrorZobrist();
	}
	if (nMoves == 0)return 0;
	// 5. ���ŷ�����ֵ����
	for (i = 0; i < nMoves - 1; i++) {
		for (j = nMoves - 1; j > i; j--) {
			if (Books[j - 1].value < Books[j].value) {
				std::swap(Books[j - 1], Books[j]);
			}
		}
	}
	return nMoves;
}