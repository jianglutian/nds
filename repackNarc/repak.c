/*
 *	narc打包工具
 *
 */
#include <direct.h>
#include <errno.h>
#include <io.h>
#include "../common.h"
#include "handleAllPak.h"

//#define DEBUG

#ifdef DEBUG
#define PAK_NAME_DEBUG
#endif

int main(int argc, char * argv[])
{
	FILE *fpPak,	//所需生成的总文件
	     *fpSrc;	//各个包
#ifdef PAK_NAME_DEBUG
	FILE *fpPakNameTxt;
#endif		
	PakInfo tPakInfo [MAX_FILE_COUNT] = {0};
	char srcName[MAX_FILE_NAME_LENGTH];
	char strOutName[MAX_FILE_NAME_LENGTH] = "stcm.narc";
	char strPakDir [MAX_FILE_NAME_LENGTH] = "./stcm";
	char strPakName [MAX_FILE_COUNT] [ MAX_FILE_NAME_LENGTH ] = {0};
	BYTE *dat;
	BYTE byteTmp;
	struct _finddata_t c_file;
	intptr_t hFile;
	int iCount = 0, c;
	ULONG	ulPakCount = 0,
		ulPakLengthSum,
		ulOutLength;

	while ( -1 != (c = getopt (argc, argv, "i:o:")))
		switch (c)
		{
			case 'i':
				if ('-' == optarg[0])
				{
					printf ("Option -%c requires an argument.\n", c);
					exit (0);
				}
				strcpy (strPakDir, optarg);
				break;
			case 'o':
				if ('-' == optarg[0])
				{
					printf ("Option -%c requires an argument.\n", c);
					exit (0);
				}
				strcpy (strOutName, optarg);
				break;
			case '?':
				if ('i' == optopt || 'o' == optopt)
					;
				else if (optopt == '?')
					;
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr,
							"Unknown option character `\\x%x'.\n",
							optopt);
				fprintf (stdout, "Narc文件打包工具\n");
				fprintf (stdout, "%s [-i 需打包的文件所在的文件夹的名称] [-o 输出文件名]\n", argv[0]);
				return 1;
			default:
				abort ();
		}

	fpPak = fopen (strOutName, "wb+");
	if (fpPak == NULL)
	{
		printf ("Can't create \"%s\" file\n", strOutName);
		return 1;
	}



#ifdef PAK_NAME_DEBUG
	if (NULL == (fpPakNameTxt = fopen ("PakName.txt", "w+")))
	{
		printf ("Can't create \"PakName.txt\" file.\n");
		exit (0);
	}
#endif


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

	if( (hFile = _findfirst( "*.bin", &c_file )) == -1L )
	{
		printf( "No *.bin files in current directory!\n" );
	}
	else
	{
		ulPakCount = 0;
		do
		{
			strcpy (strPakName[ulPakCount], c_file.name);
			tPakInfo [ulPakCount].ulLength = c_file.size;
			tPakInfo [ulPakCount].ulOffsetBegin = (0 != ulPakCount)?
				((4 - (tPakInfo[ulPakCount - 1].ulOffsetEnd % 4)) % 4 + tPakInfo[ulPakCount -1].ulOffsetEnd)
				:0;
			tPakInfo [ulPakCount].ulOffsetEnd = c_file.size + tPakInfo[ulPakCount].ulOffsetBegin;

			ulPakCount ++;
		}
		while( _findnext( hFile, &c_file ) == 0 );
		printf ("There are %d files\n", ulPakCount);
		_findclose( hFile );
	}

	ulPakLengthSum = tPakInfo[ulPakCount - 1].ulOffsetEnd + 0x8 + (4 - (tPakInfo[ulPakCount - 1].ulOffsetEnd % 4)) % 4;
	fileHeadHandle (fpPak, ulPakCount, tPakInfo);	
	fileHeadSecHandle (fpPak, strPakName, strOutName);
	fileHeadTrdHandle (fpPak, ulPakLengthSum);

	//将各个包写入需要打包的narc包中间
	for (iCount = 0; iCount < ulPakCount; iCount ++)
	{
#ifdef PAK_NAME_DEBUG
		fprintf (fpPakNameTxt, "%s\n", strPakName[iCount]);
		fprintf (fpPakNameTxt, "Offset begin on %0Xh.\n", tPakInfo[iCount].ulOffsetBegin);
		fprintf (fpPakNameTxt, "Offset end in %0Xh.\n", tPakInfo[iCount].ulOffsetEnd);
		fprintf (fpPakNameTxt, "Pak length = %0Xh.\n", tPakInfo[iCount].ulLength);
#endif
		fpSrc = fopen (strPakName[iCount], "rb");//需要打包的源文件
		if (fpSrc == NULL)
		{
			printf ("No \"%s\" file.\n", srcName);
			return 2;
		}

		dat = (BYTE *) malloc (sizeof (BYTE) * tPakInfo [iCount].ulLength);
		fread (dat, sizeof(BYTE), tPakInfo [iCount].ulLength, fpSrc);
		fwrite (dat, sizeof(BYTE), tPakInfo [iCount].ulLength, fpPak);
		switch ((tPakInfo[iCount].ulLength) % 4)
		{
			case 1:
				fputc (FILL_BYTE, fpPak);
			case 2:
				fputc (FILL_BYTE, fpPak);
			case 3:
				fputc (FILL_BYTE, fpPak);
			case 0:
				break;
			default:
				;
		}
		free (dat);
		fclose (fpSrc);

	}
#ifdef PAK_NAME_DEBUG
	fclose (fpPakNameTxt);
#endif
	ulOutLength = ftell (fpPak);
	fseek(fpPak, 8, SEEK_SET);
	fwrite (&ulOutLength, sizeof(ULONG), 1, fpPak);
	fclose (fpPak);
	printf("Repak successed.\n");
	return 0;
}

