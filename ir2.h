/**
 * Description of IR2 Serial memory chip 
 *
 * Ported from RusCalc project.
 * https://habr.com/ru/post/505612 "MK-61: History,emulation,internals"
 * http://mk-61.moy.su/emulator.html
 */
#ifndef __IR2_H__
#define __IR2_H__

#include "types.h"


typedef struct Ir2
{
	// 252 nibbles
	U8 data[126];

	// interface
	U8 win;
	U8 wout;

	// for emulation purposes
	U8 tick;
} Ir2;


void Ir2_init(Ir2 *);
void Ir2_step(Ir2 *);

#endif
