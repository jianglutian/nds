#ifndef DUMP_H
#define DUMP_H

#include "Common.h"

extern FILE *g_fpTRACE;
extern FILE *g_fpDEBUG;
extern FILE *g_fpERROR;

BOOL LogDumpInit();
void DumpHexData (FILE *fpStream, BYTE *pbyteBuf, long lLenght);
void LogDumpInfo(FILE *fpStream, const char *format, ...);

#define TRACE(format, ...) LogDumpInfo(g_fpTRACE, format, ##__VA_ARGS__);
#define DEBUG(format, ...) LogDumpInfo(g_fpDEBUG, format, ##__VA_ARGS__);
#define ERROR(format, ...) LogDumpInfo(g_fpERROR, format, ##__VA_ARGS__);


#endif // #ifndef DUMP_H
