#pragma once

#include <stdint.h>

//短整形高低字节交换
#define Swap16(A) ((((uint16_t)(A) & 0xff00) >> 8) | (((uint16_t)(A) & 0x00ff) << 8))
//长整形高低字节交换
#define Swap32(A) ((((uint32_t)(A) & 0xff000000) >> 24) | \
				   (((uint32_t)(A) & 0x00ff0000) >>  8) | \
				   (((uint32_t)(A) & 0x0000ff00) <<  8) | \
				   (((uint32_t)(A) & 0x000000ff) << 24))
                   
static union {   
    char c[4];   
    unsigned long mylong;   
} endian_test = {{ 'l', '?', '?', 'b' } }; 

/******************************************************************************
ENDIANNESS返回结果
	l:小端模式
	b:大端模式
******************************************************************************/
#define ENDIANNESS ((char)endian_test.mylong)  

uint16_t dhtons(uint16_t hs);
uint32_t dhtonl(uint32_t hl);
 
//将一个无符号短整形数从网络字节顺序转换为主机字节顺序
uint16_t dntohs(uint16_t ns);
 
//将一个无符号长整形数从网络字节顺序转换为主机字节顺序
uint32_t dntohl(uint32_t nl);
