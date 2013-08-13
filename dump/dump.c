#include "dump.h"

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

