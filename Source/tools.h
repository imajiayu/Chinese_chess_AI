#ifndef TOOLS_H
#define TOOLS_H
#include <cstdint>
#include <sys/timeb.h>

inline int64_t GetTime()
{
	timeb tb;
	ftime(&tb);
	return (int64_t)tb.time * 1000 + tb.millitm;
}

void Idle();

char* strcasestr(const char* sz1, const char* sz2);
int strncasecmp(const char* sz1, const char* sz2, size_t n);
void StrCutCrLf(char* sz);

bool StrEqv(const char* temp, const char* comp);
bool StrEqvSkip(char*& temp, const char* comp);
bool StrScanSkip(char*& temp, const char* comp);
int StrTime(const char* temp, int nMin, int nMax);

bool StrScan(const char* sz1, const char* sz2);
bool StrSplitSkip(const char*& szSrc, int nSeparator, char* szDst = NULL);
bool StrSplitSkip(char*& szSrc, int nSeparator, char* szDst = NULL);


class RC4Class
{
	uint8_t s[256];
	int x, y;

public:
	RC4Class();
	//void InitZero(void);
	uint8_t NextByte();  //返回1字节长随机数
	uint32_t NextLong(); //返回4字节长随机数
};

#endif