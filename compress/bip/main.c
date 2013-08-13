/* 
 * 前四个字节是解压后的大小，后面的数据格式为“ 控制符 + 压缩数据 + 控制符 + 压缩数据…………” 
 * 控制符定义：1未压缩，0复制窗口数据。
 * 控制字节ab cd, 其中d+3为复制的长度，cab为要在窗口复制的绝对位置（不是后退量，不会出现在写入指针后面16格之内）。
 * 需要建立长度是4096的窗口，解压前全部用0初始化，解压时首次写入的位置是4078（也就是将压缩文件中的原文的写入）。
 * 另外窗口是环形的读/写，也就是读/写完最后的位置就要跳到最开始的位置读/写入。
 */

#include <stdio.h>

#include "../../common.h"

#define WINDOW_LENGTH 4096
#define CACHE_LENGTH 20
#define MAX_DATA_LENGTH 18
#define BUFFER_LENGTH	20

#define DEBUG_OUT

#ifdef DEBUG_OUT
FILE *fpDEBUG;
#endif

void startBipCompress (FILE *fpSrc, FILE *fpDst);
int isSameInWindowData (BYTE *pbyWrite, BYTE *pbyWindow, BYTE *pbyData, BYTE **ppbyRead);
void dumpCache (BYTE *pbyCache);
void dumpWindowData (BYTE *pbyWindow, BYTE *pbyLocate, int iLength);
int readCacheData (FILE *fp, BYTE *pbyReadCache, BYTE **ppbyRead, BYTE *pbyParam);
int readAndRefreashCache (FILE *fp, BYTE *pbyReadCache, BYTE **ppbyRead);
void writeDataIntoBuffer (FILE *fp, BYTE *pbyControl, int *piCtrlLen,
			BYTE *pbyWrite, BYTE *pbyData, int iDataLen);
inline void checkWindowPoint (BYTE *pbyWindow, BYTE **ppbyPoint);
void writeWindowData (BYTE *pbyWindow, BYTE **ppbyWrite, BYTE *pbyData, int iLength);


int main (int argc, char *argv[])
{
	char strSrc[MAX_FILE_NAME_LENGTH] = "is.bin",
	     strDst[MAX_FILE_NAME_LENGTH] = "";
	char *pch;
	char c;
	DWORD dwLength;
	FILE *fpSrc,	//待压缩的包的文件指针 
	     *fpDst;

#ifdef DEBUG_OUT
	if (NULL == (fpDEBUG = fopen("debug.bin", "wb")))
	{
		printf ("Error when creating debug file.\n");
		exit (0);
	}	
#endif
	////////////////////////////////////////
	//输入参数解析
	
	while ((c = getopt (argc, argv, "s:o:")) != -1)
		switch (c)
		{
			case 's':
				if ('-' == optarg[0])
				{
					printf ("Option -%c requires an argument.\n", c);
					exit (0);
				}
				strcpy (strSrc, optarg); 
				break;
			case 'o':
				if ('-' == optarg[0])
				{
					printf ("Option -%c requires an argument.\n", c);
					exit (0);
				}
				strcpy (strDst, optarg); 
				break;
			case '?':
				if (optopt == 's' || optopt == 'o')
					;
				else if (optopt == '?')
					;
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
				fprintf (stdout, "bip文件压缩程序\n");
				fprintf (stdout, "%s [-s 源文件名] [-o 输出文件名]\n", argv[0]);
				return 1;
			default:
				abort ();
		}

	//参数解析完成
	////////////////////////////////////////

	//输出文件后缀名为bip，替换原文件的后缀名（没有则直接添加）。
	if (0 == strcmp (strDst, ""))
	{
		strcpy (strDst, strSrc);
		if (NULL != (pch = strrchr (strDst, '.')))
		{
			strcpy (pch, ".bip");
		}
		else
		{
			strcat (strDst, ".bip");
		}
	}

	//打开源文件
	if (NULL == (fpSrc = fopen (strSrc, "rb")))
	{
		printf ("Error when reading \"%s\" file.\n", strSrc);
		return 1;
	}

	//打开目标文件
	if (NULL == (fpDst = fopen (strDst, "wb")))
	{
		printf ("Error when creating \"%s\" file.\n", strDst);
		return 1;
	}

	//获取包的大小，写入压缩包前4个字节
	fseek (fpSrc, 0, SEEK_END);
	dwLength = ftell (fpSrc);
	fwrite (&dwLength, sizeof (DWORD), 1, fpDst);
	fseek (fpSrc, 0, SEEK_SET);

	//开始压缩写入压缩包
	startBipCompress (fpSrc, fpDst);

	fclose (fpDst);
	fclose (fpSrc);
#ifdef DEBUG_OUT
	fclose (fpDEBUG);
#endif
	printf ("Done.\n");

	return 0;
}

