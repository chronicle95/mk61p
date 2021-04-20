#ifndef __UTIL_H__
#define __UTIL_H__

#include "types.h"

inline void nibble_write(U8 *a, U8 i, U8 v)
{
	a[i] = v;
}

inline U8 nibble_read(const U8 *a, U8 i)
{
	return a[i];
}

void nibble_write4(U8 *, U8, U8);
U8 nibble_read4(const U8 *, U8);

#endif
