// No.177
// 001C2EC0,59
// －－－－－－－－－－－－－－－－
// These waters are treacherous!
// You shouldn't come back here!
// －－－－－－－－－－－－－－－－
// These waters are treacherous!
// You shouldn't come back here!
// －－－－－－－－－－－－－－－－
// =========================================
// 17个减号（16个汉字+一个控制符）

// 当译文比原文长的时候需要有标记，方便导入时改变偏移地址

#ifndef IMPORT_H
#define IMPORT_H

#include <stdio.h>

#include <common.h>
#include <ctype.h>


#ifdef _DEBUG
//#define ENCODING_TEST
//#define ENCODING_DEBUG
//#define TXT_READ_DEBUG
//#define DUMP_DEBUG
//#define DEBUG_TRACE
#endif



WORD pwCtrlCodeIndex[] = {0xFC, 0xFD};
#define CTRL_COUNT (sizeof (pwCtrlCodeIndex) / sizeof (pwCtrlCodeIndex[0]))
//文本区间链表
typedef struct txtExtent
{
	struct txtExtent * next;
	ULONG start, end;
	int iWidth;
} txtExt;

#ifdef DEBUG_TRACE
FILE *fpDEBUG;
#endif

FILE 	*fpError;
long 	g_lErrorNo = 0;
BYTE 	*g_pbyRom;
long 	g_lRomLength = 0;

void 	getCodingTable (FILE * fp, char strTable[CODING_LENGTH + 1][10], int *iarrLength);
void 	getCodingLength (char strTable[CODING_LENGTH + 1][10], int * iarrLength);

int 	changeRomTxt (FILE *fpRom, FILE * fpSrcTxt, char strTable[CODING_LENGTH + 1][10], int *iarrLength, txtExt *linkList);
int 	handleLine (char *strLine, char strTable[CODING_LENGTH + 1][10], int *iarrLength, BYTE *pbyteBuffer);
void 	skipBOM (FILE *fp);
char 	*fGetLine (char * strBuffer, FILE * fp);
int 	fGetCutLine (char * strBuffer, FILE * fp);
int 	renewTextAddr (FILE *fp, ULONG ulOldAddr, ULONG ulNewAddr);
txtExt 	*getAddrTable (FILE *fp);
int 	isInsertDirectly(ULONG ulAddr, txtExt * linkList);

#endif
