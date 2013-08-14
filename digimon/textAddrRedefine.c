#include <stdio.h>
#include <stdlib.h>

//#define DEBUG

#define MAX_CHAR	256
#define OFFSET		-0xfc	 

void convert (wchar_t * src, int offset);

int main(int argc, char *argv[])
{
	FILE *srcFile, *dstFile;
	char srcFileLabel[MAX_CHAR], dstFileLabel[MAX_CHAR];
	wchar_t buffer[MAX_CHAR];

	#ifndef DEBUG
		//没有输入源文件名
		if (argc < 2)
		{
			printf ("No source file name input\n");
			return 1;
		}
	
		//打开源文件
		strcpy (srcFileLabel, argv[1]);
		strcpy (dstFileLabel, srcFileLabel);
		strcat (dstFileLabel, ".end.txt");
	#endif

	#ifdef DEBUG
		strcpy (srcFileLabel, "arm9-000EF6D0-000EF71F");
		strcpy (dstFileLabel, srcFileLabel);
		strcat (srcFileLabel, ".txt");
		strcat (dstFileLabel, ".end.txt");
	#endif

	if (NULL == (srcFile = fopen (srcFileLabel, "rb")))
	{
		printf ("Unable to open %f file.\n", srcFileLabel);
		return 1;
	}

	if (NULL == (dstFile = fopen (dstFileLabel, "wb+")))
	{
		printf ("Unable to create %f file.\n", dstFileLabel);
		return 2;
	}

	while ( fgetws (buffer, MAX_CHAR, srcFile) != NULL)
	{
		if ( wcsstr (buffer, L"No.") != NULL)
		{
			fputws (buffer, dstFile);
			_putws (buffer);
			#ifdef DEBUG
				printf ("find it\n");
			#endif
			if (fgetws (buffer, MAX_CHAR, srcFile) != NULL)
			{
				convert (buffer, OFFSET);
			}
		}		
		fputws (buffer, dstFile);
		_putws (buffer);
	}

	fclose (srcFile);
	fclose (dstFile);

	return 0;
}

void convert (wchar_t *src, int offset)
{
	long num;
	wchar_t temp[MAX_CHAR], *temp2;

	
	num = wcstol (src, &temp2, 16);
	wcscpy (temp, src);
	
	#ifdef DEBUG
		printf ("%08lx\n",num);
	#endif

	num += offset;
	swprintf (src, L"%08lX%s", num, temp + 8);

	#ifdef DEBUG
		_putws (src);
	#endif
}
