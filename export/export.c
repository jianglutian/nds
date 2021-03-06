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

//#define DEBUG

#ifdef DEBUG
#define	ENCODING_TEST
//#define	ENCODING_DEBUG
#endif
//#define COLOR_TWO

#include "export.h"

void getCodingTable (FILE * fp, char chTable[][]);
void insertCtrlChar (char chTable[][]);
txtExt * getAddrTable (FILE *fp);
void clearAddrTable (txtExt *head);
void getParagraph (FILE *fpSrc, FILE *fpDst, txtExt * linkList, char strTable[][]);
void formattedOutput (FILE *fpSrc, FILE *fpDst,
						int iIndex, long lOffset, int iLength, int iWidth,
						char strTable[0xff + 1][10]);
void fprintfSP (FILE *fpDst, BYTE byteIndex,int *ipCount ,FILE *fpSrc, char strTable[0xff + 1][10]);


//特殊控制字符，从0xFA开始
char *ctrlChar[] = 
{
	"[翻页]","[换页]","[c]","[$]","[换行]","[结束]"
};

int main (int argc, char *argv[])
{
	txtExt *linkList;									//用来储存文本区间的结构体，链表类型的
	int c, 
		count = 0;
	long lCount = 0;
	char strRom [MAX_FILE_NAME_LENGTH] = "PMFRUS_CHPLUS_RELEASE3.gba",	//Rom的文件名 
	     strCodingList [MAX_FILE_NAME_LENGTH] = "codingList.txt",		//码表的文件名
	     strOffsetAddr [MAX_FILE_NAME_LENGTH] = "offsetAddr.txt",		//文本区间的文件名
		 strOutFile [MAX_FILE_NAME_LENGTH] = "text.txt",	//导出的文本的文件名
	     strBuffer[MAX_STRING_LENGTH];

	FILE	* fpCodingList,		//指向码表的file指针
			* fpRom,			//指向gba的rom的file指针
			* fpOffsetAddr,		//指向文本区间的file指针
			* fpOutTxt;			//指向导出文本的file指针

#ifdef DEBUG
	int iDebugCount;
#endif

#ifdef ENCODING_TEST
	FILE	* fpTest;
#endif

	char strTable[0xff + 1][10] = {0},	//游戏的对照码表
	     *stopStr;

       while ((c = getopt (argc, argv, "a:c:o:r:")) != -1)
		   switch (c)
		   {
		   case 'a':				
				if ('-' == optarg[0])
				{
					printf ("Option -%c requires an argument.\n", c);
					exit (0);
				}
				strcpy (strOffsetAddr, optarg);
				break;
           case 'c':
   				if ('-' == optarg[0])
				{
					printf ("Option -%c requires an argument.\n", c);
					exit (0);
				}
				strcpy (strCodingList, optarg);
				break;
           case 'o':
   				if ('-' == optarg[0])
				{
					printf ("Option -%c requires an argument.\n", c);
					exit (0);
				}
				strcpy (strOutFile, optarg);
				break;
           case 'r':
   				if ('-' == optarg[0])
				{
					printf ("Option -%c requires an argument.\n", c);
					exit (0);
				}
				strcpy (strRom, optarg);
				break;
           case '?':
				if (optopt == 'a' || 'c' == optopt || 'r' == optopt || 'o' == optopt)
					;
				else if (optopt == '?')
					;
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr,
							  "Unknown option character `\\x%x'.\n",
								optopt);
				fprintf (stdout, "口袋妖怪火红文本导出程序\n");
				fprintf (stdout, "%s [-r rom文件] [-a 文本区间文件] [-c 码表文件] [-o 输出文件名]\n", argv[0]);
				return 1;
			default:
				abort ();
           }

	//这里开始读取各个所需的rom和码表、文本区间

	//读取rom文件
		if (argc > 1)
	{
		strcpy (strRom, argv[1]);
	}
		if ( (fpRom = fopen (strRom, "rb")) == NULL )
	{
		printf ("Error when loading rom \"%s\"\n", strRom);
		exit (0);
	}
	//读取码表文件
	if ( (fpCodingList = fopen (strCodingList, "rt")) == NULL )
	{
		printf ("Error when loading CodingList\n");
		exit (0);
	}
	//读取文本区间
	if ( (fpOffsetAddr= fopen (strOffsetAddr, "rb")) == NULL )
	{
		printf ("Error when loading OffsetAddr\n");
		exit (0);
	}

	//将码表存入字符串数组strTable里面
	getCodingTable (fpCodingList, strTable);
	insertCtrlChar(strTable);


	//将记录文本区间的数据读入txtExt链表中，返回链表头
	linkList = getAddrTable (fpOffsetAddr);

	if (NULL == linkList)
	{
		printf ("Error when reading address offset.\n");
		exit (0);
	}

	if (NULL == (fpOutTxt = fopen (strOutFile, "wb+")))
	{
		printf ("Error when create %s file.\n", strOutFile);
		exit (0);
	}

	getParagraph (fpRom, fpOutTxt, linkList, strTable);

