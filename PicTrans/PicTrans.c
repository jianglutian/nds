// =====================================================================================
//
//       Filename:  PicTrans.c
//
//    Description:  图片格式转换
//
//        Version:  1.0
//        Created:  2015年04月14日 14时36分14秒
//       Revision:  none
//       Compiler:  gcc
//
//         Author:  JiangLutian (JLT), jianglutian@gmail.com
//   Organization:  None
//
// =====================================================================================

#include "PicTrans.h"

#include <stdlib.h>
#include <ctype.h>

#include "Dump.h"

UINT8 szTransMap[] =
{
    0,  4,  1,  5,
    8,  12, 9,  13,
    2,  6,  3,  7,
    10, 14, 11, 15
};

UINT8 byMapSize = sizeof(szTransMap)/sizeof(UINT8);

// ===  FUNCTION  ======================================================================
//         Name:  main
//  Description:  程序主函数
// =====================================================================================
int main ( int argc, char *argv[] )
{
    BOOL bRet = FALSE;          // 正确值

	// 数据的文件名
	char strData [MAX_FILE_NAME_LENGTH] = "data.bin";

	// 码表的文件名
	char strOutFile [MAX_FILE_NAME_LENGTH] = "out.bin";	

    ETitleBit eType = 0;        // 像素Bit数类型

    UINT32 dwDataLength = 0;    // 数据长度
    void* pData = NULL;         // 数据

    UINT32 dwStartIndex = 0;    // 目标数据起始偏移量
    UINT32 dwCount = 1;         // 目标数据个数

#ifdef DEBUG
	FILE	* fpTest = NULL;   // 调试输出用文件
#endif

#ifdef DEBUG
	fpTest = fopen ("Log.txt", "wt");
    LogDumpInit();
#endif

    bRet = ParamParse(argc, argv, strData, strOutFile, &eType, &dwStartIndex, &dwCount);
    if (!bRet)
        return EXIT_FAILURE;

    pData = LoadFile(strData, &dwDataLength);

    bRet = TransData(eType, pData, dwDataLength, dwStartIndex, dwCount);
    if (!bRet)
        return EXIT_FAILURE;

    bRet = WriteFile(strOutFile, pData, dwDataLength);
    if (!bRet)
        return EXIT_FAILURE;

    if (pData)
    {
        free(pData);
        pData = NULL;
    }

#ifdef DEBUG
	fclose (fpTest);
#endif

    return EXIT_SUCCESS;
}				// ----------  end of function main  ---------- //

// ===  FUNCTION  ======================================================================
//         Name:  ParamParse
//  Description:  参数解析
// =====================================================================================
BOOL ParamParse(int argc, char *argv[], 
        char strData[], char strOutFile[], 
        ETitleBit* pType, UINT32* pStartIndex, UINT32* pCount)
{
	int	iMenuParam = 0;	// 程序调用参数解析用

    // =====================================================================================
	// 对命令行的参数进行解析
	while ((iMenuParam = getopt (argc, argv, "d:o:b:h:s:c:")) != -1)
	{
		switch (iMenuParam)
		{
			case 'd': // 指定数据文件
				if ('-' == optarg[0])
				{
					fprintf(stderr, "Option -%c requires an argument.\n", iMenuParam);
                    return FALSE;
				}
                strcpy(strData, optarg);
				break;
			case 'o': // 指定输出文件
				if ('-' == optarg[0])
				{
					fprintf(stderr, "Option -%c requires an argument.\n", iMenuParam);
                    return FALSE;
				}
                strcpy(strOutFile, optarg);
				break;
			case 'b': // 指定数据格式
				if ('-' == optarg[0])
				{
					fprintf(stderr, "Option -%c requires an argument.\n", iMenuParam);
                    return FALSE;
				}
                *pType = (ETitleBit) atoi(optarg);
				break;
            case 'c':
				if ('-' == optarg[0])
				{
					fprintf(stderr, "Option -%c requires an argument.\n", iMenuParam);
                    return FALSE;
				}
                *pCount = atoi(optarg);
				break;
            case 's':
				if ('-' == optarg[0])
				{
					fprintf(stderr, "Option -%c requires an argument.\n", iMenuParam);
                    return FALSE;
				}
                *pStartIndex = atoi(optarg);
				break;
			case '?':
			default:
				if ('d' == optopt || 'o' == optopt || 'b' == optopt)
					;
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n",	optopt);
                DumpHelp(argv[0]);
                return FALSE;
		}
	}
	// 解析结束
    // =====================================================================================
    return TRUE;
}		// -----  end of function ParamParse  ----- //

