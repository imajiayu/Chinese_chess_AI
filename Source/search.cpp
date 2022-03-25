#include"Search.h"


void MoveSortClass::Init(int mvHash_, PositionClass* pPosition, int* HistoryTable, unsigned short KillerTable[][2]) { // ��ʼ�����趨�û����߷�������ɱ���߷�
	pPos = pPosition;
	historyTable = HistoryTable;
	mvHash = mvHash_;
	mvKiller1 = KillerTable[(*pPos).nDistance][0];
	mvKiller2 = KillerTable[(*pPos).nDistance][1];
	phase = PHASE_HASH;
}


// �õ���һ���߷�
int MoveSortClass::NextMove() {
	int mv;
	switch (phase) {			//����MoveSort�Ľ׶ν�����Ӧ����
	case PHASE_HASH:			//�û�������
		phase++;
		if (mvHash != 0)
			return mvHash;

	case PHASE_KILLER_1:		//��һ��ɱ�ֽڵ�
		phase++;
		if (mvKiller1 != mvHash && mvKiller1 != 0 && (*pPos).LegalMove(mvKiller1))
			return mvKiller1;

	case PHASE_KILLER_2:		//�ڶ���ɱ�ֽڵ�
		phase++;
		if (mvKiller2 != mvHash && mvKiller2 != 0 && (*pPos).LegalMove(mvKiller2))
			return mvKiller2;

	case PHASE_GEN_SORT:		//���������ŷ�������ʷ������
		phase++;
		genMoveNum = (*pPos).GenMoves(mvlist);
		HistorySort(mvlist, mvlist + genMoveNum);
		index = 0;
	case PHASE_GET:				//����ʣ��ķ��û�����ɱ�ֽڵ������ȡ
		while (index < genMoveNum) {
			mv = mvlist[index];
			index++;
			if (mv != mvHash && mv != mvKiller1 && mv != mvKiller2)
				return mv;
		}
	default:					//�ŷ�ȡ��������0
		return 0;
	}
}

// ��̬��������
int SearchClass::SearchQuiesc(int alpha, int beta) {
	int bestValue, value, genMoveNum;
	int mvlist[MAX_GEN_MOVES];

	//�ظ��ü���
	value = Pos.RepPruning();
	if (value > -MATE_VALUE)
		return value;


	//�����ﵽ������ȣ���������ֵ
	if (Pos.nDistance == LIMIT_DEPTH)
		return Pos.Evaluate();

	//��̬������ʼ����
	bestValue = -MATE_VALUE;
	if (Pos.InCheck()) {			//���ڱ��������棬���������ŷ�
		genMoveNum = Pos.GenMoves(mvlist);
		std::sort(mvlist, mvlist + genMoveNum, [this](int mv1, int mv2) {return (HistoryTable[mv1] > HistoryTable[mv2]); });
	}
	else {							//δ���������ȶԾ����������
		value = Pos.Evaluate();
		if (value >= beta)
			return value;
		bestValue = value;
		alpha = std::max(value, alpha);
		//��ʹ��MVVLVA�������ŷ�����
		genMoveNum = Pos.GenMoves(mvlist, GEN_CAPTURE);
		Pos.SortMoves(mvlist, mvlist + genMoveNum);
	}
	//��Alpha-Beta�㷨����
	for (int i = 0; i < genMoveNum; i++) {
		if (Pos.MakeMove(mvlist[i])) {
			value = -SearchQuiesc(-beta, -alpha);
			Pos.UndoMakeMove();
			if (value > bestValue) {
				if (value >= beta)
					return value;
				else {
					bestValue = value;
					alpha = std::max(value, alpha);
				}
			}
		}
	}
	//����ֵ
	if (bestValue == -MATE_VALUE)
		return Pos.nDistance - MATE_VALUE;
	else
		return bestValue;
}
//�㴰����Ҫ��������
int SearchClass::SearchPVCut(int beta, int depth, bool useNull) {
	int newDepth, hashFlag, bestValue, value, currentTime;
	int mvBest, mvHash, mv;
	MoveSortClass MoveSort;

	//��Ҷ�ӽڵ�ʹ�þ�̬����
	if (depth <= 0)
		return SearchQuiesc(beta - 1, beta);

	//�ظ��ü���
	value = Pos.RepPruning();
	if (value > -MATE_VALUE)
		return value;

	//�û��ü���
	value = HashTable.Load(Pos, beta - 1, beta, depth, mvHash);
	if (useHash && value > -MATE_VALUE)
		return value;

	//�����ﵽ������ȣ���������ֵ
	if (Pos.nDistance == LIMIT_DEPTH)
		return Pos.Evaluate();

	//���Կ��Ųü�
	if (USE_NULL && useNull && !Pos.InCheck() && Pos.NullOkay()) {
		Pos.NullMove();
		value = -SearchPVCut(1 - beta, depth - NULL_DEPTH - 1, false);
		Pos.UndoNullMove();

		if (value >= beta) {
			if (Pos.NullSafe()) {				//���Ųü���ȫ����¼�������ΪNULL_DEPTH+1
				HashTable.Save(Pos, HASH_BETA, value, std::max(depth, NULL_DEPTH + 1), 0);
				return value;
			}
			else if (SearchPVCut(beta, depth - NULL_DEPTH, false) >= beta) {
				//���Ųü���ȫ����¼�������ΪNULL_DEPTH
				HashTable.Save(Pos, HASH_BETA, value, std::max(depth, NULL_DEPTH), 0);
				return value;
			}
		}
	}
	//��ʼ��
	mvBest = 0;
	hashFlag = HASH_ALPHA;
	bestValue = -MATE_VALUE;

	//ͨ��MoveSortClass���ŷ��������򣬲������������
	MoveSort.Init(mvHash, &Pos, HistoryTable, KillerTable);
	for (mv = MoveSort.NextMove(); mv != 0; mv = MoveSort.NextMove()) {
		if (Pos.MakeMove(mv)) {
			//ѡ�������죻
			newDepth = (Pos.InCheck() ? depth : depth - 1);
			//��Ҫ�����㴰������		
			value = -SearchPVCut(1 - beta, newDepth, true);
			Pos.UndoMakeMove();
			if (isStop) 
				return bestValue;
			
			//�߽��ж���
			if (value > bestValue) {
				bestValue = value;
				if (value >= beta) {
					mvBest = mv;
					hashFlag = HASH_BETA;
					break;
				}
			}

			//�Ƿ�ʱ
			currentTime = int(GetTime() - startTime);
			if (currentTime > maxTime) 
				isStop = true;

		}
	}

	//�����û�����ʷ���ɱ���ŷ���
	if (bestValue == -MATE_VALUE) 
		return Pos.nDistance - MATE_VALUE;
	else {
		HashTable.Save(Pos, hashFlag, bestValue, depth, mvBest);
		if (mvBest != 0) 
			MoveSort.SetBestMove(mvBest, depth, KillerTable[Pos.nDistance]);
		return bestValue;
	}


}

