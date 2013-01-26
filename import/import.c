/*
No.177
001C2EC0,59
－－－－－－－－－－－－－－－－
These waters are treacherous!
You shouldn't come back here!
－－－－－－－－－－－－－－－－
These waters are treacherous!
You shouldn't come back here!
－－－－－－－－－－－－－－－－
=========================================
17个减号（16个汉字+一个控制符）

当译文比原文长的时候需要有标记，方便导入时改变偏移地址
*/

#define DEBUG

#ifdef DEBUG
//#define ENCODING_TEST
//#define ENCODING_DEBUG
//#define TXT_READ_DEBUG
//#define DUMP_DEBUG
#define DEBUG_TRACE
#endif


#include "import.h"

void getCodingTable (FILE * fp, char chTable[][]);
void getCodingLength (char strTable[CODING_LENGTH + 1][10], int * iarrLength);
void insertCtrlChar (char chTable[][]);
int changeRomTxt (FILE *fpRom, FILE * fpSrcTxt, 
		char strTable[CODING_LENGTH + 1][10], int *iarrLength,
		txtExt *linkList);
int handleLine (char *strLine, char strTable[CODING_LENGTH + 1][10], int *iarrLength,
		BYTE *pbyteBuffer);
void skipBOM (FILE *fp);
char *fGetLine (char * strBuffer, FILE * fp);
int fGetCutLine (char * strBuffer, FILE * fp);
int renewTextAddr (FILE *fp, ULONG ulOldAddr, ULONG ulNewAddr);
txtExt *getAddrTable (FILE *fp);
int isInsertDirectly(ULONG ulAddr, txtExt * linkList);


#ifdef DEBUG_TRACE
FILE *fpDEBUG;
#endif

FILE *fpError;
long g_lErrorNo = 0;
BYTE *g_pbyRom;
long g_lRomLength = 0;
	
