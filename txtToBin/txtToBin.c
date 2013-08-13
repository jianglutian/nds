#include <stdio.h>

#ifndef COMMON_H
typedef unsigned char BYTE;
#endif

#define DEBUG

int main(int argc, char *argv[])
{
	char strSrc [256] = "123.txt",
		 strDst [256] = "123.bin",
		 strBuf [256] = {0};

	char *stopStr;

	unsigned long ulBuf;
	int iCount;
	BYTE byteBuf;

	FILE *fpSrc,
		 *fpDst;

	if (argc > 1)
	{
		strcpy (strSrc, argv[1]);
		strcpy (strDst, strSrc);
		strcpy (strrchr (strDst, '.'), ".bin");
	}

	#ifdef DEBUG
		puts (strSrc);
		puts (strDst);
	#endif

	if (NULL == (fpSrc = fopen (strSrc, "r")))
	{
		printf ("Error when opening \"%s\" file.\n", strSrc);
		exit (0);
	}

	if (NULL == (fpDst = fopen (strDst, "wb")))
	{
		printf ("Error when creating \"%s\" file.\n", strDst);
		exit (0);
	}

	while (NULL != fgets (strBuf, 256, fpSrc))
	{
		if (strBuf[strlen(strBuf)-1] == '\n')
			strBuf[strlen(strBuf)-1] = '\0'; 

		for ( iCount = 0; iCount < 16; iCount ++)
		{
			ulBuf = strtol (strBuf + 9 + (iCount * 3), &stopStr, 16) ;
			if (ulBuf > 0xff)
			{
				printf ("Error when reading data.\n");
				exit (0);
			}
			byteBuf = (BYTE) ulBuf;
			fwrite (&byteBuf, sizeof (BYTE), 1, fpDst);
		}
	}
	fclose (fpSrc);
	fclose (fpDst);
	return 0;
}
