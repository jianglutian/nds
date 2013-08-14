#include "../common.h"

#define NARC_HEAD
#ifdef NARC_HEAD
#define HEAD_LENGTH			 0x14	//文件头名称的长度
#endif


#define PAK_START_OFFSET	(0x3C8C)	//第一个包开始的偏移位置

typedef struct tagPakInfo
{
	ULONG	ulLength,
			ulOffsetBegin,
			ulOffsetEnd;
	int		iFlag;
}PakInfo;

void fileHeadHandle (FILE *outFile, ULONG ulPakCount, PakInfo [MAX_FILE_COUNT]);
void fileHeadSecHandle (FILE *fp, char strPakName[MAX_FILE_COUNT][MAX_FILE_NAME_LENGTH], char strOutName[MAX_FILE_NAME_LENGTH]);
void fileHeadTrdHandle (FILE *fp, ULONG ulLength);
ULONG getFileLength (FILE *fp);
void writeFileLength (FILE *fp, PakInfo tPakInfo[MAX_FILE_COUNT], ULONG ulPakCount);
static ULONG writeFileName (FILE *fp, char strTable[MAX_FILE_COUNT] [ MAX_FILE_NAME_LENGTH ]);
