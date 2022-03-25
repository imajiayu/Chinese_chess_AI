#define _CRT_SECURE_NO_WARNINGS
#include"tools.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <windows.h>
#include <shlwapi.h>

void Idle(void) {
	Sleep(1);
}

char* strcasestr(const char* sz1, const char* sz2) {
	return StrStrIA(sz1, sz2);
}


int strncasecmp(const char* sz1, const char* sz2, size_t n) {
	return _strnicmp(sz1, sz2, n);
}

bool StrEqv(const char* temp, const char* comp)
{
	return strncasecmp(temp, comp, strlen(comp)) == 0;
}

//比较完sz1向后移动
bool StrEqvSkip(char*& temp, const char* comp)
{
	if (strncasecmp(temp, comp, strlen(comp)) == 0)
	{
		temp += strlen(comp);
		return true;
	}
	else
	{
		return false;
	}
}

bool StrScan(const char* sz1, const char* sz2)
{
	return strcasestr(sz1, sz2) != NULL;
}

bool StrScanSkip(char*& temp, const char* comp) {
	char* lpsz;
	lpsz = StrStrIA(temp, comp);
	if (lpsz == NULL)
	{
		return false;
	}
	else
	{
		temp = lpsz + strlen(comp);
		return true;
	}
}

bool StrSplitSkip(const char*& szSrc, int nSeparator, char* szDst) {
	const char* lpsz;
	lpsz = strchr(szSrc, nSeparator);
	if (lpsz == NULL) {
		if (szDst != NULL) {
			strcpy(szDst, szSrc);
		}
		szSrc += strlen(szSrc);
		return false;
	}
	else {
		if (szDst != NULL) {
			strncpy(szDst, szSrc, lpsz - szSrc);
			szDst[lpsz - szSrc] = '\0';
		}
		szSrc = lpsz + 1;
		return true;
	}
}

bool StrSplitSkip(char*& szSrc, int nSeparator, char* szDst) {
	char* lpsz;
	lpsz = strchr(szSrc, nSeparator);
	if (lpsz == NULL) {
		if (szDst != NULL) {
			strcpy(szDst, szSrc);
		}
		szSrc += strlen(szSrc);
		return false;
	}
	else {
		if (szDst != NULL) {
			strncpy(szDst, szSrc, lpsz - szSrc);
			szDst[lpsz - szSrc] = '\0';
		}
		szSrc = lpsz + 1;
		return true;
	}
}

int StrTime(const char* temp, int nMin, int nMax)
{
	int nRet;
	if (sscanf(temp, "%d", &nRet) > 0)
	{
		return min(max(nRet, nMin), nMax);
	}
	else
	{
		return nMin;
	}
}

RC4Class::RC4Class()
{
	int i, j;
	uint8_t uc;

	x = y = j = 0;
	for (i = 0; i < 256; i++) {
		s[i] = i;
	}
	for (i = 0; i < 256; i++) {
		j = (j + s[i]) & 255;
		uc = s[i];
		s[i] = s[j];
		s[j] = uc;
	}
}

uint8_t RC4Class::NextByte()
{
	x = (x + 1) & 255;
	y = (y + s[x]) & 255;
	std::swap(s[x], s[y]);
	return s[(s[x] + s[y]) & 255];
}

uint32_t RC4Class::NextLong()
{
	union
	{
		uint8_t uc[4];
		uint32_t dw;
	}Ret;

	Ret.uc[0] = NextByte();
	Ret.uc[1] = NextByte();
	Ret.uc[2] = NextByte();
	Ret.uc[3] = NextByte();
	return Ret.dw;
}