// ===  FUNCTION  ======================================================================
//         Name:  DumpHelp
//  Description:  输出帮助文件
// =====================================================================================
void DumpHelp (const char* strName)
{
    fprintf(stderr, "图片转换:\n");
    fprintf(stderr, "%s [-d 数据文件] [-b 数据格式（4/8/16/24/32）] [-o 生成文件] [-s 起始偏移量] [-c 数据个数]\n", strName);
}		// -----  end of function DumpHelp  ----- //

// ===  FUNCTION  ======================================================================
//         Name:  LoadFile
//  Description:  读取数据文件到内存中
// =====================================================================================
void* LoadFile (const char* strData, UINT32* pdwLength)
{
	FILE* fpData    = NULL;    // 数据文件
    void* pData = NULL;

    *pdwLength = 0;

	// 读取数据文件
	if ( (fpData = fopen (strData, "rb")) == NULL )
	{
		fprintf(stderr, "Error when loading data \"%s\"\n", strData);
        return pData;
	}
	// 复制到内存中
	fseek (fpData, 0, SEEK_END);
	*pdwLength = sizeof (UINT8) * ftell (fpData);
	pData = malloc (*pdwLength);
	fseek (fpData, 0, SEEK_SET);
	fread (pData, sizeof (UINT8), *pdwLength, fpData);
	fseek (fpData, 0, SEEK_SET);

    if (fpData)
    {
        fclose(fpData);
        fpData = NULL;
    }
    return pData;
}		// -----  end of function LoadFile  ----- //

// ===  FUNCTION  ======================================================================
//         Name:  TransData
//  Description:  数据转换
// =====================================================================================
BOOL TransData (ETitleBit eType, void* pData, UINT32 dwDataLength, 
        UINT32 dwStartIndex, UINT32 dwCount)
{
    BOOL bRet = FALSE;
    UINT8 byPixel = 0;
    void* pTmpData = NULL;
    UINT32 dwIndex = 0;
    UINT32 dwBlockSize = 0;
    UINT32 dwTmpCount = 0;

    if (!pData)
        return bRet;
    switch (eType)
    {
        case eTB4Bit:
        case eTB8Bit:
        case eTB16Bit:
        case eTB24Bit:
        case eTB32Bit:
            byPixel = (UINT8)eType * 2 / 8;
            break;
        default:
            fprintf(stderr, "Invalid ETitleBit: %d\n", eType);
            return bRet;
            break;
    }

    dwBlockSize = byPixel * 16;

    if (!dwCount)
        dwCount = (dwDataLength - dwStartIndex) / dwBlockSize;

    if (dwDataLength < dwStartIndex + dwCount * dwBlockSize)
        return bRet;

    for (dwTmpCount = 0; dwTmpCount < dwCount; ++dwTmpCount)
    {
        pTmpData = malloc(byPixel * 16);
        if (!pTmpData)
            return bRet;
        memcpy(pTmpData, pData + dwStartIndex + dwTmpCount * dwBlockSize, byPixel * 16);
        for(dwIndex = 0; dwIndex < byMapSize; ++dwIndex)
        {
            memcpy(pData + dwStartIndex + dwTmpCount * dwBlockSize + szTransMap[dwIndex] * byPixel, pTmpData + dwIndex * byPixel, byPixel);
        }
#ifdef DEBUG
        fprintf(stderr, "%d/%d:\n", dwTmpCount, dwCount);
        fprintf(stderr, "SrcData:\n");
        DumpHexData(stderr, pTmpData, byPixel * 16);
        fprintf(stderr, "DstData:\n");
        DumpHexData(stderr, pData + dwStartIndex, byPixel * 16);
        fprintf(stderr, "\n");
#endif
    }


    free(pTmpData);

    bRet = TRUE;
    return bRet;
}		// -----  end of function TransData  ----- //

// ===  FUNCTION  ======================================================================
//         Name:  WriteFile
//  Description:  生成指定的文件
// =====================================================================================
BOOL WriteFile (const char* strOutFile, void* pData, UINT32 dwLength)
{
	FILE* fpOutFile = NULL;    // 输出文件

    BOOL bRet = FALSE;

    if (!pData)
        return bRet;

    // 创建输出文件
	if ( (fpOutFile = fopen (strOutFile, "wb+")) == NULL )
	{
		fprintf(stderr, "Error when loading data \"%s\"\n", strOutFile);
        return bRet;
	}
    if (pData)
        fwrite(pData, sizeof (UINT8), dwLength, fpOutFile);

    if (fpOutFile)
    {
        fclose(fpOutFile);
        fpOutFile = NULL;
    }

    bRet = TRUE;
    return bRet;
}		// -----  end of function WriteFile  ----- //
