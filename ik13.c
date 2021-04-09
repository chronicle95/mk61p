#include <avr/pgmspace.h>

#include "ik13.h"


static void Ik13_control_signal(Ik13 *p, U8 n)
{
	U8 x, y, z;
	// CPU control signals carved in
	switch (n)
	{
		case 0: p->alu.alpha |= nibble_read (p->r.byte, p->tick); break;
		case 1: p->alu.alpha |= nibble_read (p->m.byte, p->tick); break;
		case 2: p->alu.alpha |= nibble_read (p->st.byte, p->tick); break;
		case 3: p->alu.alpha |= ~nibble_read (p->r.byte, p->tick) & 0b1111; break;
		case 4: if (!p->l) p->alu.alpha |= 0xa; break;
		case 5: p->alu.alpha |= p->s; break;
		case 6: p->alu.alpha |= 4; break;
		case 7: p->alu.beta |= p->s; break;
		case 8: p->alu.beta |= ~p->s & 0b1111; break;
		case 9: p->alu.beta |= p->s1; break;
		case 10: p->alu.beta |= 6; break;
		case 11: p->alu.beta |= 1; break;
		case 12: p->alu.gamma |= p->l; break;
		case 13: p->alu.gamma |= !p->l; break;
		case 14: p->alu.gamma |= !p->t; break;
		case 15: nibble_write (p->r.byte, p->tick,
				nibble_read (p->r.byte, (p->tick + 3) % 42)); break;
		case 16: nibble_write (p->r.byte, p->tick, p->alu.sigma); break;
		case 17: nibble_write (p->r.byte, p->tick, p->s); break;
		case 18: nibble_write (p->r.byte, p->tick,
				nibble_read (p->r.byte, p->tick)
				| p->s
				| p->alu.sigma); break;
		case 19: nibble_write (p->r.byte, p->tick, p->s | p->alu.sigma); break;
		case 20: nibble_write (p->r.byte, p->tick,
				nibble_read (p->r.byte, p->tick) | p->s); break;
		case 21: nibble_write (p->r.byte, p->tick,
				nibble_read (p->r.byte, p->tick) | p->alu.sigma); break;
		case 22: nibble_write (p->r.byte, (p->tick + 41) % 42, p->alu.sigma); break;
		case 23: nibble_write (p->r.byte, (p->tick + 40) % 42, p->alu.sigma); break;
		case 24: nibble_write (p->m.byte, p->tick, p->s); break;
		case 25: p->l = p->p; break;
		case 26: p->s = p->s1; break;
		case 27: p->s = p->alu.sigma; break;
		case 28: p->s = p->s1 | p->alu.sigma; break;
		case 29: p->s1 = p->alu.sigma; break;
		case 30: p->s1 = p->s1; break; // wat?
		case 31: p->s1 = p->s1 | p->alu.sigma; break;
		case 32:
			nibble_write (p->st.byte, (p->tick + 2) % 42,
					nibble_read (p->st.byte, (p->tick + 1) % 42));
			nibble_write (p->st.byte, (p->tick + 1) % 42,
					nibble_read (p->st.byte, p->tick));
			nibble_write (p->st.byte, p->tick, p->alu.sigma);
			break;
		case 33:
			x = nibble_read (p->st.byte, p->tick);
			nibble_write (p->st.byte, p->tick,
					nibble_read (p->st.byte, (p->tick + 1) % 42));
			nibble_write (p->st.byte, (p->tick + 1) % 42,
					nibble_read (p->st.byte, (p->tick + 2) % 42));
			nibble_write (p->st.byte, (p->tick + 2) % 42, x);
			break;
		case 34:
			x = nibble_read (p->st.byte, p->tick);
			y = nibble_read (p->st.byte, (p->tick + 1) % 42);
			z = nibble_read (p->st.byte, (p->tick + 2) % 42);
			nibble_write (p->st.byte, p->tick, p->alu.sigma | y);
			nibble_write (p->st.byte, (p->tick + 1) % 42, x | z);
			nibble_write (p->st.byte, (p->tick + 2) % 42, y | x);
			break;
	}
}

