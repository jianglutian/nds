/*
 * =====================================================================================
 *
 *       Filename:  ControlChar.h
 *
 *    Description:  有关控制字符的处理
 *
 *        Version:  1.0
 *        Created:  2013年09月25日 15时30分33秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  JiangLutian (JLT), jianglutian@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef CONTROL_CHAR_H
#define CONTROL_CHAR_H

#include "common.h"
BOOL loadCtrlCharFile(const char* strName);
void insertCtrlCoding (char strTable[CODING_LENGTH + 1][10], int *iarrLength);
#endif

