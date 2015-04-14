// =====================================================================================
//
//       Filename:  PicTrans.h
//
//    Description:  图片格式转换
//
//        Version:  1.0
//        Created:  2015年04月14日 14时46分38秒
//       Revision:  none
//       Compiler:  gcc
//
//         Author:  JiangLutian (JLT), jianglutian@gmail.com
//   Organization:  None
//
// =====================================================================================

#ifndef PIC_TRANS_H
#define PIC_TRANS_H

#include "Common.h"
//#include "BitOperator.h"

// Title的Bit类型
typedef enum
{
    eTB4Bit     = 4,
    eTB8Bit     = 8,
    eTB16Bit    = 16,
    eTB24Bit    = 24,
    eTB32Bit    = 32,
} ETitleBit;

static BOOL ParamParse(int argc, char *argv[], 
        char strData[], char strOutFile[], ETitleBit* pType);
static void DumpHelp (const char* strName);
static void* LoadFile (const char* strData, UINT32* pdwLength);
static BOOL TransData (ETitleBit eType, void* pData, UINT32 dwDataLength, 
        UINT32 dwStartIndex, UINT32 dwCount);
static BOOL WriteFile (const char* strOutFile, void* pData, UINT32 dwLength);

#endif // #ifndef PIC_TRANS_H
