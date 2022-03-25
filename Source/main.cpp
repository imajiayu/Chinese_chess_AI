#include "ucci.h"
#include "position.h"
#include "search.h"

inline void PrintLn(const char* sz)
{
    printf("%s\n", sz);
    fflush(stdout);
}

int main()
{
    PositionClass Pos;
    SearchClass Search;
    UcciCommClass Ucci;
    if (BootLine() != UCCI_COMM_UCCI) 
    {
        return 0;
    }
    bool INLOOP = true;
    int move;
    PrintLn("ucciok");
    while (INLOOP)
    {
        int status = Ucci.read();
        switch (status)
        {
            case UCCI_COMM_UCCI:
                PrintLn("ucciok");
                break;
            case UCCI_COMM_ISREADY:
                PrintLn("readyok");
                break;
            case UCCI_COMM_POSITION:
                Pos.FromUcciComm(Ucci);
                break;
            case UCCI_COMM_GO_TIME:
                move = Search.SearchMain(Pos, Ucci);
                if (move == 0)
                    PrintLn("nobestmove");
                else
                {
                    printf("bestmove ");
                    printf("%.4s", (const char*)&move);
                    PrintLn("");
                }
                break;
            case UCCI_COMM_QUIT:
                INLOOP = false;
                PrintLn("bye");
                break;
            default:
                break;
        }
    }
    return 0;
}