void startBipCompress (FILE *fpSrc, FILE *fpDst)
{
	BYTE	byControl,
		arrbyCpmData[2];
	BYTE	*pbyWindow,			//动态字典窗口
		*pbyWindowRead,			//字典读取的指针
		*pbyWindowWrite,		//字典写入的指针
		arrbyWriteBuffer[BUFFER_LENGTH] = {0},	//用于存储即将写入的压缩数据的
		arrbyReadCache[CACHE_LENGTH] = {0},	//用于从源文件读取临时储存的地方
		*pbyReadBuffer;			//指向下一个源文件需要读取的数据
	int iLastLen,				//最后未处理的数据的长度
	    iDataLength,			//字典内存在的相同子串的长度
	    iCtrlLen;				//控制符已使用的位数
	
	//初始化一些压缩过程中遇到的变量
	pbyWindow = calloc (WINDOW_LENGTH, sizeof (BYTE));
	iCtrlLen = 0;
	pbyWindowWrite = &pbyWindow[4078];	//窗口写入地址从4078开始
	if (NULL == pbyWindow || NULL == arrbyWriteBuffer)
	{
		printf ("Memory full.\n"
			"Soft stopped.\n");
		return ;
	}

	///////////////////////////////////////////
	//开始读取源文件进行压缩。
	pbyReadBuffer = arrbyReadCache + CACHE_LENGTH;	//设置数据读取指针的位置使其第一次正确读取数据

	//读取源文件数据并开始压缩写入压缩文件中
	//FIXME:
	//	测试出来压缩的数据再解压有很多数据出错。
	while (0 == (iLastLen = readAndRefreashCache(fpSrc, arrbyReadCache, &pbyReadBuffer)))
	{
#ifdef DEBUG_OUT
		fwrite (pbyReadBuffer, sizeof (BYTE), 1, fpDEBUG);
#endif
		iDataLength = isSameInWindowData(pbyWindowWrite, pbyWindow, pbyReadBuffer, &pbyWindowRead);
		if (iDataLength >= 3)
		{
			writeWindowData (pbyWindow, &pbyWindowWrite, pbyReadBuffer, iDataLength); 
			arrbyCpmData[0] = (pbyWindowRead - pbyWindow) && 0xff;
			arrbyCpmData[1] = ((pbyWindowRead - pbyWindow) && 0xf00) >> 1 | iDataLength - 3;
			writeDataIntoBuffer (fpDst, &byControl, &iCtrlLen, arrbyWriteBuffer, arrbyCpmData, 2);
			pbyReadBuffer += iDataLength;
		}
		else 
		{
			writeWindowData (pbyWindow, &pbyWindowWrite, pbyReadBuffer, 1); 
			writeDataIntoBuffer (fpDst, &byControl, &iCtrlLen, arrbyWriteBuffer, pbyReadBuffer ++, 1);
		}
	}
/*
	//TODO:
	//	最后包含文件结尾的数据处理（长度为iLastLen，不是原来定义的CACHE_LENGTH）
	while (iLastLen > 0)
	{
		iDataLength = isSameInWindowData(pbyWindowWrite, pbyWindow, pbyReadBuffer, &pbyWindowRead);
		if ((iDataLength > 3) && (iDataLength <= iLastLen))
		{
			fwrite (pbyReadBuffer, sizeof (BYTE), iDataLength, fpDst);
			pbyReadBuffer += iDataLength;
			iLastLen -= iDataLength;
		}
		else
		{
			fwrite (pbyReadBuffer ++, sizeof (BYTE), 1, fpDst);
			-- iLastLen;
		}
	}
*/	
	free (pbyWindow);
	return;
}