#ifdef ENCODING_TEST
	fpTest = fopen ("test.txt", "wb");
	count = 0;
	while (count < 0xff + 1)
	{
		fprintf (fpTest, "%2X = %s\n", count, strTable[count]);
		count++;
	}
	printf ("count = %d\n", count);
	fclose (fpTest);
#endif

	clearAddrTable (linkList);
	fclose (fpRom);
	fclose (fpCodingList);
	fclose (fpOffsetAddr);
	fclose (fpOutTxt);
	return 0;
}

//将码表存入字符串数组strTable里面
void getCodingTable (FILE * fp, char strTable[MAX_STRING_LENGTH][10])
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
		strcpy (strTable[lOffset], strBuffer + 3);
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
	for (iCount = 0; iCount < (0xff + 1); iCount ++ )
	{
		if (0 == strcmp (strTable[iCount], ""))
		{
			sprintf (strBuffer, "[%02Xh]", iCount);
			strcpy (strTable[iCount], strBuffer);
		}
	}
}


//将记录文本区间的数据读入txtExt链表中，返回链表头
txtExt *getAddrTable (FILE *fp)
{
	txtExt *head = NULL;
	txtExt *linkListTmp;
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

//清空地址链表
void clearAddrTable (txtExt *head)
{
	txtExt	*pLink1 = head,
			*pLink2 = head;
	while (NULL != pLink2)
	{
		pLink1 = pLink2;
		pLink2 = pLink1 -> next;
		free (pLink1);
	}
}

//获取一段对话
void getParagraph (FILE *fpSrc, FILE *fpDst, txtExt * linkList, char strTable[0xff + 1][10])
{
	int iSentenceCount = 0,
		iLength;
	long lAddrOffset = 0;
	BYTE byteTmp ;

	//该层循环用于在各个文本区间里面导出文本
	for (; NULL != linkList; linkList = linkList -> next)
	{
		lAddrOffset = linkList -> start;
		fseek (fpSrc, lAddrOffset, SEEK_SET);


		//该层循环用于在某个文本区间中导出具体句子
		while (lAddrOffset <= (linkList -> end))
		{
			fread (&byteTmp, sizeof (BYTE), 1, fpSrc);
			iLength = 1;
			
			//遍历完整的一句话
			while ((BYTE)0xff != byteTmp)		
			{
				fread (&byteTmp, sizeof (BYTE), 1, fpSrc);
#ifdef OUTPUT_TEST

				printf ("%02X ", (int)byteTmp);
#endif
				iLength ++;
			}
#ifdef OUTPUT_TEST
			printf ("\n");
			sleep (1000);
#endif
			iSentenceCount ++;
			formattedOutput (fpSrc, fpDst, 
				iSentenceCount, lAddrOffset, iLength, linkList -> iWidth, 
				strTable);
			lAddrOffset = ftell (fpSrc);
		}
	}
}


//格式化输出一段话
void formattedOutput (	FILE *fpSrc, FILE *fpDst, 
						int iIndex, long lOffset, int iLength, int iWidth, 
						char strTable[0xff + 1][10]
						)
{
	int iCount, iTmp;
	BYTE byteTmp;

	//消除前面多余的空格
	fseek (fpSrc, lOffset, SEEK_SET);
	fread (&byteTmp, sizeof (BYTE), 1, fpSrc);
	while ( 0x00 == byteTmp )
	{
		lOffset ++, iLength --;
		fread (&byteTmp, sizeof (BYTE), 1, fpSrc);

	}
	fseek (fpSrc, -1, SEEK_CUR);

//	#ifdef DEBUG
//		printf ("Start in %Xh, %d chars long.\n", lOffset, iLength);
//	#endif

	fprintf (fpDst, "No.%d\r\n", iIndex);
	fprintf (fpDst, "%08Xh,%d\r\n", lOffset, iLength);
	fprintf (fpDst, "－－－－－－－－－－－－－－－－\r\n");
	fseek (fpSrc, lOffset, SEEK_SET);
	iTmp = iLength;
	do
	{
		for (iCount = 0; iCount < iTmp; iCount ++)
		{
			fread (&byteTmp, sizeof (BYTE), 1, fpSrc);
			if (byteTmp == 0xFA||byteTmp == 0xFB||byteTmp == 0xFE)
			{
				fprintf (fpDst, "%s", strTable[byteTmp]);	
				iCount ++;
				break;
			}
			else
			{
				fprintfSP (fpDst, byteTmp, &iCount, fpSrc, strTable);
			}
		}
		fprintf (fpDst, "\r\n");
		iTmp -= iCount;


	}
	while (iTmp > 0);

	fprintf (fpDst, "－－－－－－－－－－－－－－－－\r\n");
	fseek (fpSrc, lOffset, SEEK_SET);
	iTmp = iLength;
	do
	{
		for (iCount = 0; iCount < iTmp; iCount ++)
		{
			fread (&byteTmp, sizeof (BYTE), 1, fpSrc);
			if (byteTmp == 0xFA||byteTmp == 0xFB||byteTmp == 0xFE)
			{
				fprintf (fpDst, "%s", strTable[byteTmp]);	
				iCount ++;
				break;
			}
			else
			{
				fprintfSP (fpDst, byteTmp, &iCount, fpSrc, strTable);
			}
		}
		fprintf (fpDst, "\r\n");
		iTmp -= iCount;

	}
	while (iTmp > 0);
	fprintf (fpDst, "－－－－－－－－－－－－－－－－\r\n\r\n");

	if (0 != iWidth)
	{
		fseek (fpSrc, lOffset + iWidth, SEEK_SET);
	}
}


//在程序需要的码表中插入特殊控制符
void insertCtrlChar (char strTable[0xff + 1][10])
{
	int index = 0xFA, count = 0;
	for (index = 0xFA, count = 0; index <= 0xFF; index ++, count ++)
	{
		strcpy (strTable[index], ctrlChar[count]);
	}
}

//输出包含变量的句子
void fprintfSP (FILE *fpDst, BYTE byteIndex,int *ipCount ,FILE *fpSrc, char strTable[0xff + 1][10])
{
	BYTE byteColor, byteVar;
	BYTE byteColor1, byteColor2;
	char strTmp [MAX_STRING_LENGTH] = {0};
	switch (byteIndex)
	{
	case 0xFC:
#ifdef COLOR_TWO
		fread (&byteColor1, sizeof (BYTE), 1, fpSrc);
		fread (&byteColor2, sizeof (BYTE), 1, fpSrc);
		fprintf (fpDst, "[c0x%02X%02X]", byteColor1, byteColor2);
		*ipCount += 2;
#else
		fread (&byteColor, sizeof (BYTE), 1, fpSrc);
		fprintf (fpDst, "[c0x%02X]", byteColor);
		*ipCount += 1;
#endif
		break;
	case 0xFD:
		fread (&byteVar, sizeof (BYTE), 1, fpSrc);
#ifndef VAR_SPECIAL
		fprintf (fpDst, "[$%1d]", byteVar);
#else
		switch (byteVar)
		{
		case 0x01:
			byteVar ='n';
			break;
		case 0x05:
			byteVar ='f';
			break;
		case 0x02:
			byteVar ='1';
			break;
		case 0x03:
			byteVar ='2';
			break;
		case 0x04:
			byteVar ='3';
			break;
		case 0x06:
			byteVar ='4';
			break;
		case 0x07:
			byteVar ='5';
			break;
		default:
			break;		
		}

		fprintf (fpDst, "[$%c]", byteVar);

#endif
		*ipCount += 1;
		break;
	default:
		fprintf (fpDst, "%s", strTable[byteIndex]);
	}
}