//��Ҫ��������
int SearchClass::SearchPV(int alpha, int beta, int depth) {
	int newDepth, hashFlag, bestValue, value, currentTime;
	int mvBest, mvHash, mv;
	MoveSortClass MoveSort;

	//��Ҷ�ӽڵ�ʹ�þ�̬����
	if (depth <= 0) 
		return SearchQuiesc(alpha, beta);

	//�ظ��ü�
	value = Pos.RepPruning();
	if (value > -MATE_VALUE) 
		return value;
	
	//�û��ü�
	value = HashTable.Load(Pos, alpha, beta, depth, mvHash);
	if (useHash && value > -MATE_VALUE) 
		return value;
	
	//�����ﵽ������ȣ���������ֵ
	if (Pos.nDistance == LIMIT_DEPTH) 
		return Pos.Evaluate();
	
	//��ʼ��
	mvBest = 0;
	hashFlag = HASH_ALPHA;
	bestValue = -MATE_VALUE;

	//ͨ��MoveSortClass���ŷ��������򣬲������������
	MoveSort.Init(mvHash, &Pos, HistoryTable, KillerTable);
	while ((mv = MoveSort.NextMove()) != 0) {
		if (Pos.MakeMove(mv)) {
			//ѡ��������
			newDepth = (Pos.InCheck() ? depth : depth - 1);
			//��Ҫ��������			
			if (bestValue == -MATE_VALUE) 
				value = -SearchPV(-beta, -alpha, newDepth);
			else {
				value = -SearchPVCut(-alpha, newDepth, true);
				if (value > alpha && value < beta) 
					value = -SearchPV(-beta, -alpha, newDepth);
			}
			Pos.UndoMakeMove();
			if (isStop) 
				return bestValue;
			
			//�߽��ж�
			if (value > bestValue) {
				bestValue = value;
				if (value >= beta) {
					mvBest = mv;
					hashFlag = HASH_BETA;
					break;
				}
				if (value > alpha) {
					alpha = value;
					mvBest = mv;
					hashFlag = HASH_PV;
				}
			}

			currentTime = int(GetTime() - startTime);
			if (currentTime > maxTime) 
				isStop = true;
			
		}
	}

	//�����û�����ʷ���ɱ���ŷ���
	if (bestValue == -MATE_VALUE) 
		return Pos.nDistance - MATE_VALUE;
	else {
		HashTable.Save(Pos, hashFlag, bestValue, depth, mvBest);
		if (mvBest != 0) 
			MoveSort.SetBestMove(mvBest, depth, KillerTable[Pos.nDistance]);
		return bestValue;
	}
}
// �������������
int SearchClass::SearchRoot(int depth) {
	int newDepth, bestValue, value, mv, currentTime;
	MoveSortClass MoveSort;

	//��ʼ��
	bestValue = -MATE_VALUE;

	//ͨ��MoveSortClass���ŷ��������򣬲������������
	MoveSort.Init(finalMove, &Pos, HistoryTable, KillerTable);
	while ((mv = MoveSort.NextMove()) != 0) {
		if (Pos.MakeMove(mv)) {
			//ѡ��������
			newDepth = (Pos.InCheck() ? depth : depth - 1);
			//��Ҫ��������
			if (bestValue == -MATE_VALUE) 
				value = -SearchPV(-MATE_VALUE, MATE_VALUE, newDepth);
			else {
				value = -SearchPVCut(-bestValue, newDepth, true);
				if (value > bestValue)
					value = -SearchPV(-MATE_VALUE, -bestValue, newDepth);
			}
			Pos.UndoMakeMove();
			if (isStop) 
				return bestValue;
			
			//�߽��ж�
			if (value > bestValue) {
				//�����������һ�ŷ�����ô"δ�ı�����ŷ�"�ļ�������1����������
				unchangedDepth = (bestValue == -MATE_VALUE ? unchangedDepth + 1 : 0);
				bestValue = value;
				//����������ŷ�ʱ��¼��Ҫ����
				finalMove = mv;
			}

			currentTime = int(GetTime() - startTime);
			if (currentTime > maxTime) 
				isStop = true;

		}
	}
	HashTable.Save(Pos, HASH_PV, bestValue, depth, finalMove);
	MoveSort.SetBestMove(finalMove, depth, KillerTable[Pos.nDistance]);
	return bestValue;
}
int SearchClass::SearchMain(PositionClass& Pos_, UcciCommClass& Ucci) {
	int i, value, lastValue;
	int currentTime, limitTime;
	int nBookMoves;
	RC4Class RC4;
	Pos = Pos_;
	maxTime = int(0.95 * Ucci.nTime);
	if (useBook) {
		//��ȡ���ֿ��е��߷�
		nBookMoves = BookData.GetBookMoves(Pos);
		if (nBookMoves > 0) {
			i = 0;
			//������ֿ��е��ŷ�����ѭ�����棬��ô��������ŷ�
			Pos.MakeMove(BookData.Books[i].mv);
			if (Pos.RepStatus(1) == 0) {
				finalMove = BookData.Books[i].mv;
				Pos.UndoMakeMove();
				return MOVE_COORD(finalMove);
			}
			Pos.UndoMakeMove();
		}
	}
	//��ʼ��
	isStop = false;
	unchangedDepth = 0;
	finalMove = 0;
	ClearKillerTable();
	ClearHistory();
	HashTable.Clear();
	startTime = GetTime();
	lastValue = 0;
	currentTime = 0;

	//��ǰ����������Գ�ɱ�壬���ɱ��
	int winMove = Pos.GenWinMoves();
	if (winMove != 0) {
		return MOVE_COORD(winMove);
	}

	//������������
	for (i = 1; i <= UCCI_MAX_DEPTH; i++) {
		//���������
		value = SearchRoot(i);
		if (isDebug) {
			printf("info depth %d score %d time %d\n", i, value, int(GetTime() - startTime));
			fflush(stdout);
		}
		if (isStop) {
			if (value > -MATE_VALUE) 
				lastValue = value; // ������lastValue�������ж������Ͷ����������Ҫ�������һ��ֵ
			break; // û����������"vl"�ǿɿ�ֵ
		}
		currentTime = int(GetTime() - startTime);
		// �������ʱ�䳬���ʵ�ʱ�ޣ�����ֹ����
		limitTime = maxTime;
		//�����ǰ����ֵû�����ǰһ��ܶ࣬��ô�ʵ�ʱ�޼��루������
		//limitTime = (value + DROPDOWN_VALUE >= lastValue ? limitTime / 2 : limitTime);
		//�������ŷ��������û�б仯����ô�ʵ�ʱ�޼��루������
		//limitTime = (unchangedDepth >= UNCHANGED_DEPTH ? limitTime / 2 : limitTime);

		lastValue = value;
		if (currentTime > limitTime) 			
			break;
		

		//������ɱ������ֹ����
		if (fabs(lastValue) > WIN_VALUE) {
			break;
		}
		
	}

	//�������ŷ�
	uint32_t result = MOVE_COORD(finalMove);
	return result;
}

