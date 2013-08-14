#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>

//#define DEBUG
#define MAX_FILE_NAME_LENGTH 127	//文件名的最大长度
#define HEAD_LENGTH			 8		//文件头名称的长度
#define PAK_COUNT			 31		//需打包的文件的个数

#define PAK_START_OFFSET	(0x90)	//第一个包开始的偏移位置

typedef unsigned long ULONG;	//用于储存4个字节的数据

unsigned char headData[] = 
	{ 0x46, 0x42, 0x49, 0x4E,	//文件格式名NIBF
	  0x1E, 0x00, 0x00, 0x00	//文件内有30个有效文件（包含文件头实际有31个）
	};

void fileHeadHandle (FILE *outFile);
long getFileLength (FILE *fp);
void writeFileLength (FILE *fp, ULONG *lengthTab, int count);

int main(int argc, char * argv[])
{
	FILE *pak,
		 *src;
	ULONG		  num,
				  fileLength[PAK_COUNT],	//储存PAK_COUNT个文件包的长度
				  len,
				  i;
	char srcName[MAX_FILE_NAME_LENGTH];
	char outName[MAX_FILE_NAME_LENGTH] = "EventMessageDungeonGimmic.bin";
	unsigned char *dat;
	unsigned char c;

	#ifdef DEBUG
		printf ("sizeof(unsigned long) = %d\n", sizeof (unsigned long));
	#endif

	pak = fopen (outName, "wb");
	if (pak == NULL)
	{
		printf ("Can't create \"%s\" file\n", outName);
		return 1;
	}

	#ifdef DEBUG
		printf ("File \"%s\" created.\n", outName);
	#endif

	fileHeadHandle (pak);	


	for(i = 1; i < PAK_COUNT; i++)//逐个输出文件
	{
		sprintf (srcName, "%s%d", outName, i + 1);


		src = fopen (srcName, "rb");//需要打包的源文件
		if (src == NULL)
		{
			printf ("No \"%s\" file.\n", srcName);
			return 2;
		}

		#ifdef DEBUG
			printf ("file \"%s\" is being packing\n", srcName);
		#endif

		//获取该包的长度
		if ((fileLength [i] = getFileLength (src)) < 0)
		{
			printf ("Wrong file length\n");
			return 4;
		}
		

		dat = (unsigned char *) malloc (sizeof (unsigned char) * fileLength [i]);

		fseek (src, 0, SEEK_SET);
		fread (dat, sizeof(unsigned char), fileLength[i], src);
		fwrite (dat, sizeof(unsigned char), fileLength[i], pak);
		free (dat);
		fclose (src);
	}

	writeFileLength (pak, fileLength, PAK_COUNT);
	fclose(pak);

	return 0;
}

//文件头的写入，各个包的长度预置为0
//函数结束时FILE指针指向文件的末尾
void fileHeadHandle (FILE *outFile)
{
	int i;
	ULONG zeroField[PAK_COUNT + 3] = {0};


	#ifdef DEBUG
		printf ("Start insert head data\n");
	#endif

	fwrite ( headData, sizeof (unsigned char), HEAD_LENGTH, outFile);

	#ifdef DEBUG
		printf ("Head data inserted\n");
	#endif

	
	fwrite ( zeroField, sizeof (ULONG), PAK_COUNT + 3, outFile);
	
	#ifdef DEBUG
		printf ("Now we're in the offset of %lx\n", ftell (outFile));
	#endif
		
}

//获取包的长度
//不改变参数中文件指针的地址
long getFileLength (FILE *fp)
{
	long orgAddr,
		 fileLength;
	orgAddr = ftell (fp);
	if ( (fseek (fp, 0, SEEK_END)) )
	{
		printf ("Error when caculating file length.\n");
		return -1;
	}
	fileLength = ftell(fp);

	fseek (fp, orgAddr, SEEK_CUR);

	#ifdef DEBUG
		printf ("the file lentgh is %ld\n", fileLength);
	#endif

	return fileLength;
}

void writeFileLength (FILE *fp, ULONG *lengthTable, int count)
{
	int i;
	fseek (fp, HEAD_LENGTH, SEEK_SET);

	fwrite (lengthTable, sizeof (ULONG), count, fp);
}

