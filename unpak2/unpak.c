#include "../common.h"

#include "getPakInfo.c"

int main(int argc, char *argv[])
{
	FILE *fpPak,	//源文件
		 *fpOut,	//输出的各个包的独立文件
		 *fpTxt;	//拆包后索引用文件

	ULONG	i,
			pakCount,							//储存文件包的个数
			base[MAX_FILE_COUNT] = {0},			//各个文件在源文件中的基址
			size[MAX_FILE_COUNT] = {0},			//包的大小
			zipedSize[MAX_FILE_COUNT] = {0},	//压缩后包的大小
			zipFlag[MAX_FILE_COUNT] = {0},		//是否压缩的标志位
			pakLen,								//储存文件包的长度
			zipCount = 0;						//压缩包的个数

	char fileName[MAX_FILE_NAME_LENGTH],		//打开文件的文件名
		 idx[40],
		 *buffer;		

	BYTE *dat;
	

	//没有输入源文件名
	if (argc < 2)
	{
		printf ("No source file fileName input\n");
		return 1;
	}

	//打开源文件

	strcpy (fileName, argv[1]);
	if (strstr (fileName, ".PAK") == NULL)
	{
		strcat (fileName, ".PAK");
	}	


	fpPak = fopen (fileName, "rb");

	if (fpPak == NULL)
	{
		printf ("file \"%s\" did not exist.\n", fileName);
		return 2;
	}
	else
	{
		#ifdef DEBUG
			printf ("file \"%s\" opened.\n", fileName);
		#endif
	}

	//获得包的数目
	pakCount = getPakCount (fpPak);

	//创建目标文件
	strcpy (fileName, argv[1]);
	if ( (buffer = strstr (fileName, ".PAK")) != NULL )
	{
		strcpy (buffer ,"");
	}
	strcat (fileName, ".idx");
	fpTxt = fopen (fileName, "w");
	if ( fpTxt == NULL )
	{
		printf ("Can't create %s file.\n", fileName);
		return 3;
	}
	printf ("%s flie created.\n", fileName);

	//获取文件偏移地址,并生成有关各个包索引信息的文件（包的数目、大小和是否压缩）
	handlePakInfo (fpPak, pakCount, base, zipFlag, fpTxt, argv[1]);

	fclose (fpTxt);

	for(i = 0; i < pakCount; i++)//逐个输出文件
	{
		fseek (fpPak, base[i], 0);
		pakLen = base[i+1] - base[i];

		fileName[0] = '_';
		fileName[1] = 0;
		
		strcat (fileName, argv[1]);

		//如果zipFlag位有标志，则后缀为zip表示为zip压缩包
		if(zipFlag[i])
			sprintf (idx, "_%02d.zip.PAK", i);
		else 
			sprintf (idx, "_%02d.PAK", i);
		strcat (fileName, idx);
		fpOut = fopen (fileName, "wb");		//输出文件
		
		dat = (BYTE *) malloc ( (sizeof (BYTE)) * pakLen);

		fread (dat, sizeof (BYTE), pakLen, fpPak);
		fwrite (dat, sizeof (BYTE), pakLen, fpOut);
		free (dat);
		fclose (fpOut);
	}

	fclose (fpPak);

	return 0;
}
