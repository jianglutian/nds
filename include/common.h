#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#if defined(WIN32) || defined(WIN64) 
#include <windows.h> 
#define sleep(n) Sleep(1000 * (n)) 
#else 
#include <unistd.h> 
#endif

typedef unsigned char 	BOOL;
//typedef bool 		 	BOOL;
typedef unsigned char 	BYTE;
typedef unsigned short 	WORD;
typedef unsigned long 	ULONG;
typedef unsigned long 	DWORD;
typedef int 			INT;
typedef unsigned int 	UINT;

#define TRUE 	1
#define FALSE 	0


#define MAX_FILE_NAME_LENGTH 	127			//文件名的最大长度
#define MAX_FILE_COUNT 			0xfff
#define MAX_STRING_LENGTH 		0xFF		//字符串的最大长度

#define FILL_BYTE	((BYTE)(0xff))
#define ZERO_BYTE	((BYTE)(0x00))

#define WAIT	sleep(1000)

#endif
