/**
 * Description of IK13 MCU
 *
 * Ported from RusCalc project.
 * https://habr.com/ru/post/505612 "MK-61: History,emulation,internals"
 * http://mk-61.moy.su/emulator.html
 */
#ifndef __IK13_H__
#define __IK13_H__

#include "types.h"
#include "util.h"


typedef struct Reg42 { U8 byte[42]; } Reg42;   //42 nibbles
typedef struct ALU
{
	U8 alpha;
	U8 beta;
	U8 gamma;
	U8 sigma;
} ALU;

typedef struct Ik13
{
	const Cmd23  *commands;
	const Synch  *sprograms;
	const UCmd28 *ucommands;

	// internal stuff
	Cmd23 command;
	U8 synaddr;
	ALU   alu;
	Reg42 m;
	Reg42 r;
	Reg42 st;
	U8 s;
	U8 s1;
	U8 l;
	U8 t;
	U8 p;
	
	// interface
	U8 win;
	U8 wout;

	// for simulation purposes
	U8 tick;
	U8 key_x;
	U8 key_y;
	U8 disp_upd;
	U8 disp_commas[14];
} Ik13;

#define CHECK_BIT(arr, n) (arr[(n) >> 3] & (1 << ((n) & 7)))

void Ik13_init(Ik13 *, Cmd23 *, Synch *, UCmd28 *);
void Ik13_step(Ik13 *);

#endif
