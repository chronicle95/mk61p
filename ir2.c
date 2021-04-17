#include "ir2.h"
#include "util.h"


void Ir2_init(Ir2 *m)
{
	for (U8 i = 0; i < sizeof (m->data) ; i++) m->data[i] = 0;
	m->wout = 0;
	m->win  = 0;
	m->tick = 0;
}

void Ir2_step(Ir2 *m)
{
	m->wout = nibble_read4 (m->data, m->tick);
	nibble_write4 (m->data, m->tick, m->win);
	m->tick++;
	if (m->tick == 252) m->tick = 0;
}
