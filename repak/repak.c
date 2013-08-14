/*
 *	打包工具
 *
 *	参数为需要生成的包名文件，格式为 *.PAK
 *	需要打包的包名要求格式为	_*.PAK_$$.PAK		($$代表小包的序号，比如84)
 *						或者	_*.PAK_$$.zip.PAK	($$代表小包的序号，比如84)
 *
*/

#include "../common.h"
#include "handleAllPak.c"

inline void getSrcName_i (char *srcName, char *outName, ULONG i);
inline void getZipSrcName (char *srcName, char *outName, ULONG i);

int main(int argc, char * argv[])
{
	FILE *fpPak,	//所需生成的总文件
		 *fpSrc;	//各个包
	ULONG		  num,
				  fileLength[PAK_COUNT],	//储存PAK_COUNT个文件包的长度
				  len,
				  i;
	char srcName[MAX_FILE_NAME_LENGTH];
	char outName[MAX_FILE_NAME_LENGTH] = "SPR_NCLR.PAK";
	BYTE *dat;
	BYTE c;

	#ifdef DEBUG
		printf ("sizeof(unsigned long) = %d\n", sizeof (unsigned long));
	#endif

	fpPak = fopen (outName, "wb");
	if (fpPak == NULL)
	{
		printf ("Can't create \"%s\" file\n", outName);
		return 1;
	}

	#ifdef DEBUG
		printf ("File \"%s\" created.\n", outName);
	#endif

	fileHeadHandle (fpPak);	


	for(i = 0; i < PAK_COUNT; i++)//逐个输出文件
	{

		getSrcName_i (srcName, outName, i);


		fpSrc = fopen (srcName, "rb");//需要打包的源文件
		if (fpSrc == NULL)
		{
			getZipSrcName (srcName, outName, i);
			fpSrc = fopen (srcName, "rb");
			if (fpSrc == NULL)
			{
				printf ("No \"%s\" file.\n", srcName);
				return 2;
			}

		}

		#ifdef DEBUG
			printf ("file \"%s\" is being packing\n", srcName);
		#endif

		//获取该包的长度
		if ((fileLength [i] = getFileLength (fpSrc)) < 0)
		{
			printf ("Wrong file length\n");
			return 4;
		}
		

		dat = (BYTE *) malloc (sizeof (BYTE) * fileLength [i]);

		fseek (fpSrc, 0, SEEK_SET);
		fread (dat, sizeof(BYTE), fileLength[i], fpSrc);
		fwrite (dat, sizeof(BYTE), fileLength[i], fpPak);
		free (dat);
		fclose (fpSrc);
	}

	writeFileLength (fpPak, fileLength, PAK_COUNT);
	fclose(fpPak);

	return 0;
}

inline void getSrcName_i (char *srcName, char *outName, ULONG i)
{
	sprintf (srcName, "_%s_%d.PAK", outName, i);
}

inline void getZipSrcName (char *srcName, char *outName, ULONG i)
{
		sprintf (srcName, "_%s_%d.zip.PAK", outName, i);
}
