// =====================================================================================
//
//       Filename:  Dump.c
//
//    Description:  各种调试输出
//
//        Version:  1.0
//        Created:  2015年02月27日 14时44分57秒
//       Revision:  none
//       Compiler:  gcc
//
//         Author:  JiangLutian (JLT), jianglutian@gmail.com
//   Organization:  None
//
// =====================================================================================

#include "Dump.h"

#include <stdarg.h>

FILE *g_fpTRACE = NULL;
FILE *g_fpDEBUG = NULL;
FILE *g_fpERROR = NULL;

// ===  FUNCTION  ======================================================================
//         Name:  LogDumpInit
//  Description:  初始化输出日志
// =====================================================================================
BOOL LogDumpInit()
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
}		// -----  end of function LogDumpInit  ----- //

// ===  FUNCTION  ======================================================================
//         Name:  DumpHexData
//  Description:  输出十六进制字符串
// =====================================================================================
void DumpHexData (FILE *fpStream, BYTE *pbyteBuf, long lLenght)
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
}		// -----  end of function DumpHexData  ----- //

// ===  FUNCTION  ======================================================================
//         Name:  LogDumpInfo
//  Description:  输出格式化的字符串
// =====================================================================================
void LogDumpInfo(FILE *fpStream, const char *format, ...)
{
	if (!fpStream)
		return;
	va_list ap;
	va_start(ap, format);
	vfprintf(fpStream, format, ap);
	va_end(ap);
	fprintf(fpStream, "\r\n");
}		// -----  end of function LogDumpInfo  ----- //
