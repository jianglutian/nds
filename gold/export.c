/*
 * 口袋金银导出HGSS日版文本
 * 前2个字节为句子数目，后2字节无视
 * 然后每8个字节一组，前4个字节为句子偏移地址，后4个字节为句子长度
 */
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <errno.h>

#include "../common.h"

#define CODING_LENGTH	(0xffff)

void getCodingTable (FILE * fp, char strTable[CODING_LENGTH + 1][10]);
int exportTxt (char *strSrc, char *strDst, char strTable[CODING_LENGTH][10]);

int main (int argc, char *argv[])
{
	char strPakDir[MAX_STRING_LENGTH] = "HGSS";
	char strCodingList[MAX_STRING_LENGTH] = "pkm.tbl";
	char strSrc[MAX_STRING_LENGTH],
	     strDst[MAX_STRING_LENGTH];
	char strTable[CODING_LENGTH + 1][10] = {0};	//游戏的对照码表
	struct _finddata_t c_file;
	intptr_t hFile;
	FILE *fpCodingList;
	if (NULL == (fpCodingList = fopen (strCodingList, "rb")))
	{
		printf ("Error when reading \"%s\" file.\n", strCodingList);
		return 0;
	}
	getCodingTable (fpCodingList, strTable);
	//进入文件夹
	if(_chdir( strPakDir ) )
	{
		switch (errno)
		{
			case ENOENT:
				printf( "Unable to locate the directory: %s\n", strPakDir );
				break;
			case EINVAL:
				printf( "Invalid buffer.\n");
				break;
			default:
				printf( "Unknown error.\n");
		}
		exit (0);
	}
	//到这里进入文件夹就成功了

	//获取bin文件的句柄

	if( (hFile = _findfirst( "*", &c_file )) == -1L )
	{
	}
	do
	{
		if (NULL != strstr(c_file.name, ".txt"))
		{
			continue;
		}
		strcpy (strSrc, c_file.name);
		sprintf (strDst, "%s.txt", strSrc);
		if (0 != (exportTxt (strSrc, strDst, strTable)))
		{
			printf ("Error when handle \"%s\" file.\n", strSrc);
		}
	}while( _findnext( hFile, &c_file ) == 0 );
	return 0;
}

//==================================================================
//函数名： 	getCodingTable
//作者：   	蒋乐天
//日期：   	2011.08.11
//功能：   	从给定的码表文件中将码表存入给定的字符串数组里面
//		这里我把索引为零的单独赋值为空格
//输入参数：	fp	(指向码表文件的头)
//		strTable(存储码表的数组)
//返回值：	类型(void) 
//修改记录：
//		2012.02.26 修改为双字节的字符存储
//==================================================================
void getCodingTable (FILE * fp, char strTable[CODING_LENGTH + 1][10])
{
	int iCount = 0;
	long lOffset = 0;
	char * stopStr;
	char strBuffer[MAX_STRING_LENGTH];
	while(fgets (strBuffer, MAX_STRING_LENGTH, fp) != NULL)
	{
		if (strBuffer[strlen(strBuffer)-1] == '\n')
		strBuffer[strlen(strBuffer)-1] = '\0'; 

		//FIXME:这里获取的十六进制数字有错误
		//从字符串中获取十六进制的数字
		lOffset = strtol(strBuffer, &stopStr, 16);

		//注意：strTable里面的字符串长度从3到6不等
		strcpy (strTable[lOffset], stopStr + 1);
		printf ("%d=%s\n", lOffset, strTable[lOffset]);
		++ iCount;

	}
	#ifdef	ENCODING_DEBUG
	printf ("iCount = %d\n", iCount);
	#endif

	//将未赋值的都标注为[XXh]或者[XXXXh]的形式
	for (iCount = 0; iCount < (CODING_LENGTH + 1); iCount ++ )
	{
		if (0 == strcmp (strTable[iCount], ""))
		{
			if (iCount <= 0x00ff)
				sprintf (strBuffer, "[%02Xh]", iCount);
			else
				sprintf (strBuffer, "[%04Xh]", iCount);
			strcpy (strTable[iCount], strBuffer);
		}
	}
}

//==================================================================
//函数名： 	exportTxt
//作者：   	蒋乐天
//日期：   	2012.02.25
//功能：   	根据码表和指定文件名开始导出文件中的句子
//输入参数：	char *strSrc 需导出的文件的文件名
//		char *strDst 输出文本的文件名
//		char strTable[CODING_LENGTH][10] 码表数组
//返回值：	类型(int) 
//		0 	导出正常
//		非零	导出有误
//修改记录：
//==================================================================
int exportTxt (char *strSrc, char *strDst, char strTable[CODING_LENGTH][10])
{
	BYTE byVal = 0,
	     *pbyBuf;
	WORD wVal = 0;
	int iCount = 0,
	    iCount2 = 0;
	FILE *fpSrc,
	     *fpDst;
	int iSentence = 0,
	    iLength = 0;
	DWORD dwOffset = 0;
	if (NULL == (fpSrc = fopen (strSrc, "rb")))
	{
		printf ("Error when reading \"%s\" file.\n", strSrc);
		return 1;
	}	

	if (NULL == (fpDst = fopen (strDst, "w")))
	{
		printf ("Error when creating \"%s\" file.\n", strDst);
		fclose (fpSrc);
		return 1;
	}	

	fread (&iSentence, sizeof (WORD), 1, fpSrc);

	for (iCount = 0; iCount < iSentence; ++ iCount)
	{
		fseek (fpSrc, iCount * 8 + 4, SEEK_SET);
		fread (&dwOffset, sizeof (DWORD), 1, fpSrc);
		fread (&iLength, sizeof (DWORD), 1, fpSrc);
		fseek (fpSrc, dwOffset, SEEK_SET);
		for (iCount2 = 0; iCount2 < iLength; iCount2 += 2)
		{
			wVal = 0;
			fread (&byVal, sizeof (BYTE), 1, fpSrc);
			wVal = byVal << 8;
			fread (&byVal, sizeof (BYTE), 1, fpSrc);
			wVal |= byVal;
			fputs (strTable[wVal], fpDst);
		}
		free (pbyBuf);
	}

	fclose (fpSrc);
	fclose (fpDst);

	return 0;
}