////////////////////////////////////////
//名称：isSameInWindowData
//作用：判断是否在字典窗口里面有对应的数据串
//参数：BYTE *pbyWrite		当前字典窗口的写指针地址
//	BYTE *pbyWindow		字典窗口的首地址
//	BYTE *pbyData		需要比较的目标数据串
//	BYTE **ppbyRead		匹配数据串的首地址，未找到则为NULL
//返回：int 	零表示在窗口字典里未找到相同的数据串
//		非零表示已经在字典里找到相同的数据串，其值为最大的匹配长度
////////////////////////////////////////
int isSameInWindowData (BYTE *pbyWrite, BYTE *pbyWindow, BYTE *pbyData, BYTE **ppbyRead)
{
	BYTE *pbyCheckStart = pbyWrite,
	     *pbyCheck = pbyWrite,
	     *pbyTmp = pbyData;
	int iCount,
	    iOffset,
	    iSameLength,
	    iMaxSameLength;
	*ppbyRead = NULL;

	//开始从字典里的写指针之前一个个倒退匹配数据串
	for (iCount = 0, iMaxSameLength = iSameLength = 0; iCount < WINDOW_LENGTH - MAX_DATA_LENGTH; ++ iCount)
	{
		iOffset = iCount + 1;
		pbyCheck = pbyWrite - iOffset - 1;
		checkWindowPoint (pbyWindow, &pbyCheck);
		pbyCheckStart = pbyCheck;
		pbyTmp = pbyData;

		if (iOffset < MAX_DATA_LENGTH)
		//如果是循环子串（即解压时读指针会读取写指针新写入的数据）
		{
			while (iSameLength < MAX_DATA_LENGTH)
			{
				if (*pbyCheck++ != *pbyTmp++)
				{
					if (iSameLength > iMaxSameLength)
					{
						iMaxSameLength = iSameLength;
						*ppbyRead = pbyCheckStart;
						checkWindowPoint (pbyWindow, ppbyRead);
					}
					break;
				}
				//循环子串的复位
				if (iSameLength >= iOffset)
					pbyCheck = pbyCheckStart;
				checkWindowPoint (pbyWindow, &pbyCheck);
			}
		}
		else
		//如果不是循环子串
		{
			while (iSameLength < MAX_DATA_LENGTH)
			{
				if (*pbyCheck++ != *pbyTmp++)
				{
					if (iSameLength > iMaxSameLength)
					{
						iMaxSameLength = iSameLength;
						*ppbyRead = pbyCheckStart;
						checkWindowPoint (pbyWindow, ppbyRead);
					}
					break;
				}
				checkWindowPoint (pbyWindow, &pbyCheck);
				++iSameLength;
			}
		}
		iSameLength = 0;
	}
	return iMaxSameLength;
}

////////////////////////////////////////////
//名称：checkWindowPoint
//作用：检测指向字典的指针是否越界，如果越界则重置为字典开始处或字典结尾处
//	（即表示为头尾连接的循环字典）
//参数：BYTE *pbyWindow 指向字典窗口的首地址
//	BYTE **ppbyPoint待检测是否越界的指针的地址
//返回：void
////////////////////////////////////////////
inline void checkWindowPoint (BYTE *pbyWindow, BYTE **ppbyPoint)
{
	if (*ppbyPoint >= pbyWindow + WINDOW_LENGTH)
		*ppbyPoint = pbyWindow;
	else if (*ppbyPoint < pbyWindow)
		*ppbyPoint = pbyWindow + WINDOW_LENGTH;
}

////////////////////////////////////////////
//名称：writeWindowData
//作用：在循环的字典窗口中写入数据
//参数：BYTE *pbyWindow		字典窗口的起始地址
//	BYTE **ppbyWrite	字典窗口写入的初始地址
//	BYTE *pbyData		待写入数据的起始地址
//	int iLength		写入数据的长度
//返回：void
////////////////////////////////////////////
void writeWindowData (BYTE *pbyWindow, BYTE **ppbyWrite, BYTE *pbyData, int iLength)
{
	while (iLength-- > 0)
	{
		*(*ppbyWrite)++ = *pbyData++;
		checkWindowPoint (pbyWindow, ppbyWrite);
	}
}

////////////////////////////////////////////
//名称：readAndRefreashCache
//作用：更新缓存内的数据，使得能容纳一次字典内的最长子串
//参数：FILE *fp		数据的源文件
//	BYTE *pbyReadCache	读取数据的缓存
//	BYTE **ppbyRead		当前数据读取的指针
//返回：int	零  代表成功读取一个
//		非零代表读取到文件末尾，其值为剩余数据的长度
////////////////////////////////////////////
int readAndRefreashCache (FILE *fp, BYTE *pbyReadCache, BYTE **ppbyRead)
{
	int iCount;
	int iOffset = *ppbyRead - pbyReadCache;
	BYTE *pbyTmp = pbyReadCache;

	for (iCount = 0; iCount < (CACHE_LENGTH - iOffset); iCount ++)
	{
		*pbyTmp++ = *(*ppbyRead)++;
	}

	for (iCount = 0; iCount < iOffset; iCount ++)
	{
		fread (pbyTmp++, sizeof (BYTE), 1, fp);
		if (feof (fp))
		{
			*ppbyRead = pbyReadCache;
			return pbyTmp - pbyReadCache - 1;
		}
	}
	*ppbyRead = pbyReadCache;
	return 0;
}

