#include <avr/pgmspace.h>

#include "ik13.h"


void Ik13::runSignal(U8 n)
{
	U8 x, y, z;
	// CPU control signals carved in
	switch (n)
	{
		case 0: alu.alpha |= nibble_read (r.byte, tick); break;
		case 1: alu.alpha |= nibble_read (m.byte, tick); break;
		case 2: alu.alpha |= nibble_read (st.byte, tick); break;
		case 3: alu.alpha |= ~nibble_read (r.byte, tick) & 0b1111; break;
		case 4: if (!l) alu.alpha |= 0xa; break;
		case 5: alu.alpha |= s; break;
		case 6: alu.alpha |= 4; break;
		case 7: alu.beta |= s; break;
		case 8: alu.beta |= ~s & 0b1111; break;
		case 9: alu.beta |= s1; break;
		case 10: alu.beta |= 6; break;
		case 11: alu.beta |= 1; break;
		case 12: alu.gamma |= l; break;
		case 13: alu.gamma |= !l; break;
		case 14: alu.gamma |= !t; break;
		case 15: nibble_write (r.byte, tick,
				nibble_read (r.byte, (tick + 3) % 42)); break;
		case 16: nibble_write (r.byte, tick, alu.sigma); break;
		case 17: nibble_write (r.byte, tick, s); break;
		case 18: nibble_write (r.byte, tick,
				nibble_read (r.byte, tick)
				| s
				| alu.sigma); break;
		case 19: nibble_write (r.byte, tick, s | alu.sigma); break;
		case 20: nibble_write (r.byte, tick,
				nibble_read (r.byte, tick) | s); break;
		case 21: nibble_write (r.byte, tick,
				nibble_read (r.byte, tick) | alu.sigma); break;
		case 22: nibble_write (r.byte, (tick + 41) % 42, alu.sigma); break;
		case 23: nibble_write (r.byte, (tick + 40) % 42, alu.sigma); break;
		case 24: nibble_write (m.byte, tick, s); break;
		case 25: l = p; break;
		case 26: s = s1; break;
		case 27: s = alu.sigma; break;
		case 28: s = s1 | alu.sigma; break;
		case 29: s1 = alu.sigma; break;
		case 30: s1 = s1; break; // wat?
		case 31: s1 = s1 | alu.sigma; break;
		case 32:
			nibble_write (st.byte, (tick + 2) % 42,
					nibble_read (st.byte, (tick + 1) % 42));
			nibble_write (st.byte, (tick + 1) % 42,
					nibble_read (st.byte, tick));
			nibble_write (st.byte, tick, alu.sigma);
			break;
		case 33:
			x = nibble_read (st.byte, tick);
			nibble_write (st.byte, tick,
					nibble_read (st.byte, (tick + 1) % 42));
			nibble_write (st.byte, (tick + 1) % 42,
					nibble_read (st.byte, (tick + 2) % 42));
			nibble_write (st.byte, (tick + 2) % 42, x);
			break;
		case 34:
			x = nibble_read (st.byte, tick);
			y = nibble_read (st.byte, (tick + 1) % 42);
			z = nibble_read (st.byte, (tick + 2) % 42);
			nibble_write (st.byte, tick, alu.sigma | y);
			nibble_write (st.byte, (tick + 1) % 42, x | z);
			nibble_write (st.byte, (tick + 2) % 42, y | x);
			break;
	}
}

void Ik13::runMicroCommand()
{
	U8 third = tick / 3;

	alu.alpha = 0;
	alu.beta  = 0;
	alu.gamma = 0;
	alu.sigma = 0;

	if (CHECK_BIT(ucommand.byte, 25) && (third != key_x - 1))
	{
		s1 |= key_y;
	}

	for (U8 bit = 0; bit < 12; bit++)
	{
		if (CHECK_BIT(ucommand.byte, bit))
			runSignal (bit);
	}

	if (command.byte[2] & 0b111111)
	{
		if (key_y == 0) t = 0;
	}
	else
	{
		if ((third == key_x - 1) && key_y)
		{
			s1 = key_y;
			t  = 1;
		}
		disp_commas[third] = l;
		disp_upd = 1;
	}

	for (U8 bit = 12; bit < 15; bit++)
	{
		if (CHECK_BIT(ucommand.byte, bit))
			runSignal (bit);
	}

	U8 sum = alu.alpha + alu.beta + alu.gamma;
	alu.sigma = sum & 0b1111;
	p     = (sum >> 4) & 1;

	if (!CHECK_BIT(command.byte, 22) || (third == 12) || (third == 13))
	{
		// bits 17-15 of command 
		U8 ucfield = ((ucommand.byte[2] & 0b11) << 1) | (ucommand.byte[1] >> 7);

		if (ucfield)
			runSignal (ucfield + 14);

		for (U8 bit = 18; bit < 20; bit++)
		{
			if (CHECK_BIT(ucommand.byte, bit))
				runSignal (bit + 4);
		}
	}
	
	for (U8 bit = 20; bit < 22; bit++)
	{
		if (CHECK_BIT(ucommand.byte, bit))
			runSignal (bit + 4);
	}

	for (U8 i = 0; i < 3; i++)
	{
		U8 ucfield = (!!CHECK_BIT(ucommand.byte, 23 + i * 2) << 1) | !!CHECK_BIT(ucommand.byte, 22 + i * 2);
		if (ucfield)
		{
			runSignal (ucfield + 25 + i * 3);
		}
	}
}

