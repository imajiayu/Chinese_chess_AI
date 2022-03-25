#include"Search.h"


void MoveSortClass::Init(int mvHash_, PositionClass* pPosition, int* HistoryTable, unsigned short KillerTable[][2]) { // 初始化，设定置换表走法和两个杀手走法
	pPos = pPosition;
	historyTable = HistoryTable;
	mvHash = mvHash_;
	mvKiller1 = KillerTable[(*pPos).nDistance][0];
	mvKiller2 = KillerTable[(*pPos).nDistance][1];
	phase = PHASE_HASH;
}


// 得到下一个走法
int MoveSortClass::NextMove() {
	int mv;
	switch (phase) {			//根绝MoveSort的阶段进行相应动作
	case PHASE_HASH:			//置换表启发
		phase++;
		if (mvHash != 0)
			return mvHash;

	case PHASE_KILLER_1:		//第一个杀手节点
		phase++;
		if (mvKiller1 != mvHash && mvKiller1 != 0 && (*pPos).LegalMove(mvKiller1))
			return mvKiller1;

	case PHASE_KILLER_2:		//第二个杀手节点
		phase++;
		if (mvKiller2 != mvHash && mvKiller2 != 0 && (*pPos).LegalMove(mvKiller2))
			return mvKiller2;

	case PHASE_GEN_SORT:		//生成所有着法并按历史表排序
		phase++;
		genMoveNum = (*pPos).GenMoves(mvlist);
		HistorySort(mvlist, mvlist + genMoveNum);
		index = 0;
	case PHASE_GET:				//对于剩余的非置换表、非杀手节点逐个获取
		while (index < genMoveNum) {
			mv = mvlist[index];
			index++;
			if (mv != mvHash && mv != mvKiller1 && mv != mvKiller2)
				return mv;
		}
	default:					//着法取尽，返回0
		return 0;
	}
}

// 静态搜索过程
int SearchClass::SearchQuiesc(int alpha, int beta) {
	int bestValue, value, genMoveNum;
	int mvlist[MAX_GEN_MOVES];

	//重复裁剪；
	value = Pos.RepPruning();
	if (value > -MATE_VALUE)
		return value;


	//搜索达到极限深度，返回评价值
	if (Pos.nDistance == LIMIT_DEPTH)
		return Pos.Evaluate();

	//静态搜索初始化；
	bestValue = -MATE_VALUE;
	if (Pos.InCheck()) {			//处在被将军局面，生成所有着法
		genMoveNum = Pos.GenMoves(mvlist);
		std::sort(mvlist, mvlist + genMoveNum, [this](int mv1, int mv2) {return (HistoryTable[mv1] > HistoryTable[mv2]); });
	}
	else {							//未被将军，先对局面进行评价
		value = Pos.Evaluate();
		if (value >= beta)
			return value;
		bestValue = value;
		alpha = std::max(value, alpha);
		//再使用MVVLVA启发对着法排序
		genMoveNum = Pos.GenMoves(mvlist, GEN_CAPTURE);
		Pos.SortMoves(mvlist, mvlist + genMoveNum);
	}
	//用Alpha-Beta算法搜索
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
	//返回值
	if (bestValue == -MATE_VALUE)
		return Pos.nDistance - MATE_VALUE;
	else
		return bestValue;
}
//零窗口主要变例搜索
int SearchClass::SearchPVCut(int beta, int depth, bool useNull) {
	int newDepth, hashFlag, bestValue, value, currentTime;
	int mvBest, mvHash, mv;
	MoveSortClass MoveSort;

	//对叶子节点使用静态搜索
	if (depth <= 0)
		return SearchQuiesc(beta - 1, beta);

	//重复裁剪；
	value = Pos.RepPruning();
	if (value > -MATE_VALUE)
		return value;

	//置换裁剪；
	value = HashTable.Load(Pos, beta - 1, beta, depth, mvHash);
	if (useHash && value > -MATE_VALUE)
		return value;

	//搜索达到极限深度，返回评价值
	if (Pos.nDistance == LIMIT_DEPTH)
		return Pos.Evaluate();

	//尝试空着裁剪
	if (USE_NULL && useNull && !Pos.InCheck() && Pos.NullOkay()) {
		Pos.NullMove();
		value = -SearchPVCut(1 - beta, depth - NULL_DEPTH - 1, false);
		Pos.UndoNullMove();

		if (value >= beta) {
			if (Pos.NullSafe()) {				//空着裁剪安全，记录深度至少为NULL_DEPTH+1
				HashTable.Save(Pos, HASH_BETA, value, std::max(depth, NULL_DEPTH + 1), 0);
				return value;
			}
			else if (SearchPVCut(beta, depth - NULL_DEPTH, false) >= beta) {
				//空着裁剪安全，记录深度至少为NULL_DEPTH
				HashTable.Save(Pos, HASH_BETA, value, std::max(depth, NULL_DEPTH), 0);
				return value;
			}
		}
	}
	//初始化
	mvBest = 0;
	hashFlag = HASH_ALPHA;
	bestValue = -MATE_VALUE;

	//通过MoveSortClass对着法进行排序，并逐个进行搜索
	MoveSort.Init(mvHash, &Pos, HistoryTable, KillerTable);
	for (mv = MoveSort.NextMove(); mv != 0; mv = MoveSort.NextMove()) {
		if (Pos.MakeMove(mv)) {
			//选择性延伸；
			newDepth = (Pos.InCheck() ? depth : depth - 1);
			//主要变例零窗口搜索		
			value = -SearchPVCut(1 - beta, newDepth, true);
			Pos.UndoMakeMove();
			if (isStop) 
				return bestValue;
			
			//边界判定；
			if (value > bestValue) {
				bestValue = value;
				if (value >= beta) {
					mvBest = mv;
					hashFlag = HASH_BETA;
					break;
				}
			}

			//是否超时
			currentTime = int(GetTime() - startTime);
			if (currentTime > maxTime) 
				isStop = true;

		}
	}

	//更新置换表、历史表和杀手着法表。
	if (bestValue == -MATE_VALUE) 
		return Pos.nDistance - MATE_VALUE;
	else {
		HashTable.Save(Pos, hashFlag, bestValue, depth, mvBest);
		if (mvBest != 0) 
			MoveSort.SetBestMove(mvBest, depth, KillerTable[Pos.nDistance]);
		return bestValue;
	}


}

