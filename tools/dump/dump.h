#ifndef DUMP_H
#define DUMP_H

#include "common.h"

extern FILE *g_fpTRACE;
extern FILE *g_fpDEBUG;
extern FILE *g_fpERROR;

BOOL logDumpInit();
void dumpHexData (FILE *fpStream, BYTE *pbyteBuf, long lLenght);
void logDumpInfo(FILE *fpStream, const char *format, ...);

#define TRACE(format, ...) logDumpInfo(g_fpTRACE, format, ##__VA_ARGS__);
#define DEBUG(format, ...) logDumpInfo(g_fpDEBUG, format, ##__VA_ARGS__);
#define ERROR(format, ...) logDumpInfo(g_fpERROR, format, ##__VA_ARGS__);


#endif
