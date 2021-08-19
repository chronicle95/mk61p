#include <avr/pgmspace.h>

#include "ik13.h"


void Ik13::runSignal(U8 n)
{
	U8 x, y, z;
	// CPU control signals carved in
	switch (n)
	{
		case 0: alu.alpha |= r.byte[tick]; break;
		case 1: alu.alpha |= m.byte[tick]; break;
		case 2: alu.alpha |= st.byte[tick]; break;
		case 3: alu.alpha |= ~r.byte[tick] & 0b1111; break;
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
		case 15: r.byte[tick] = r.byte[(tick + 3) % 42]; break;
		case 16: r.byte[tick] = alu.sigma; break;
		case 17: r.byte[tick] = s; break;
		case 18: r.byte[tick] |= s | alu.sigma; break;
		case 19: r.byte[tick] = s | alu.sigma; break;
		case 20: r.byte[tick] |= s; break;
		case 21: r.byte[tick] |= alu.sigma; break;
		case 22: r.byte[(tick + 41) % 42] = alu.sigma; break;
		case 23: r.byte[(tick + 40) % 42] = alu.sigma; break;
		case 24: m.byte[tick] = s; break;
		case 25: l = p; break;
		case 26: s = s1; break;
		case 27: s = alu.sigma; break;
		case 28: s = s1 | alu.sigma; break;
		case 29: s1 = alu.sigma; break;
		case 30: s1 = s1; break; // wat?
		case 31: s1 = s1 | alu.sigma; break;
		case 32:
			st.byte[(tick + 2) % 42] = st.byte[(tick + 1) % 42];
			st.byte[(tick + 1) % 42] = st.byte[tick];
			st.byte[tick] = alu.sigma;
			break;
		case 33:
			x = st.byte[tick];
			st.byte[tick] = st.byte[(tick + 1) % 42];
			st.byte[(tick + 1) % 42] = st.byte[(tick + 2) % 42];
			st.byte[(tick + 2) % 42] = x;
			break;
		case 34:
			x = st.byte[tick];
			y = st.byte[(tick + 1) % 42];
			z = st.byte[(tick + 2) % 42];
			st.byte[tick] = alu.sigma | y;
			st.byte[(tick + 1) % 42] = x | z;
			st.byte[(tick + 2) % 42] = y | x;
			break;
	}
}

void Ik13::runMicroCommand()
{
	U8 third = tick / 3;
	register U32 ucmd = *( (U32*) &ucommand );

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
		if ((U8) ucmd & 1)
			runSignal (bit);
		ucmd >>= 1;
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
		if ((U8) ucmd & 1)
			runSignal (bit);
		ucmd >>= 1;
	}

	U8 sum = alu.alpha + alu.beta + alu.gamma;
	alu.sigma = sum & 0b1111;
	p     = (sum >> 4) & 1;

	if (!CHECK_BIT(command.byte, 22) || (third == 12) || (third == 13))
	{
		// bits 17-15 of command 
		U8 ucfield = (U8) ucmd & 0b111;
		ucmd >>= 3;

		if (ucfield)
			runSignal (ucfield + 14);

		for (U8 bit = 22; bit < 24; bit++)
		{
			if ((U8) ucmd & 1)
				runSignal (bit);
			ucmd >>= 1;
		}
	}
	else
	{
		ucmd >>= 5;
	}
	
	for (U8 bit = 24; bit < 26; bit++)
	{
		if ((U8) ucmd & 1)
			runSignal (bit);
		ucmd >>= 1;
	}

	for (U8 i = 25; i < 32; i += 3)
	{
		U8 ucfield = (U8) ucmd & 0b11;
		ucmd >>= 2;
		if (ucfield)
		{
			runSignal (ucfield + i);
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

	nine_idx = nine_mod = tick = 0;
	disp_upd = 0;
	key_x = 0;
	key_y = 0;
}

void Ik13::step()
{
	if (tick == 0)
	{
		// fetch new command
		U8 cmdaddr = (r.byte[39] << 4) | r.byte[36];
		memcpy_P (&command, &commands[cmdaddr], sizeof (Cmd23));

		if (command.byte[2] & 0b111111) t = 0;
	}

	// derive synchroprogram address from command word
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
					r.byte[37] = synaddr & 0xf;
					r.byte[40] = synaddr >> 4;
				}
				synaddr = 95;
			}
		}

		memcpy_P (&sprg, &sprograms[synaddr], sizeof (Synch));
	}

	// derive microcommand address from synchroprogram in a strange way
	U8 ucmdaddr = sprg.byte[
				(tick < 6) ? tick :
				((tick < 21) ? (tick % 3 + 3) :
				nine_mod)];

	switch (ucmdaddr)
	{
		case 0x00: // NOP, do nothing
			break;
		case 0x01: // (uc 0x00800001) is actually just S = R[tick]
			s = r.byte[tick];
			break;
		case 0x02: // (uc 0x00040020) is R[tick-1] = S for specific commands in the end of a cycle
			if (!CHECK_BIT(command.byte, 22) || (tick >= 36))
			{
				r.byte[(tick + 41) % 42] = s;
			}
			break;
		case 0x04: // (uc 0x00008000)
			if (!CHECK_BIT(command.byte, 22) || (tick >= 36))
			{
				r.byte[tick] = r.byte[(tick + 3) % 42];
			}
			break;
		case 0x08: // (uc 0x00800008) is S = ~R[tick]
			s = ~r.byte[tick] & 0b1111;
			break;
		case 0x0b: // (uc 0x00080020) is R[tick-2] = S for specific commands in the end of a cycle
			if (!CHECK_BIT(command.byte, 22) || (tick >= 36))
			{
				r.byte[(tick + 40) % 42] = s;
			}
			break;
		default:
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
	}

	// read/write I/O data and update tick counter
	wout = m.byte[tick];
	m.byte[tick] = win;
	tick++;
	nine_mod++;
	if (nine_mod == 9)
	{
		nine_mod = 0;
		nine_idx++;
	}
	if (tick == 42)
	{
		tick = 0;
		nine_idx = 0;
		nine_mod = 0;
	}
}

U8 Ik13::readFromRegister(U8 addr)
{
	return r.byte[addr];
}

void Ik13::writeToMemory(U8 value)
{
	m.byte[(tick + 41) % 42] = value;
}
