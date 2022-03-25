#ifndef PARAMETERS_H
#define PARAMETERS_H

//POSITION
//
const int MAX_GEN_MOVES = 128;          // 最大的生成走法数
const int MAX_MOVEHIS = 256;            // 最大的历史走法数
const int MATE_VALUE = 10000;           // 最高分值，即将死的分值
const int WIN_VALUE = MATE_VALUE - 100; // 搜索出胜负的分值界限，超出此值就说明已经搜索出杀棋了
const int DRAW_VALUE = 20;              // 和棋时返回的分数(取负值)

const int ADVANCED_VALUE = 3; // 先行权分值
const int NULLOKEY_MARGIN = 200;  // 空步裁剪的子力边界
const int NULLSAFE_MARGIN = 400;  // 空步裁剪安全的子力边界

//HASH
//
const int HASH_ALPHA = 1;      // ALPHA节点的置换表项
const int HASH_BETA = 2;       // BETA节点的置换表项
const int HASH_PV = 3;         // PV节点的置换表项
const int HASH_SIZE = 1 << 24; // 置换表大小

const int LIMIT_DEPTH = 64;       // 搜索的极限深度
const int SORT_VALUE_MAX = 65535; // 着法序列最大值

// "GenMoves"参数
const bool GEN_CAPTURE = true;
const int NULL_DEPTH = 2;    // 空着裁剪的深度
const bool USE_NULL = true; // "SearchPV()"的参数

//SEARCH
const int UNCHANGED_DEPTH = 4;   // 未改变最佳着法的深度
const int DROPDOWN_VALUE = 20;   // 落后的分值

#endif