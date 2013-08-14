#include "../common.h"

#define HEAD_LENGTH			 8		//文件头名称的长度
#define PAK_COUNT			 31		//需打包的文件的个数

#define PAK_START_OFFSET	(0x90)	//第一个包开始的偏移位置


BYTE headData[] = 
	{ 0x46, 0x42, 0x49, 0x4E,	//文件格式名NIBF
	  0x1E, 0x00, 0x00, 0x00	//文件内有30个有效文件（包含文件头实际有31个）
	};



void fileHeadHandle (FILE *outFile);
ULONG getFileLength (FILE *fp);
inline void writeFileLength (FILE *fp, ULONG *lengthTab, int count);



//文件头的写入，各个包的长度预置为0
//函数结束时FILE指针指向第一个需写入包的偏移地址
void fileHeadHandle (FILE *outFile)
{
	int i;
	ULONG zeroField[PAK_COUNT + 3] = {0};


	#ifdef DEBUG
		printf ("Start insert head data\n");
	#endif

	fwrite ( headData, sizeof (BYTE), HEAD_LENGTH, outFile);

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
ULONG getFileLength (FILE *fp)
{
	ULONG orgAddr,
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

inline void writeFileLength (FILE *fp, ULONG *lengthTable, int count)
{
	int i;
	fseek (fp, HEAD_LENGTH, SEEK_SET);

	fwrite (lengthTable, sizeof (ULONG), count, fp);
}
