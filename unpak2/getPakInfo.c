#include "../common.h"

ULONG getPakCount (FILE *fpPak);		//获得包的数目

int	handlePakInfo (FILE *fpPak, ULONG pakCount, 
					ULONG base[], ULONG zipFlag[], 
					FILE *fpTxt, char fileName []);		//获取包的信息并输出到索引文件里面

void putPakSize (FILE *fpPak, FILE *fpTxt);				//在索引文件里面输出包的大小

int getPakZipFlag (FILE *fpPak, ULONG *zipFlag);		//获取包压缩位的标志


//获得包的数目
ULONG getPakCount (FILE *fpPak)
{
	ULONG count = 0;
		//源文件的第一个字节为内部文件的个数

	fread (&count, sizeof(ULONG), 1, fpPak);	//文件个数
	printf ("%lX documents founded.\n", count);
	fseek (fpPak, 0x10, SEEK_SET);
	
	//如果读取到的文件数目大于MAX_FILE_COUNT个，则表示错误，退出程序
	if(count > MAX_FILE_COUNT)
		{
			printf ("error num!\n");
			exit (0);
		}
	return count;
}

//获取包的偏移地址
inline int getPakAddr (FILE *fpPak, ULONG *baseAddr)
{
	fread (baseAddr, sizeof (ULONG), 1, fpPak);
	return 0;
}

int	handlePakInfo (FILE *fpPak, ULONG pakCount, ULONG base[], ULONG zipFlag[], 
				FILE *fpTxt, char fileName[])
{
	ULONG	i,
			zipCount = 0;
	char	tempName[MAX_FILE_NAME_LENGTH];
	for (i = 0; i < pakCount; i ++)
	{
		getPakAddr (fpPak, &base[i]);
		putPakSize (fpPak, fpTxt);
		zipCount += getPakZipFlag (fpPak, &zipFlag[i]);
		
		tempName[0] = '_';
		tempName[1] = 0;
		
		strcat (tempName, fileName);
		fprintf (fpTxt, "%s", tempName);

		//如果zipFlag位有标志，则后缀为zip表示为zip压缩包
		if(zipFlag[i])
			fprintf (fpTxt, "_%02d.zip.PAK\n\n", i);
		else 
			fprintf (fpTxt, "_%02d.PAK\n\n", i);
	}
	fseek (fpPak, 0, SEEK_END);
	base [i] = ftell (fpPak) + 1;
	fprintf (fpTxt, "共有%lu个包，其中里面有%lu个压缩包\n", pakCount, zipCount);
	return 0;
}

void putPakSize (FILE *fpPak, FILE *fpTxt)
{
	ULONG size, zipedSize;
	fread (&size, sizeof (ULONG), 1, fpPak);
	fprintf (fpTxt, "原大小为%10lu", size);
	fread (&zipedSize, sizeof (ULONG), 1, fpPak);
	if (size != zipedSize)
	{
		fprintf (fpTxt, "，压缩后大小为%10lu。\n", zipedSize);
	}
	else
	{
		fprintf (fpTxt, "。\n");
	}

}

int getPakZipFlag (FILE *fpPak, ULONG *zipFlag)
{
	ULONG flag;
	fread (&flag, sizeof (ULONG), 1, fpPak);
	return *zipFlag = !(flag & 0x80000000);

}
