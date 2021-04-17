#include "util.h"


void nibble_write4(U8 *arr, U8 addr, U8 value)
{
	U8 adiv2 = addr >> 1;
	if (addr & 1)
	{
		arr[adiv2] = (value << 4) | (arr[adiv2] & 0xf);
	}
	else
	{
		arr[adiv2] = (arr[adiv2] & 0xf0) | (value & 0xf);
	}
}

U8 nibble_read4(const U8 *arr, U8 addr)
{
	U8 adiv2 = addr >> 1;
	if (addr & 1)
	{
		return arr[adiv2] >> 4;
	}
	else
	{
		return arr[adiv2] & 0xf;
	}
}