////////////////////////////////////////////
//名称：writeDataIntoBuffer
//作用：更新待写入数据内容，如果控制符已用满则写入目标文件
//参数：FILE *fp		目标文件指针
//	BYTE *pbyControl	控制符
//	int *piCtrlLen		已使用的控制符的位数
//	BYTE *pbyWrite		写入缓存的首地址
//	BYTE *pbyData		待加入写入缓存的数据地址
//	int iDataLen		待加入的数据长度
//返回：void
////////////////////////////////////////////
void writeDataIntoBuffer (FILE *fp, BYTE *pbyControl, int *piCtrlLen,
			BYTE *pbyWrite, BYTE *pbyData, int iDataLen)
{
	BYTE byControl = *pbyControl;
	int iCount,
	    iCtrlLen = *piCtrlLen,
	    iBufferLen = 0;
	
	//计算出缓存中已经拥有的数据长度
	for (iCount = 0; iCount < iCtrlLen; ++ iCount)
	{
		if ((0x01 << iCount) & byControl)
			iBufferLen += 1;
		else
			iBufferLen += 2;
	}

	if (1 == iDataLen)
	//未压缩数据，控制符写入1，缓存中写入原数据
	{
		pbyWrite[iBufferLen ++] = *pbyData;
		byControl |= (0x01 << iCtrlLen ++);
	}
	else
	//压缩数据，控制符写入0，缓存中写入压缩控制字节
	{
		pbyWrite[iBufferLen ++] = *pbyData++;
		pbyWrite[iBufferLen ++] = *pbyData;
		byControl &= ~(0x01 << iCtrlLen ++);
	}

	//如果控制符用满，则将控制符和压缩后的数据写入文件中
	if (8 == iCtrlLen)
	{
		//dumpCache (pbyWrite);
		WAIT;
		fwrite (&byControl, sizeof (BYTE), 1, fp);
		fwrite (pbyWrite, sizeof (BYTE), iBufferLen, fp);
		iCtrlLen = 0;
		byControl = 0;
	}

	*pbyControl = byControl;
	*piCtrlLen = iCtrlLen;
	return;
}

////////////////////////////////////////////
//名称：dumpCache
//作用：输出读取文件中缓存的数据（调试用）
//参数：BYTE *pbyCache	指向缓存的指针
//返回：void
////////////////////////////////////////////
void dumpCache (BYTE *pbyCache)
{
	int iCount;
	printf ("////////////////////\n");
	printf ("Cache data:\n");
	for (iCount = 0; iCount < CACHE_LENGTH; ++ iCount)
	{
		printf ("%2Xh ", pbyCache[iCount]);
		if (! ((iCount + 1) % 8))
			printf ("\n");
	}
	printf ("\n");
	printf ("////////////////////\n");
}

////////////////////////////////////////////
//名称：dumpWindowData
//作用：输出字典窗口中的数据（调试用）
//参数：BYTE *pbyWindow 指向字典窗口的指针
//	BYTE *pbyLocate	指向想输出的起始地址，
//			如果为NULL则默认从字典开头算起
//	int iLength	想输出的数据的长度，
//			如果为0则默认为输出到字典的结束
//返回：void
////////////////////////////////////////////
void dumpWindowData (BYTE *pbyWindow, BYTE *pbyLocate, int iLength)
{
	int iCount;
	if (NULL == pbyLocate)
	{
		pbyLocate = pbyWindow;
	}
	if ( 0 == iLength)
	{
		iLength = WINDOW_LENGTH - (pbyLocate - pbyWindow);
	}
	printf ("////////////////////\n");
	printf ("Window data:\n");
	for (iCount = 0; iCount < iLength; ++ iCount)
	{
		if (&pbyLocate[iCount] >= &pbyWindow[WINDOW_LENGTH])
		{
			pbyLocate -= WINDOW_LENGTH;
		}
		printf ("%2Xh ", pbyLocate[iCount]);
		if (! ((iCount + 1) % 16))
			printf ("\n");
	}
	if (((iCount + 1) % 16))
		printf ("\n");
	printf ("////////////////////\n");
}

