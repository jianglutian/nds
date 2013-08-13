#include <stdio.h>

#include <common.h>
#include <ctype.h>
#include "dump.h"


#define CODING_LENGTH	(0xffff)

WORD pwCtrlCodeIndex[] = {0xFC, 0xFD};
#define CTRL_COUNT (sizeof (pwCtrlCodeIndex) / sizeof (pwCtrlCodeIndex[0]))
//文本区间链表
typedef struct txtExtent
{
	struct txtExtent * next;
	ULONG start, end;
	int iWidth;
} txtExt;