int main (int argc, char *argv[])
{
	int	iMenuParam,	//程序调用参数解析用
		iCount = 0;
	long lCount = 0;
	char	
		//Rom的文件名
		strRom [MAX_FILE_NAME_LENGTH] = "PMFRUS_CHPLUS_RELEASE3.gba",
		//码表的文件名
		strCodingList [MAX_FILE_NAME_LENGTH] = "codingList.txt",	
		//导入的文本的文件名
		strSrcTxt [MAX_FILE_NAME_LENGTH] = "text.txt",		
		strDirAddr [MAX_FILE_NAME_LENGTH] = "addrOffset.txt",
		strBuffer[MAX_STRING_LENGTH];

	FILE	* fpCodingList,		//指向码表的file指针
		* fpRom,		//指向gba的rom的file指针
		* fpSrcTxt,		//指向翻译文本的file指针
		* fpDirAddr;		//指向不要超长的文本的区间的file指针
	txtExt *linkList;

#ifdef DEBUG
	int iDebugCount;
#endif


#ifdef ENCODING_TEST
	FILE	* fpTest;
#endif
	int  iarrLength [CODING_LENGTH + 1];		//码表中各个字符串的长度
	char strTable[CODING_LENGTH + 1][10] = {0},	//游戏的对照码表
	     *stopStr;

#ifdef DEBUG_TRACE
	if (NULL == (fpDEBUG = fopen( "DEBUG.txt", "wb")))
	{
		printf ("Can't create \"DEBUG.txt\" file.\n");
		return 1;
	}
#endif

	//*************************************
	//对命令行的参数进行解析
	//*************************************
       	while ((iMenuParam = getopt (argc, argv, "a:c:s:r:")) != -1)
	{
		switch (iMenuParam)
		{
		//指定非超长处理的文件名
           	case 'a':
   			if ('-' == optarg[0])
			{
				printf ("Option -%c requires an argument.\n", iMenuParam);
				exit (0);
			}
			strcpy (strDirAddr, optarg);
			break;
		//指定码表文件名
           	case 'c':
   			if ('-' == optarg[0])
			{
				printf ("Option -%c requires an argument.\n", iMenuParam);
				exit (0);
			}
			strcpy (strCodingList, optarg);
			break;
		//指定导入文本文件名
           	case 's':
   			if ('-' == optarg[0])
			{
				printf ("Option -%c requires an argument.\n", iMenuParam);
				exit (0);
			}
			strcpy (strSrcTxt, optarg);
			break;
		//指定rom文件名
           	case 'r':
   			if ('-' == optarg[0])
			{
				printf ("Option -%c requires an argument.\n", iMenuParam);
				exit (0);
			}
			strcpy (strRom, optarg);
			break;
           	case '?':
			if ('c' == optopt || 'r' == optopt || 's' == optopt ||'a' == optopt)
				;
			else if (optopt == '?')
				;
			else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr, "Unknown option character `\\x%x'.\n",	optopt);
			printf ("口袋妖怪火红文本导入程序\n");
			printf ("%s [-r rom文件] [-c 码表文件] [-s 导入文本文件名] [-a 直接导入的文本区间文件]\n", argv[0]);
			return 1;
		default:
			abort ();
           	}
	}
	//**************************************
	//解析结束
	//**************************************

	//这里开始读取各个所需的rom和码表、文本区间

	//读取rom文件
	//并且复制到内存中
	if ( (fpRom = fopen (strRom, "rb+")) == NULL )
	{
		printf ("Error when loading rom \"%s\"\n", strRom);
		exit (0);
	}
	fseek (fpRom, 0, SEEK_END);
	g_lRomLength = sizeof (BYTE) * ftell (fpRom);
	g_pbyRom = (BYTE *)malloc (g_lRomLength);
	fseek (fpRom, 0, SEEK_SET);
	fread (g_pbyRom, sizeof (BYTE), g_lRomLength, fpRom);
	fseek (fpRom, 0, SEEK_SET);

	//读取码表文件
	if ( (fpCodingList = fopen (strCodingList, "rt")) == NULL )
	{
		printf ("Error when loading CodingList\n");
		exit (0);
	}

	//读取非超长的文本区间文件
	if ( (fpDirAddr = fopen (strDirAddr, "rb+")) == NULL )
	{
		printf ("Error when loading file \"%s\"\n", strDirAddr);
		exit (0);
	}

	//读取导入文本文件
	if ( NULL == (fpSrcTxt = fopen (strSrcTxt, "rt")))
	{
		printf ("Error when loading txt file:\"%s\"\n", strSrcTxt);
		exit (0);
	}

	if ( NULL == (fpError = fopen ("error.txt", "w")))
	{
		printf ("Error when creating \"error.txt\".\n");
		exit (0);
	}

	//将码表存入字符串数组strTable里面
	getCodingTable (fpCodingList, strTable);
	getCodingLength (strTable, iarrLength);

	linkList = getAddrTable(fpDirAddr);

#ifdef ENCODING_TEST
	fpTest = fopen ("test.txt", "wt");
	iCount = 0;
	while (iCount < CODING_LENGTH + 1)
	{
		if (iCount <= 0xff)
			fprintf (fpTest, "%02X = %s\n", iCount, strTable[iCount]);
		else
			fprintf (fpTest, "%04X = %s\n", iCount, strTable[iCount]);
		iCount++;
	}
	printf ("iCount = %d\n", iCount);
	fclose (fpTest);
#endif

	//开始根据码表和文本导入到rom中	
	changeRomTxt (fpRom, fpSrcTxt, strTable, iarrLength, linkList);

	free (g_pbyRom);
	fclose (fpRom);
	fclose (fpCodingList);
	fclose (fpSrcTxt);
#ifdef DEBUG_TRACE
	fclose (fpDEBUG);
