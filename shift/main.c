//////////////////////////////////////////////////////////////////
//每次读取四个字节
//第4个字节低2位左移6位，跟第1字节高6位右移两位后or	//第1个新字节
//第1个字节低2位左移6位，跟第2字节高6位右移两位后or	//第2个新字节
//第2个字节低2位左移6位，跟第3字节高6位右移两位后or	//第3个新字节
//第3个字节低2位左移6位，跟第4字节高6位右移两位后or	//第4个新字节
//////////////////////////////////////////////////////////////////
#include "stdio.h"

#include "../common.h"

BYTE getNewByte (BYTE byLow, BYTE byHigh);
BYTE getOldByte (BYTE byLow, BYTE byHigh);

int main(int argc, char *argv[])
{
	char strSrc[MAX_FILE_NAME_LENGTH] = "in.bin",
	     strDst[MAX_FILE_NAME_LENGTH] = "out.bin",
	     strBuf[MAX_STRING_LENGTH];
	FILE *fpSrc,
	     *fpDst;
	BYTE arrbyReadBuf[4] = {0},
	     arrbyWriteBuf[4] = {0};
	int iReaded = 0,
	    iCount = 0,
	    fReserve = 0,
	    iOffset = 0;
	int c;

	////////////////////////////////////////
	//输入参数解析
	
	while ((c = getopt (argc, argv, "s:o:r:n:")) != -1)
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
			case 'r':
				if ('-' == optarg[0])
				{
					printf ("Option -%c requires an argument.\n", c);
					exit (0);
				}
				strcpy (strBuf, optarg); 
				iOffset = atoi (strBuf);
				fReserve = 1;
				break;
			case 'n':
				if ('-' == optarg[0])
				{
					printf ("Option -%c requires an argument.\n", c);
					exit (0);
				}
				strcpy (strBuf, optarg); 
				iOffset = atoi (strBuf);
				break;
			case '?':
				if (optopt == 's' || optopt == 'o' || optopt == 'r' || optopt =='n')
					;
				else if (optopt == '?')
					;
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
				fprintf (stdout, "Shift小程序\n");
				fprintf (stdout, "%s [-s 源文件名] [-o 输出文件名] [-r（恢复） 起始偏移地址]\n", argv[0]);
				return 1;
			default:
				abort ();
		}

	//参数解析完成
	////////////////////////////////////////

	if (NULL == (fpSrc = fopen (strSrc, "rb")))
	{
		printf ("Error when reading \"%s\" file.\n", strSrc);
		exit (0);
	}

	if (NULL == (fpDst= fopen (strDst, "wb")))
	{
		printf ("Error when creating \"%s\" file.\n", strDst);
		exit (0);
	}

	printf ("%d\n", iOffset);
	fread (arrbyReadBuf, sizeof(BYTE), iOffset, fpSrc);
	fwrite (arrbyReadBuf, sizeof (BYTE), iOffset, fpDst);

	while (4 == (iReaded = fread (arrbyReadBuf, sizeof (BYTE), 4, fpSrc)))
	{
		iCount = 0;
		if (0 == fReserve)
		{
			arrbyWriteBuf[iCount++] = getNewByte (arrbyReadBuf[3], arrbyReadBuf[0]);
			arrbyWriteBuf[iCount++] = getNewByte (arrbyReadBuf[0], arrbyReadBuf[1]);
			arrbyWriteBuf[iCount++] = getNewByte (arrbyReadBuf[1], arrbyReadBuf[2]);
			arrbyWriteBuf[iCount++] = getNewByte (arrbyReadBuf[2], arrbyReadBuf[3]);
		}
		else
		{
			arrbyWriteBuf[iCount++] = getOldByte (arrbyReadBuf[1], arrbyReadBuf[0]);
			arrbyWriteBuf[iCount++] = getOldByte (arrbyReadBuf[2], arrbyReadBuf[1]);
			arrbyWriteBuf[iCount++] = getOldByte (arrbyReadBuf[3], arrbyReadBuf[2]);
			arrbyWriteBuf[iCount++] = getOldByte (arrbyReadBuf[0], arrbyReadBuf[3]);
		}
		fwrite (arrbyWriteBuf, sizeof (BYTE), 4, fpDst);
		memset (arrbyWriteBuf, 0, sizeof (BYTE) / sizeof (size_t) * 4);
	}

	fwrite (arrbyReadBuf, sizeof (BYTE), iReaded, fpDst);

	fclose (fpSrc);
	fclose (fpDst);
	printf ("Done.\n");
	return 0;
}

BYTE getNewByte (BYTE byLow, BYTE byHigh)
{
	BYTE byVal = 0;
	byVal = ((byLow & 0x03) << 6) | ((byHigh & 0xFC) >> 2);
	//printf ("%02Xh %02Xh\n", byLow, byHigh);
	//printf ("%02Xh\n", byVal);
	//WAIT;
	return byVal;
}

BYTE getOldByte (BYTE byLow, BYTE byHigh)
{
	BYTE byVal = 0;
	byVal = ((byLow & 0xC0) >> 6) | ((byHigh & 0x3F) << 2);
	//printf ("%02Xh %02Xh\n", byLow, byHigh);
	//printf ("%02Xh\n", byVal);
	//WAIT;
	return byVal;
}

