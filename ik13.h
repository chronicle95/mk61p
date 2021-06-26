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

struct Reg42 {
	U8 byte[42];
};

struct ALU
{
	U8 alpha;
	U8 beta;
	U8 gamma;
	U8 sigma;
};

#define CHECK_BIT(arr, n) (arr[(n) >> 3] & (1 << ((n) & 7)))

class Ik13
{
private:
	const Cmd23  *commands;
	const Synch  *sprograms;
	const UCmd28 *ucommands;

	// internal stuff
	UCmd28 ucommand;
	Synch sprg;
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

public:
	// interface
	U8 win;
	U8 wout;

	// for simulation purposes
	U8 tick;
	U8 nine_idx, nine_mod;
	U8 key_x;
	U8 key_y;
	U8 disp_upd;
	U8 disp_commas[14];

private:
	void runSignal(U8 n);
	void runMicroCommand();

public:
	void init(Cmd23 *rom_cmd, Synch *rom_syn, UCmd28 *rom_ucmd);
	void step();
	U8 readFromRegister(U8 addr);
	void writeToMemory(U8 value);
};

#endif
