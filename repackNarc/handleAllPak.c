#include "handleAllPak.h"

#define NARC_PAK		//NARC的文件头

#ifdef NARC_PAK
BYTE headData [HEAD_LENGTH] =
{
	0x4e, 0x41, 0x52, 0x43, 0xfe, 0xff, 0x00, 0x01,
	0xec, 0xed, 0x39, 0x00, 0x10, 0x00, 0x03, 0x00,
	0x42, 0x54, 0x41, 0x46
};

BYTE headDataSec [] =
{
	0x42, 0x54, 0x4e, 0x46, 0x58, 0x12, 0x00, 0x00,
	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
	0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0,
	0x82
};

BYTE headDataTrd[] =
{
	0x47, 0x4d, 0x49, 0x46
};
#endif



//写入第一个文件头，NARC文件头
void fileHeadHandle (FILE *fp, ULONG ulPakCount, PakInfo tPakInfo[MAX_FILE_COUNT])
{
	ULONG	ulCount,
			ulOffset;
	fwrite (headData, sizeof (BYTE), sizeof(headData) / sizeof(BYTE), fp);
	
	//写入第一个文件头结束地址
	ulOffset = ulPakCount * 0x8 + 0xC;
	fwrite (&ulOffset, sizeof (ULONG), 1, fp);
	
	//写入包的数目
	fwrite (&ulPakCount, sizeof(ULONG), 1, fp);

	for (ulCount = 0; ulCount < ulPakCount; ulCount ++)
	{
		fwrite (&(tPakInfo[ulCount].ulOffsetBegin), sizeof (ULONG), 1, fp);
		fwrite (&(tPakInfo[ulCount].ulOffsetEnd),	 sizeof (ULONG), 1, fp);
	}
		
}

//在第一个NARC文件头里面逐个写入各个包的起始地址和结束地址
//（都是相对地址，计算绝对的话要加上NARC的header的0xC偏移量）
void writeFileLength (FILE *fp, PakInfo tPakInfo[MAX_FILE_COUNT], ULONG ulPakCount)
{
	ULONG ulCount;
	for (ulCount = 0; ulCount < ulPakCount; ulCount ++)
	{
		fwrite (&(tPakInfo[ulCount].ulOffsetBegin), sizeof (ULONG), 1, fp);
		fwrite (&(tPakInfo[ulCount].ulOffsetEnd), sizeof (ULONG), 1, fp);
	}
}

//写入BTAF文件头
void fileHeadSecHandle (FILE *fp, char strPakName[MAX_FILE_COUNT][MAX_FILE_NAME_LENGTH], char strOutName[MAX_FILE_NAME_LENGTH])
{
	int iNameLength = strlen(strOutName) - 5;
	char strBuf[MAX_STRING_LENGTH] ={0};
	ULONG ulOffset = 0;

	strncpy(strBuf, strOutName, iNameLength);
	ulOffset = ftell(fp);
	fwrite ( headDataSec, sizeof (BYTE), sizeof (headDataSec) / sizeof(BYTE), fp);
	fseek(fp, -9, SEEK_CUR);
	fputc (iNameLength + 0x14, fp);
	fseek(fp, 7, SEEK_CUR);
	fputc (iNameLength + 0x80, fp);
	fputs (strBuf, fp);
	fputc (0x01,fp);
	fputc (0xf0,fp);
	fputc (0x00,fp);
	ulOffset = writeFileName (fp, strPakName) - ulOffset;
	fseek (fp, -ulOffset + 4, SEEK_CUR);
	fwrite (&ulOffset, sizeof (ULONG), 1, fp);
	fseek (fp, 0, SEEK_END);
}

//最后一个文件头，后面就是各个具体的包的数据
void fileHeadTrdHandle (FILE *fp, ULONG ulLength)
{
	fwrite (headDataTrd, sizeof (BYTE), sizeof (headDataTrd) / sizeof(BYTE), fp);
	fwrite (&ulLength, sizeof (ULONG), 1, fp);
}

//在BTAF文件头中逐个写入各个包的名称（格式为：文件名长度+文件名）
ULONG writeFileName (FILE *fp, char strTable[MAX_FILE_COUNT] [ MAX_FILE_NAME_LENGTH ])
{
	ULONG ulOffset = 0;
	ULONG ulCount = 0;
	while ( 0 != (strcmp(strTable[ulCount], "")))
	{
		ulOffset += strlen(strTable[ulCount]) + 1;
		fputc (strlen(strTable[ulCount]), fp);
		fwrite (strTable[ulCount], sizeof (BYTE), strlen(strTable[ulCount]), fp);
		++ ulCount ;
	}
	fputc (ZERO_BYTE, fp);
	switch ((ftell(fp)) % 4)
	{
	case 1:
		fputc (FILL_BYTE, fp);
	case 2:
		fputc (FILL_BYTE, fp);
	case 3:
		fputc (FILL_BYTE, fp);
	case 0:
		break;
	default:
		;
	}
	return ftell(fp);
}