#endif
	printf ("Done.\n");
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

		//从字符串中获取十六进制的数字
		lOffset = strtol(strBuffer, &stopStr, 16);

		//注意：strTable里面的字符串长度从3到6不等
		strcpy (strTable[lOffset], stopStr + 1);
		#ifdef ENCODING_DEBUG
			printf ("%d=%s\n", lOffset, strTable[lOffset]);
		#endif
		++ iCount;

	}
	#ifdef	ENCODING_DEBUG
	printf ("iCount = %d\n", iCount);
	#endif

	//单独赋给00为空格
	strcpy (strTable[0x00], " ");

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
//函数名：	insertCtrlCoding
//作者：	蒋乐天
//日期：	2011.08.14
//功能：	在码表中插入变量（不定长）
//		将相应的
//输入参数：	strTable	存放码表的字符串数组
//		iarrLength	存放码表字符串长度的数组
//返回值：	类型(void)
//修改记录：
//==================================================================
void insertCtrlCoding (char strTable[CODING_LENGTH + 1][10], int *iarrLength)
{
	//TODO:
	//	这里面还什么都没动。
	return;
}

//==================================================================
//函数名： 	getCodingLength
//作者：   	蒋乐天
//日期：   	2011.08.11
//功能：   	从给定的码表字符串数组获取各个字符串的长度，存入给定的长度数组
//输入参数：	strTable	存放码表的字符串数组
//		iarrLength	存放码表字符串长度的数组
//返回值：	类型(void) 
//修改记录：
//==================================================================
void getCodingLength (char strTable[CODING_LENGTH + 1][10], int * iarrLength)
{
	int iCount;
	for (iCount = 0; iCount < CODING_LENGTH + 1; ++ iCount)
	{
		iarrLength[iCount] = strlen (strTable[iCount]);
	}
	return;
}

