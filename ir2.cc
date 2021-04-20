#include "ir2.h"
#include "util.h"

void Ir2::init()
{
	for (U8 i = 0; i < sizeof (data) ; i++) data[i] = 0;
	wout = 0;
	win  = 0;
	tick = 0;
}

void Ir2::step()
{
	wout = nibble_read4 (data, tick);
	nibble_write4 (data, tick, win);
	tick++;
	if (tick == 252) tick = 0;
}
