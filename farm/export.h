#include <stdio.h>

#include "../common.h"


//文本区间链表
typedef struct txtExtent
{
	struct txtExtent * next;
	ULONG start, end;
	int iWidth;
} txtExt;
