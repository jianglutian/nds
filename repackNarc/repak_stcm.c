/*
 *	打包工具
 *
*/
#include <direct.h>
#include <errno.h>
#include <io.h>
#include "../common.h"
#include "handleAllPak.h"

#define DEBUG

int main(int argc, char * argv[])
{
	FILE *fpPak,	//所需生成的总文件
		 *fpSrc;	//各个包
	ULONG		num,
				ulPakLength[MAX_FILE_COUNT],	//储存PAK_COUNT个文件包的长度
				ulPakOffset[MAX_FILE_COUNT];	//储存PAK_COUNT个文件包在生成的包里面的绝对偏移地址
												//通常是文件头长度加上ulPakLength的对齐版本
	char srcName[MAX_FILE_NAME_LENGTH];
	char outName[MAX_FILE_NAME_LENGTH] = "stcm.narc";
	char strPakDir [MAX_FILE_NAME_LENGTH] = "./pak";
	char strPakName [MAX_FILE_COUNT] [ MAX_FILE_NAME_LENGTH ] = {0};
	BYTE *dat;
	BYTE byteTmp;
	struct _finddata_t c_file;
	intptr_t hFile;
	int count = 0, i;

	fpPak = fopen (outName, "wb+");
	if (fpPak == NULL)
	{
		printf ("Can't create \"%s\" file\n", outName);
		return 1;
	}

	#ifdef DEBUG
		printf ("File \"%s\" created.\n", outName);
	#endif

	fileHeadHandle (fpPak);	
	if (argc > 1)
	{
		sprintf (strPakDir,"./%s", argv [1]);
	}
#ifdef DEBUG
	printf ("Now move to the \"%s\" directory.\n", strPakDir);
#endif


	//读取文件夹里面所有的包

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
		ulPakLength [0] = 0;
		do
		{

			strcpy (strPakName[count], c_file.name);
			#ifdef DEBUG
				printf ("%s\n", strPakName[count]);
				printf ("length:%d\n", strPakName[count]);
			#endif

			fpSrc = fopen (strPakName[count], "rb");//需要打包的源文件
			if (fpSrc == NULL)
			{
				printf ("No \"%s\" file.\n", srcName);
				return 2;
			}
			ulPakLength [count * 2 + 1] = c_file.size + ulPakLength [count * 2];
			ulPakLength [count * 2 + 2] = (4 - c_file.size % 4) % 4 + ulPakLength [count * 2 + 1];
	
			
			dat = (BYTE *) malloc (sizeof (BYTE) * c_file.size);

			fread (dat, sizeof(BYTE), c_file.size, fpSrc);
			fwrite (dat, sizeof(BYTE), c_file.size, fpPak);
			switch ( c_file.size % 4)
			{
			case 1:
				fputc (FILL_BYTE, fpPak);
			case 2:
				fputc (FILL_BYTE, fpPak);
			case 3:
				fputc (FILL_BYTE, fpPak);
			case 0:
				break;
			}
			free (dat);
			fclose (fpSrc);
			count ++;

		}
		while( _findnext( hFile, &c_file ) == 0 );
		printf ("There are %d files\n", count);
		_findclose( hFile );

	}

	fseek (fpPak, HEAD_LENGTH , SEEK_SET);
	fwrite (&count, sizeof(ULONG), 1, fpPak);
	writeFileLength (fpPak, ulPakLength, count * 2);
	fileHeadSecHandle (fpPak);
	writeFileName (fpPak, strPakName);
	byteTmp = 0;
	fwrite (&byteTmp, sizeof(BYTE), 1, fpPak);
	fileHeadTrdHandle (fpPak);


	fclose (fpPak);
	return 0;
}

