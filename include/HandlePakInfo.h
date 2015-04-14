// =====================================================================================
//
//       Filename:  HandlePakInfo.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2015年02月25日 15时27分06秒
//       Revision:  none
//       Compiler:  gcc
//
//         Author:  JiangLutian (JLT), jianglutian@gmail.com
//   Organization:  None
//
// =====================================================================================

#ifndef HANDLE_PAK_INFO_H
#define HANDLE_PAK_INFO_H

#include "common.h"


//获得内部包文件的数目
ULONG getPakCount (FILE *fpPak);

int	handlePakInfo (FILE *fpPak, ULONG pakCount, ULONG base[], ULONG zipFlag[], 
				FILE *fpTxt, char fileName []);
void putPakSize (FILE *fpPak, FILE *fpTxt);
int getPakZipFlag (FILE *fpPak, ULONG *zipFlag);


ULONG getPakCount (FILE *fpPak);

inline int getPakAddr (FILE *fpPak, ULONG *baseAddr);

int	handlePakInfo (FILE *fpPak, ULONG pakCount, ULONG base[], ULONG zipFlag[], 
				FILE *fpTxt, char fileName[]);

void putPakSize (FILE *fpPak, FILE *fpTxt);

int getPakZipFlag (FILE *fpPak, ULONG *zipFlag);

#endif // #ifndef HANDLE_PAK_INFO_H
