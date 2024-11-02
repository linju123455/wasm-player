#include "utils.h"

uint16_t dhtons(uint16_t hs)
{
	return (ENDIANNESS=='l') ? Swap16(hs): hs;
}

uint32_t dhtonl(uint32_t hl)
{
	return (ENDIANNESS=='l') ? Swap32(hl): hl;
}
 
//将一个无符号短整形数从网络字节顺序转换为主机字节顺序
uint16_t dntohs(uint16_t ns)
{
	return (ENDIANNESS=='l') ? Swap16(ns): ns;	
}
 
//将一个无符号长整形数从网络字节顺序转换为主机字节顺序
uint32_t dntohl(uint32_t nl)
{
	return (ENDIANNESS=='l') ? Swap32(nl): nl;	
}