static void Ik13_execute_ucommand(Ik13 *p, UCmd28 *ucommand)
{
	U8 third = p->tick / 3;

	p->alu.alpha = 0;
	p->alu.beta  = 0;
	p->alu.gamma = 0;
	p->alu.sigma = 0;

	if (CHECK_BIT(ucommand->byte, 25) && (third != p->key_x - 1))
	{
		p->s1 |= p->key_y;
	}

	for (U8 bit = 0; bit < 12; bit++)
	{
		if (CHECK_BIT(ucommand->byte, bit))
			Ik13_control_signal (p, bit);
	}

	if (p->command.byte[2] & 0b111111)
	{
		if (p->key_y == 0) p->t = 0;
	}
	else
	{
		if ((third == p->key_x - 1) && p->key_y)
		{
			p->s1 = p->key_y;
			p->t  = 1;
		}
		p->disp_commas[third] = p->l;
		p->disp_upd = 1;
	}

	for (U8 bit = 12; bit < 15; bit++)
	{
		if (CHECK_BIT(ucommand->byte, bit))
			Ik13_control_signal (p, bit);
	}

	U8 sum = p->alu.alpha + p->alu.beta + p->alu.gamma;
	p->alu.sigma = sum & 0b1111;
	p->p     = (sum >> 4) & 1;

	if (!CHECK_BIT(p->command.byte, 22) || (third == 12) || (third == 13))
	{
		// bits 17-15 of command 
		U8 ucfield = ((ucommand->byte[2] & 0b11) << 1) | (ucommand->byte[1] >> 7);

		if (ucfield)
			Ik13_control_signal (p, ucfield + 14);

		for (U8 bit = 18; bit < 20; bit++)
		{
			if (CHECK_BIT(ucommand->byte, bit))
				Ik13_control_signal (p, bit + 4);
		}
	}
	
	for (U8 bit = 20; bit < 22; bit++)
	{
		if (CHECK_BIT(ucommand->byte, bit))
			Ik13_control_signal (p, bit + 4);
	}

	for (U8 i = 0; i < 3; i++)
	{
		U8 ucfield = (!!CHECK_BIT(ucommand->byte, 23 + i * 2) << 1) | !!CHECK_BIT(ucommand->byte, 22 + i * 2);
		if (ucfield)
		{
			Ik13_control_signal (p, ucfield + 25 + i * 3);
		}
	}
}

void Ik13_init(Ik13 *p, Cmd23 *rom_cmd, Synch *rom_syn, UCmd28 *rom_ucmd)
{
	p->commands = rom_cmd;
	p->sprograms = rom_syn;
	p->ucommands = rom_ucmd;
	p->synaddr = 0;
	p->s = 0;
	p->s1 = 0;
	p->l = 0;
	p->t = 0;
	p->p = 0;
	for (U8 i = 0; i < sizeof (p->m.byte) ; i++) p->m.byte[i] = 0;
	for (U8 i = 0; i < sizeof (p->r.byte) ; i++) p->r.byte[i] = 0;
	for (U8 i = 0; i < sizeof (p->st.byte); i++) p->st.byte[i] = 0;
	for (U8 i = 0; i < sizeof (p->disp_commas); i++) p->disp_commas[i] = 0;

	p->wout = 0;
	p->win  = 0;

	p->tick = 0;
	p->disp_upd = 0;
	p->key_x = 0;
	p->key_y = 0;
}

void Ik13_step(Ik13 *p)
{
	Synch scmd;
	UCmd28 ucommand;

	if (p->tick == 0)
	{
		// fetch new command
		U8 cmdaddr = (nibble_read (p->r.byte, 39) << 4)
				| nibble_read (p->r.byte, 36);
		memcpy_P (&p->command, &p->commands[cmdaddr], sizeof (Cmd23));

		if (p->command.byte[2] & 0b111111) p->t = 0;
	}

	// derive synchroprogram address from command word
	U8 nine_idx = p->tick / 9;
	U8 nine_mod = p->tick % 9;
	if ((nine_mod == 0) && !((nine_idx > 0) && (nine_idx < 3)))
	{
		if (nine_idx < 3)
		{
			// use lower 7 bits
			p->synaddr = p->command.byte[0] & 0b1111111;
		}
		else if (nine_idx == 3)
		{
			// use middle 7 bits
			p->synaddr = ((p->command.byte[1] & 0b111111) << 1)
				| (p->command.byte[0] >> 7);
		}
		else if (nine_idx == 4)
		{
			// use upper 8 bits
			p->synaddr = (p->command.byte[2] << 2) | (p->command.byte[1] >> 6);
			if (p->synaddr > 31)
			{
				if (p->tick == 36)
				{
					nibble_write (p->r.byte, 37, p->synaddr);
					nibble_write (p->r.byte, 40, p->synaddr >> 4);
				}
				p->synaddr = 95;
			}
		}

	}

	// derive microcommand address from synchroprogram in a strange way
	memcpy_P (&scmd, &p->sprograms[p->synaddr], sizeof (Synch));
	U8 ucmdaddr = scmd.byte[
				(p->tick < 6) ? p->tick :
				((p->tick < 21) ? (p->tick % 3 + 3) :
				nine_mod)];
	
	// for addresses 60-63 we have conditional choice depending on the value of L:
	//  if L == 1 they become 60, 62, 64, 66
	//  if L == 0 they become 61, 63, 65, 67 respectively
	ucmdaddr &= 0b111111;
	if (ucmdaddr > 59)
	{
		ucmdaddr = ((ucmdaddr - 60) << 1) + !p->l + 60;
	}

	// fetch and execute current microcommand
	memcpy_P (&ucommand, &p->ucommands[ucmdaddr], sizeof (ucommand));
	Ik13_execute_ucommand (p, &ucommand);

	// read/write I/O data and update tick counter
	p->wout = nibble_read (p->m.byte, p->tick);
	nibble_write (p->m.byte, p->tick, p->win);
	p->tick++;
	if (p->tick == 42) p->tick = 0;
}