//==================================================================
//函数名： 	changeRomTxt
//作者：   	蒋乐天
//日期：   	2011.08.11
//功能：   	导入文本的函数
//输入参数：	fpRom		指向目标rom的文件指针
//		fpSrcTxt	指向已翻译好文本的文件指针
//		strTable	已经存有码表的字符串数组
//		iarrLength	码表各个字符串的长度
//		linkList	指向非超长区间的指针
//返回值：	类型(int) 
//		0	成功
//		非0	导入有错误
//修改记录：
//==================================================================
int changeRomTxt (FILE *fpRom, FILE * fpSrcTxt,
	       	char strTable[CODING_LENGTH + 1][10], int *iarrLength, txtExt * linkList)
{
	int iSentenceCount = 0, 
	    iMissed = 0, 
	    iTmp,
	    iSentenceLength = 0,
	    iTranslatedLength = 0,
	    iSkip = 0;
	ULONG ulAddr = 0;
	static ULONG ulNewAddr = 0xEB1000;
	char strLine[MAX_STRING_LENGTH],
	     strBuffer[MAX_STRING_LENGTH],
	     *pchStopChar,
	     *szTmp;
	BYTE *pbyteBuffer;

	//可能存在的三字节bom头需要跳过（UTF-8编码）
	skipBOM(fpSrcTxt);

	while (NULL != (fGetLine (strLine, fpSrcTxt)))
	{
		//获取句子的序号
		if (0 != strncmp(strLine, "No.", 3))
		{
			continue;
		}
		iSentenceCount ++;
		sprintf (strBuffer, "No.%d", iSentenceCount);
		//句子序号有跳变，可能漏了几句文本
		if ( 0 != strcmp (strLine, strBuffer))
		{
			printf ("Maybe some sentence missed in txt.\n");
			iTmp = atoi (strLine+ 3);
			printf ("From %s jump to %s.\n\n", strBuffer, strLine);
			iMissed += iTmp - iSentenceCount;
			iSentenceCount = iTmp;
		}

		#ifdef TXT_READ_DEBUG
			printf ("Find \"%s\".\n", strLine);
		#endif
	
		//printf ("Start No. %d sentence.\n", iSentenceCount);
		g_lErrorNo = iSentenceCount;
		//序号完后应该是句子在rom中的地址和句子的长度
		if (NULL == (fGetLine (strLine, fpSrcTxt)))
		{
			printf ("No words after No.%d?", iSentenceCount);
			return -1;
		}

		//获取句子在rom中的地址
		ulAddr = strtol (strLine, &pchStopChar, 16);
		
		//获取原文的长度
		szTmp = pchStopChar + 2;
		iSentenceLength = strtol (szTmp, &pchStopChar, 10);

		//之后应该是分割线
		if (0 == fGetCutLine (strLine, fpSrcTxt))
		{
			return -1;
		}
		//跳过对原文的处理
		while (0 == fGetCutLine (strLine, fpSrcTxt))
			;
		//到这里获取完原文，下面的就是译文的处理
	
		iTranslatedLength = 0;
		pbyteBuffer = (BYTE *)malloc (sizeof(BYTE) * iSentenceLength * 4);
		memset (pbyteBuffer, 0, sizeof(pbyteBuffer));
		if ( NULL == pbyteBuffer)
		{
			printf ("Out of memory.\n");
			return 1;
		}

		iSkip = 0;
		//开始读取译文
		//逐行处理
		while (0 == fGetCutLine (strLine, fpSrcTxt))
		{
			int iLength = handleLine (strLine, strTable, iarrLength, pbyteBuffer + iTranslatedLength);
			if (iLength < 0) 
			{
				printf ("No. %d has unknow char.\n", iSentenceCount);
				sprintf (strBuffer, "No. %d has unknow char.\n", iSentenceCount);
				//fputs (strBuffer, fpError);
				iSkip = 1;
				while (0 == fGetCutLine (strLine, fpSrcTxt))
					;
				break;
			}
			else
				iTranslatedLength += iLength;
		}
		
		//如果有译文无法解析成功，则跳过该句话。
		if (!iSkip)
		{

			//写入rom中
			//在遇到译文比原文长的情况下，更新指针
			
			if (iTranslatedLength <= iSentenceLength || isInsertDirectly(ulAddr, linkList))
				//如果译文比原文短或者在特殊区间段，直接写入
			{
				//将pbyteBuffer中的数据根据ulAddr的地址写入rom中
				fseek (fpRom, ulAddr, SEEK_SET);
				fwrite (pbyteBuffer, sizeof (BYTE), iTranslatedLength, fpRom);
			}
			else
				//如果译文比原文长
			{
				//	如果译文比原文长，更新rom内的调用指针
				//	EB1000h - EBA000h
				//	AABBCCh地址在rom里面存为CCBBAA
				sprintf (strBuffer, "NO.%d longer than original.\n", iSentenceCount);
				fputs (strBuffer, fpError);
				sprintf (strBuffer, "From %d to %d long.\n", iSentenceLength, iTranslatedLength);
				fputs (strBuffer, fpError);
				//将pbyteBuffer中的数据根据ulNewAddr的地址写入rom中
				fseek (fpRom, ulNewAddr, SEEK_SET);
				fwrite (pbyteBuffer, sizeof (BYTE), iTranslatedLength, fpRom);

				if (renewTextAddr (fpRom, ulAddr, ulNewAddr))
				{
					sprintf (strBuffer, "Find no pointer.\n");
					fputs (strBuffer, fpError);
				}
				ulNewAddr += iTranslatedLength + 0x10;
				//printf ("\n");
			}
		}

		else 
		//在log里输出未解析成功的语句
		{
		}

		free (pbyteBuffer);

		//获取译文后面的分割线，到这里一句话的处理结束
	}
#ifdef TXT_READ_DEBUG
	printf ("iMissed = %d\n",iMissed);
#endif
	return 0;
}

