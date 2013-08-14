#include "dump.h"
#include <stdarg.h>

FILE *g_fpTRACE = NULL;
FILE *g_fpDEBUG = NULL;
FILE *g_fpERROR = NULL;

BOOL logDumpInit()
{
	BOOL bRet = TRUE;

	g_fpTRACE = fopen ("TRACE.txt", "wb");
	if (g_fpTRACE == NULL)
	{
		printf ("Can't create \"TRACE.txt\" file.\n");
		bRet = FALSE;
	}

	g_fpDEBUG = fopen ("DEBUG.txt", "wb");
	if (g_fpDEBUG == NULL)
	{
		printf ("Can't create \"DEBUG.txt\" file.\n");
		bRet = FALSE;
	}

	g_fpERROR = fopen ("ERROR.txt", "wb");
	if (g_fpERROR == NULL)
	{
		printf ("Can't create \"ERROR.txt\" file.\n");
		bRet = FALSE;
	}

	return bRet;
}

void dumpHexData (FILE *fpStream, BYTE *pbyteBuf, long lLenght)
{
	long lCount;
	for (lCount = 0; lCount < lLenght; ++ lCount)
	{
		fprintf (fpStream, "%02Xh ", pbyteBuf[lCount]);
		if (15 == (lCount % 16))
		{
			fprintf (fpStream, "\r\n");
		}			
	}
	if (0 != (lCount % 16))
		fprintf (fpStream, "\r\n");
}

void logDumpInfo(FILE *fpStream, const char *format, ...)
{
	if (!fpStream)
		return;
	va_list ap;
	va_start(ap, format);
	vfprintf(fpStream, format, ap);
	va_end(ap);
	fprintf(fpStream, "\r\n");
}
