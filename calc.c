/*****************************************************************************
 * MK61 Clone Calculator Firmware for ATmega16L/32A                          *
 * Author: Artem Bondarenko aka chronicle95                                  *
 * Date: 25/03/2021                                                          *
 *                                                                           *
 * Notes:                                                                    *
 *  - Disable JTAG before flashing, it interferes with keypad.               *
 *  - Set frequency to 8 MHz                                                 *
 *****************************************************************************/

#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "calc.h"
#include "ik13.h"
#include "ir2.h"
#include "roms.h"

void init_io();
void init_timer();
void display_hex(U8, U8);

void main()
{
	init_io ();
	init_timer ();

	Ik13 p1302;
	Ik13 p1303;
	Ik13 p1306;
	Ir2  m1;
	Ir2  m2;

	Ik13_init (&p1302, (Cmd23*)&cmds_1302, (Synch*)&syn_1302, (UCmd28*)&ucmds_1302);
	Ik13_init (&p1303, (Cmd23*)&cmds_1303, (Synch*)&syn_1303, (UCmd28*)&ucmds_1303);
	Ik13_init (&p1306, (Cmd23*)&cmds_1306, (Synch*)&syn_1306, (UCmd28*)&ucmds_1306);
	Ir2_init (&m1);
	Ir2_init (&m2);

	// infinite simulation loop
	for (;;)
	{
		p1303.key_x = SwitchValue;
		p1303.key_y = 1;

		// one full instruction cycle
		for (U8 i = 0; i < 42; i++)
		{
			p1302.win = m2.wout;
			Ik13_step (&p1302);

			p1303.win = p1302.wout;
			Ik13_step (&p1303);

			p1306.win = p1303.wout;
			Ik13_step (&p1306);

			m1.win = p1306.wout;
			Ir2_step (&m1);

			m2.win = m1.wout;
			Ir2_step (&m2);

			nibble_write (p1302.m.byte, (p1302.tick + 41) % 42, m2.wout);
		}

		// need to update the display?
		if (p1302.disp_upd)
		{
			for (U8 i = 0; i < 9; i++)
			{
				display_hex (i, nibble_read (p1302.r.byte, (8 - i) * 3));
			}
			for (U8 i = 0; i < 3; i++)
			{
				display_hex (i + 9, nibble_read (p1302.r.byte, (11 - i) * 3));
			}
			p1302.disp_upd = 0;
		} 

		//+DEBUG
		//-DEBUG

		p1302.key_x = 0;
		p1302.key_y = 0;
	}
}

void init_io()
{
	DDRA = 0xff;    // segment control
	PORTA = 0x00;   // 0 - dark, 1 - lit
	DDRB = 0x0f;    // column selector, hi 4 bits
	PORTB = 0x0f;   // all high by default (for common cathode)
	DDRC = 0x00;    // input port for rows and a switch
	PORTC = 0xff;   // pulled up
	DDRD = 0xff;    // column selector, lo 8 bits
	PORTD = 0xff;   // all high by default (for common cathode)
}

void init_timer()
{
	// normal mode, prescaling 1/64
	TCCR0 = (1 << CS01) | (1 << CS00);
	// interrupt on overflow
	TIMSK = (1 << TOIE0);
	// reset the counter
	TCNT0 = 0;
	sei();
}

void display_hex(U8 n, U8 data)
{
	Display[n] = pgm_read_byte (&SegmentData[data]);
}

ISR(TIMER0_OVF_vect)
{
	// prevent other interrupts
	cli ();

	// check keys and switch status
	char Tmp = PINC & 0x3f;
	if (Tmp != 0x3f)
	{
		char Off = 1;
		for (; Tmp & 1; Off += 5, Tmp >>= 1);
		KeyValue = Off + Column;
		KeyStatus = KEYS_DOWN;
	}
	SwitchValue = PINC & 0x80 ? SWITCH_DEG : SWITCH_RAD;

	// calculate new column number and check key status
	Column++;
	if (Column == sizeof(Display))
	{
		if (KeyStatus == KEYS_UP)
		{
			KeyValue = KEY_NONE;
		}
		KeyStatus = KEYS_UP;
		Column    = 0;
	}

	// clear segments to avoid shadowing
	PORTA = 0x00;

	// switch (select) new column
	if (Column < 8)
	{
		PORTD = ~(1 << Column);
		PORTB = 0x0f;
	}
	else
	{
		PORTD = 0xff;
		PORTB = ~(1 << (Column - 8));
	}

	// light the segments
	PORTA = Display[Column];

	// restore interrupts
	sei ();
}