//==================================================================
//函数名： 	handleLine
//作者：   	蒋乐天
//日期：   	2011.08.12
//功能：   	将一行译文根据新码表翻译成rom字节，并统计这一行译文的长度
//输入参数：	strLine		一行译文的字符串
//		strTable	中文码表
//		iarrLength	码表中各个字码实际长度
//		pbyteBuffer	需要导入到rom数据的指针
//返回值：	类型(int)
//		改行译文的长度
//		-1		存在码表中没有的字符
//修改记录：	2011.08.13
//		增加pbyteBuffer参数。
//
//==================================================================
int handleLine (char *strLine, char strTable[CODING_LENGTH + 1][10], int *iarrLength,
		BYTE *pbyteBuffer)
{
	int fUnknowCode = 0,
	    iLength = 0,
	    iCount = 0,
	    iStrLength = strlen (strLine);
	char *pchLocate = strLine,
	     *pchTmp,
	     strBuf[MAX_STRING_LENGTH];

	//开始分析这一行的字符直至结束
	while ((0 != strcmp (pchLocate, "")) 
		&& ((pchLocate - strLine) < iStrLength))
	{
		fUnknowCode = 1;

		//开始一个个对照码表里面的字符串，确认写入rom的字节
		for (iCount = 0; iCount <= CODING_LENGTH; ++ iCount)
		{
			//先分析普通字符（对应编码都是定长的）
			if (0 == strncmp (pchLocate, strTable[iCount], iarrLength[iCount]))
			//找到对应的码表了
			{
				if (iCount  <= 0xff)
				{
					* pbyteBuffer++ = iCount; 
					++ iLength;	//译文长度加一
				}
				else 
				{
					* pbyteBuffer++ =  (iCount & 0xff00) >> 8;
					* pbyteBuffer++ =  (iCount & 0x00ff);
					++ iLength;
					++ iLength;
				}
				//译文指针后移，为下一个字节做准备
				pchLocate += iarrLength[iCount];
				fUnknowCode = 0;
				break;
			}
			//不是码表中第iCount个对应的字符串，继续对照下一个
		}

		if (0 == fUnknowCode)
			goto FIND;

		//这里分析有关控制字符串的（就是变长字节的一些参数）
		for (iCount = 0; iCount <= CTRL_COUNT; ++iCount)
		{
			if (0 == strncmp (pchLocate, strTable[pwCtrlCodeIndex[iCount]], 2))
			{
				BYTE byVal;
				char *stopStr;
				* pbyteBuffer++ = pwCtrlCodeIndex[iCount]; 
				++ iLength;	//译文长度加一
				switch (pwCtrlCodeIndex[iCount])
				{
					//如果是[$]类型的变量型字符
					case 0xFD:
						byVal = strtol (pchLocate + 2, &stopStr, 10);
						* pbyteBuffer++ = byVal;
						pchLocate = strchr (stopStr, ']') + 1;
						++ iLength;
						fUnknowCode = 0;
						break;
					//如果是[c]类型的变量型字符
					case 0xFC:
						byVal = strtol (pchLocate + 4, &stopStr, 16);
						*pbyteBuffer++ = byVal;
						pchLocate = strchr (stopStr, ']') + 1;
						++ iLength;
						fUnknowCode = 0;
						break;
					default :
						break;
				}
				break;
			}
		}

		if (0 == fUnknowCode)
			goto FIND;

		//分析是不是形如[XXh]的二进制形式

		if ((0 != fUnknowCode) && ('[' == *pchLocate))
		{
			BYTE byVal;
			char *stopStr;
			byVal = strtol (pchLocate + 1, &stopStr, 16);
			pchLocate = strstr (stopStr, "h]");
			if (NULL == pchLocate)
			{
				puts ("error");
				pchLocate = stopStr;
			}
			else 
			{
				pchLocate += 2;
				* pbyteBuffer++ = byVal;
				++ iLength;
				fUnknowCode = 0;
			}
		}

		//如果遇到码表中不存在的字符，返回错误
		if (0 != fUnknowCode)
		{
#ifdef DEBUG_TRACE
			sprintf (strBuf, "NO.%d:\r\n", g_lErrorNo);
			fputs (strBuf, fpDEBUG);
			fputs (strLine, fpDEBUG);
			fputs ("\r\n", fpDEBUG);
			fputs (pchLocate, fpDEBUG);
			fputs ("\r\n", fpDEBUG);
			fputs ("--------------------------------\r\n", fpDEBUG);
#endif
			return -1;
		}
FIND:			;
	}
	//分析完成
	
#ifdef DUMP_DEBUG
	dumpHexData (stdout, pbyteBuffer, iLength);
	sleep (1000);
#endif

	return iLength;
}