void Ik13::init(Cmd23 *rom_cmd, Synch *rom_syn, UCmd28 *rom_ucmd)
{
	commands = rom_cmd;
	sprograms = rom_syn;
	ucommands = rom_ucmd;
	synaddr = 0;
	s = 0;
	s1 = 0;
	l = 0;
	t = 0;
	p = 0;
	for (U8 i = 0; i < sizeof (m.byte) ; i++)     m.byte[i] = 0;
	for (U8 i = 0; i < sizeof (r.byte) ; i++)     r.byte[i] = 0;
	for (U8 i = 0; i < sizeof (st.byte); i++)     st.byte[i] = 0;
	for (U8 i = 0; i < sizeof (disp_commas); i++) disp_commas[i] = 0;

	wout = 0;
	win  = 0;

	tick = 0;
	disp_upd = 0;
	key_x = 0;
	key_y = 0;
}

void Ik13::step()
{
	Synch scmd;

	if (tick == 0)
	{
		// fetch new command
		U8 cmdaddr = (nibble_read (r.byte, 39) << 4)
				| nibble_read (r.byte, 36);
		memcpy_P (&command, &commands[cmdaddr], sizeof (Cmd23));

		if (command.byte[2] & 0b111111) t = 0;
	}

	// derive synchroprogram address from command word
	U8 nine_idx = tick / 9;
	U8 nine_mod = tick % 9;
	if ((nine_mod == 0) && !((nine_idx > 0) && (nine_idx < 3)))
	{
		if (nine_idx < 3)
		{
			// use lower 7 bits
			synaddr = command.byte[0] & 0b1111111;
		}
		else if (nine_idx == 3)
		{
			// use middle 7 bits
			synaddr = ((command.byte[1] & 0b111111) << 1)
				| (command.byte[0] >> 7);
		}
		else if (nine_idx == 4)
		{
			// use upper 8 bits
			synaddr = (command.byte[2] << 2) | (command.byte[1] >> 6);
			if (synaddr > 31)
			{
				if (tick == 36)
				{
					nibble_write (r.byte, 37, synaddr);
					nibble_write (r.byte, 40, synaddr >> 4);
				}
				synaddr = 95;
			}
		}

	}

	// derive microcommand address from synchroprogram in a strange way
	memcpy_P (&scmd, &sprograms[synaddr], sizeof (Synch));
	U8 ucmdaddr = scmd.byte[
				(tick < 6) ? tick :
				((tick < 21) ? (tick % 3 + 3) :
				nine_mod)];
	
	// for addresses 60-63 we have conditional choice depending on the value of L:
	//  if L == 1 they become 60, 62, 64, 66
	//  if L == 0 they become 61, 63, 65, 67 respectively
	ucmdaddr &= 0b111111;
	if (ucmdaddr > 59)
	{
		ucmdaddr = ((ucmdaddr - 60) << 1) + !l + 60;
	}

	// fetch and execute current microcommand
	memcpy_P (&ucommand, &ucommands[ucmdaddr], sizeof (ucommand));
	runMicroCommand ();

	// read/write I/O data and update tick counter
	wout = nibble_read (m.byte, tick);
	nibble_write (m.byte, tick, win);
	tick++;
	if (tick == 42) tick = 0;
}

U8 Ik13::readFromRegister(U8 addr)
{
	return nibble_read (r.byte, addr);
}

void Ik13::writeToMemory(U8 value)
{
	nibble_write (m.byte, (tick + 41) % 42, value);
}