//主要变例搜索
int SearchClass::SearchPV(int alpha, int beta, int depth) {
	int newDepth, hashFlag, bestValue, value, currentTime;
	int mvBest, mvHash, mv;
	MoveSortClass MoveSort;

	//对叶子节点使用静态搜索
	if (depth <= 0) 
		return SearchQuiesc(alpha, beta);

	//重复裁剪
	value = Pos.RepPruning();
	if (value > -MATE_VALUE) 
		return value;
	
	//置换裁剪
	value = HashTable.Load(Pos, alpha, beta, depth, mvHash);
	if (useHash && value > -MATE_VALUE) 
		return value;
	
	//搜索达到极限深度，返回评价值
	if (Pos.nDistance == LIMIT_DEPTH) 
		return Pos.Evaluate();
	
	//初始化
	mvBest = 0;
	hashFlag = HASH_ALPHA;
	bestValue = -MATE_VALUE;

	//通过MoveSortClass对着法进行排序，并逐个进行搜索
	MoveSort.Init(mvHash, &Pos, HistoryTable, KillerTable);
	while ((mv = MoveSort.NextMove()) != 0) {
		if (Pos.MakeMove(mv)) {
			//选择性延伸
			newDepth = (Pos.InCheck() ? depth : depth - 1);
			//主要变例搜索			
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
			
			//边界判定
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

	//更新置换表、历史表和杀手着法表。
	if (bestValue == -MATE_VALUE) 
		return Pos.nDistance - MATE_VALUE;
	else {
		HashTable.Save(Pos, hashFlag, bestValue, depth, mvBest);
		if (mvBest != 0) 
			MoveSort.SetBestMove(mvBest, depth, KillerTable[Pos.nDistance]);
		return bestValue;
	}
}
// 根结点搜索过程
int SearchClass::SearchRoot(int depth) {
	int newDepth, bestValue, value, mv, currentTime;
	MoveSortClass MoveSort;

	//初始化
	bestValue = -MATE_VALUE;

	//通过MoveSortClass对着法进行排序，并逐个进行搜索
	MoveSort.Init(finalMove, &Pos, HistoryTable, KillerTable);
	while ((mv = MoveSort.NextMove()) != 0) {
		if (Pos.MakeMove(mv)) {
			//选择性延伸
			newDepth = (Pos.InCheck() ? depth : depth - 1);
			//主要变例搜索
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
			
			//边界判定
			if (value > bestValue) {
				//如果搜索到第一着法，那么"未改变最佳着法"的计数器加1，否则清零
				unchangedDepth = (bestValue == -MATE_VALUE ? unchangedDepth + 1 : 0);
				bestValue = value;
				//搜索到最佳着法时记录主要变例
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
		//读取开局库中的走法
		nBookMoves = BookData.GetBookMoves(Pos);
		if (nBookMoves > 0) {
			i = 0;
			//如果开局库中的着法构成循环局面，那么不走这个着法
			Pos.MakeMove(BookData.Books[i].mv);
			if (Pos.RepStatus(1) == 0) {
				finalMove = BookData.Books[i].mv;
				Pos.UndoMakeMove();
				return MOVE_COORD(finalMove);
			}
			Pos.UndoMakeMove();
		}
	}
	//初始化
	isStop = false;
	unchangedDepth = 0;
	finalMove = 0;
	ClearKillerTable();
	ClearHistory();
	HashTable.Clear();
	startTime = GetTime();
	lastValue = 0;
	currentTime = 0;

	//当前局面如果可以出杀棋，则出杀棋
	int winMove = Pos.GenWinMoves();
	if (winMove != 0) {
		return MOVE_COORD(winMove);
	}

	//迭代加深搜索
	for (i = 1; i <= UCCI_MAX_DEPTH; i++) {
		//搜索根结点
		value = SearchRoot(i);
		if (isDebug) {
			printf("info depth %d score %d time %d\n", i, value, int(GetTime() - startTime));
			fflush(stdout);
		}
		if (isStop) {
			if (value > -MATE_VALUE) 
				lastValue = value; // 跳出后，lastValue会用来判断认输或投降，所以需要给定最近一个值
			break; // 没有跳出，则"vl"是可靠值
		}
		currentTime = int(GetTime() - startTime);
		// 如果搜索时间超过适当时限，则终止搜索
		limitTime = maxTime;
		//如果当前搜索值没有落后前一层很多，那么适当时限减半（废弃）
		//limitTime = (value + DROPDOWN_VALUE >= lastValue ? limitTime / 2 : limitTime);
		//如果最佳着法连续多层没有变化，那么适当时限减半（废弃）
		//limitTime = (unchangedDepth >= UNCHANGED_DEPTH ? limitTime / 2 : limitTime);

		lastValue = value;
		if (currentTime > limitTime) 			
			break;
		

		//搜索到杀棋则终止搜索
		if (fabs(lastValue) > WIN_VALUE) {
			break;
		}
		
	}

	//输出最佳着法
	uint32_t result = MOVE_COORD(finalMove);
	return result;
}