//==================================================================
//函数名： 	skipBOM	
//作者：   	蒋乐天
//日期：   	2011.08.11
//功能：	让文件指针跳过可能存在的BOM头，使其指向真正的文件头
//输入参数：	fp	目标文件指针
//返回值：	类型(void)
//修改记录：
//==================================================================
void skipBOM (FILE *fp)
{
	BYTE byteBOM[3];
	fseek (fp, 0, SEEK_SET);
	fread (byteBOM, sizeof(BYTE), 3, fp);

	if ((byteBOM[0] == 0xEF) && (byteBOM[1] == 0xBB) && (byteBOM[2] == 0xBF))
		return;
	else
		fseek(fp, 0, SEEK_SET);
	return;		
}

//==================================================================
//函数名： 	fGetLine
//作者：   	蒋乐天
//日期：   	2011.08.11
//功能：   	从文件中获取一行文字放入字符串中
//输入参数：	strBuffer	用来存放一行文本的字符串
//		fp		获取文字的文件指针
//返回值：	类型(char *)
//		为所获取字符串的首地址，如果是NULL则表示文本已经结束
//修改记录：
//==================================================================
char *fGetLine (char * strBuffer, FILE * fp)
{
	char * strTmp;
	strTmp = fgets (strBuffer, MAX_STRING_LENGTH, fp);

	//去除结尾的换行符
	if (strBuffer[strlen(strBuffer)-1] == '\n')
		strBuffer[strlen(strBuffer)-1] = '\0'; 

	return strTmp;
}

//==================================================================
//函数名： 	fGetCutLine
//作者：   	蒋乐天
//日期：   	2011.08.11
//功能：   	从文件中获取一行16个全角字符的"－"，即分割线
//输入参数：	strBuffer	用来存放一行文本的字符串
//		fp		获取文字的文件指针
//返回值：	类型(int)
//		0	表示获得的不是分隔线
//		1	表示获得的是分割线
//修改记录：	2010.08.13
//		修改为之比较前面16个等号，防止被混乱的换行符误导
//==================================================================
int fGetCutLine (char * strBuffer, FILE * fp)
{
	fGetLine(strBuffer, fp);
	if ( 0 != strncmp (strBuffer, "－－－－－－－－－－－－－－－－", 16))
	{
		return 0;
	}
	return 1;
}

