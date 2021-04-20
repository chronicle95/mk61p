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

class Ir2
{
	// 252 nibbles
	U8 data[126];

public:
	// interface
	U8 win;
	U8 wout;

	// for emulation purposes
	U8 tick;

public:
	void init();
	void step();
};

#endif
