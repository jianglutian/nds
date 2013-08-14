#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_FILE_COUNT 99
int main(int argc, char *argv[])
{
	FILE *pak,	//源文件
		 *out,	//输出的各个包的独立文件
		 *txt;	//目标文件

	unsigned long num,				//储存文件包的个数
				  base[100] = {0},	//各个文件在源文件中的基址
				  flag[100] = {0},	//是否LZ压缩的标志位
				  len,				//储存文件包的长度
				  i,
				  lzCount = 0;		//LZ压缩包的个数
	char name[40],		//打开文件的文件名
		 idx[40],
		 *buffer;		

	unsigned char *dat;
	

	//没有输入源文件名
	if (argc < 2)
	{
		printf ("No source file name input\n");
		return 1;
	}

	//打开源文件

	strcpy (name, argv[1]);
	if (strstr (name, ".bin") == NULL)
	{
		strcat (name, ".bin");
	}	

	pak = fopen (name, "rb");

	if (pak == NULL)
	{
		printf ("file \"%s\" did not exist.\n", name);
		return 2;
	}



	//源文件的第一个字节为内部文件的个数
	fread (&num, 4, 1, pak);//文件个数
	printf ("%d documents founded.\n", num);
	
	//如果读取到的文件数目大于MAX_FILE_COUNT个，则表示错误，退出程序
	if(num > MAX_FILE_COUNT)
		{
			printf ("error num!\n");
			return 4;
		}

	//创建目标文件
	strcpy (name, argv[1]);
	if ( (buffer = strstr (name, ".bin")) != NULL )
	{
		strcpy (buffer ,"");
	}
	strcat (name, ".idx");
	txt = fopen (name, "wb");
	if ( txt == NULL )
	{
		printf ("Can't create %s file.\n", name);
		return 3;
	}

	printf ("%s flie created.\n", name);

	fprintf (txt, "0x%06X\r\n", num);

	//将num个文件的基址存入base数组内，是否有LZ压缩标志放入flag内
	for (i = 0; i < num + 1; i++)
	{
		fread (&base[i], 4, 1, pak);
		if(base[i] & 0x80000000)
		{
			lzCount++;
			flag[i] = 1;
			base[i] -= 0x80000000;
		}
	}

	printf ("%d LZ files found\n", lzCount);
	//文件总长度==base[num]

	for(i = 0; i < num; i++)//逐个输出文件
	{
		fseek (pak, base[i], 0);
		len = base[i+1] - base[i];

		name[0] = '_';
		name[1] = 0;
		
		strcat (name, argv[1]);

		//如果flag位有标志，则后缀为LZ表示为LZ压缩包
		if(flag[i])
			sprintf (idx, "_%02d.lz", i);
		else 
			sprintf (idx, "_%02d.bin", i);
		strcat (name, idx);

		fprintf (txt, "%s\r\n", name);
		fprintf (txt, "%d\r\n", flag[i]);
		out = fopen (name, "wb");		//输出文件
		
		dat = (unsigned char *) malloc ( (sizeof (char)) * len);

		fread (dat, 1, len, pak);
		fwrite (dat, 1, len, out);
		free (dat);
		fclose (out);
	}
	fclose (txt);
	fclose (pak);

	return 0;
}