//==================================================================
//函数名： 	renewTextAddr 
//作者：   	蒋乐天
//日期：   	2011.10.22
//功能：	更新引用该句子的指针
//输入参数：	fp	目标文件指针
//		ulOldAddr 该句子原来的地址
//		ulNewAddr 该句子新的地址
//返回值：	类型(int)
//		0	表示更新成功
//		1	表示更新出错
//备注：	函数运行完文件指针仍指向原地点
//		
//		(待定)
//		176827h的指针是 27 68 17 08
//		（08 + 高地位置换）
//		FIXME:各种乱糟糟的问题
//==================================================================
int renewTextAddr (FILE *fp, ULONG ulOldAddr, ULONG ulNewAddr)
{
	long lOrignalFpOffset = ftell (fp),
	     lOffset;
	DWORD dwData;
	BYTE byOldAddr[4],
	     byNewAddr[4];

	byOldAddr[0] = (BYTE) (ulOldAddr & 0xff);
	byOldAddr[1] = (BYTE) ((ulOldAddr & (0xff << 8)) >> 8);
	byOldAddr[2] = (BYTE) ((ulOldAddr & (0xff << 16)) >> 16);
	byOldAddr[3] = (BYTE) 0x08;

	byNewAddr[0] = (BYTE) (ulNewAddr & 0xff);
	byNewAddr[1] = (BYTE) ((ulNewAddr & (0xff << 8)) >> 8);
	byNewAddr[2] = (BYTE) ((ulNewAddr & (0xff << 16)) >> 16);
	byNewAddr[3] = (BYTE) 0x08;


	fseek (fp, 0, SEEK_SET);

	for (lOffset = 0; lOffset < g_lRomLength; lOffset += 4)
	{
		if (byOldAddr[0] == g_pbyRom[lOffset +0])
		{
			if (byOldAddr[1] == g_pbyRom[lOffset +1])
			{
				if (byOldAddr[2] == g_pbyRom[lOffset +2])
				{
					if (byOldAddr[3] == g_pbyRom[lOffset +3])
					{
						fseek (fp, lOffset, SEEK_SET);
						//printf ("Find it in %Xh.\n", ftell (fp));
						fwrite (byNewAddr + 0, sizeof (BYTE), 1, fp);
						fwrite (byNewAddr + 1, sizeof (BYTE), 1, fp);
						fwrite (byNewAddr + 2, sizeof (BYTE), 1, fp);
						fwrite (byNewAddr + 3, sizeof (BYTE), 1, fp);
						lOffset = ftell (fp);
						fseek (fp, lOffset, SEEK_SET);
					}
				}
			}
		}
	}
	fseek (fp, lOrignalFpOffset, SEEK_SET);
	return 1;
}
//将记录文本区间的数据读入txtExt链表中，返回链表头
txtExt *getAddrTable (FILE *fp)
{
	txtExt *head = NULL;
	txtExt *linkListTmp = head;
	long lTmp = 0;
	char strTmp[MAX_STRING_LENGTH]={0},
		 *stopStr;

	while (NULL != fgets (strTmp, MAX_STRING_LENGTH, fp))
	{
		if ( 0 == (lTmp = strtol(strTmp, &stopStr, 16)))
		{
			continue;
		}
		
		if (NULL == head)
		{
			if (NULL == (head = linkListTmp = (txtExt *)malloc (sizeof (txtExt))))
			{
				printf ("Out of memory when malloc\n");
				exit (0);
			}
		}
		else
		{
			
			if (NULL == (linkListTmp -> next = (txtExt *)malloc (sizeof (txtExt))))
			{
				printf ("Out of memory when malloc\n");
				exit (0);
			}
			linkListTmp = linkListTmp ->next;
		}

		linkListTmp -> start = lTmp;

		lTmp = strtol (strTmp + 8, &stopStr, 16);
		linkListTmp -> end = lTmp;
		lTmp = strtol (strTmp + 15, &stopStr, 16);
		linkListTmp -> iWidth = (int)lTmp;
	}

	if (NULL != linkListTmp)
		linkListTmp -> next = NULL;
	#ifdef LINK_DEBUG		//DEBUG输出链表数据
	for (linkListTmp = head; NULL !=linkListTmp ; linkListTmp = linkListTmp -> next)
	{
		printf ("start = %Xh\t", linkListTmp -> start);
		printf ("end = %Xh\n", linkListTmp -> end);
	}
	#endif
	return head;
}

int isInsertDirectly(ULONG ulAddr, txtExt * linkList)
{
	while (NULL != linkList)
	{
		if ((ulAddr > (linkList -> start)) && (ulAddr < (linkList -> end)))
		{
			return 1;
		}
		linkList = linkList -> next;
	}
	return 0;
}
